#include "bmfile.h"
#include "grouping.h"
#include "../../utils/zip_helper.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"
#include "../../mfcwindows/bmfile_import.h"
#include <cuchar>

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {

				void ImportBM(utils::ParamPackage* pkg) {
					// ====================================== 
					// preparing temp folder
					// get bmx file and decompress it into temp folder
					std::wstring gotten_bmx_file;
					std::filesystem::path bmx_file_path, temp_folder, temp_texture_folder;
					if (!utils::win32_helper::OpenFileDialog(&gotten_bmx_file, L"BM file(*.bmx)\0*.bmx\0", L"bmx", TRUE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected BM file.");
						return;
					}
					bmx_file_path = gotten_bmx_file.c_str();
					// get merge setting
					bvh::mfcwindows::BMFileImport* bm_import_window = new bvh::mfcwindows::BMFileImport(pkg);
					if (bm_import_window->DoModal() != IDOK) {
						delete bm_import_window;
						pkg->error_proc->SetExecutionResult(FALSE, "You must specific name conflict strategies.");
						return;
					}

					// get temp folder
					utils::win32_helper::GetTempFolder(pkg->plgif->GetCKContext(), &temp_folder);
					temp_folder /= "9d2aa26133b94afaa2edcaf580c71e86";	// 9d2aa26133b94afaa2edcaf580c71e86 is guid
					temp_texture_folder = temp_folder / "Texture";
					// clean temp folder
					std::filesystem::remove_all(temp_folder);
					std::filesystem::create_directory(temp_folder);
					// decompress bmx file
					utils::zip_helper::Decompress(&bmx_file_path, &temp_folder);

					// ====================================== 
					// preparing virtools enviroment
					// ensure ckContext, ckScene and ckLevel
					CKContext* ctx = pkg->plgif->GetCKContext();
					CKScene* vtenv_currentScene = NULL;
					XObjectPointerArray vtenv_tempObjArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
					int count = vtenv_tempObjArray.Size();
					if (ctx->GetCurrentLevel() == NULL) {
						//no level, add it and add all object into it.
						CKLevel* vtenv_ckLevel = (CKLevel*)ctx->CreateObject(CKCID_LEVEL);
						for (unsigned int i = 0; i < (unsigned int)count; i++)
							vtenv_ckLevel->AddObject(vtenv_tempObjArray.GetObjectA(i));
						ctx->SetCurrentLevel(vtenv_ckLevel);
					}
					vtenv_currentScene = ctx->GetCurrentLevel()->GetLevelScene();

					// process forced no component group
					// vtenv stands for virtools environment
					// fncg stands for forced non-component group
					CKGroup* vtenv_fncgCkGroup = NULL;
					if (!pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.empty()) {
						if ((vtenv_fncgCkGroup = (CKGroup*)ctx->GetObjectByNameAndClass((char*)pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str(), CKCID_GROUP, NULL)) == NULL) {
							// no forced no component group, generate one
							vtenv_fncgCkGroup = (CKGroup*)ctx->CreateObject(CKCID_GROUP, (char*)pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str(), CK_OBJECTCREATION_RENAME);
							vtenv_currentScene->AddObjectToScene(vtenv_fncgCkGroup);
						}
					}

					// ====================================== 
					// read bmx file from temp folder
					std::ifstream findex, fobject, fmesh, fmaterial, ftexture;
					std::vector<FILE_INDEX_HELPER*> objectList, meshList, materialList, textureList;

					//CK creation var
					BOOL is_existed_probe = FALSE;
					//used in index
					FILE_INDEX_HELPER* index_entryChunk = NULL;
					uint8_t index_type;
					//used in texture
					CKTexture* importTexture = NULL;
					std::string texture_filename;
					BOOL texture_isExternal;
					std::filesystem::path texture_absolutePath;
					//used in material
					CKMaterial* importMaterial = NULL;
					VxColor material_color;
					float material_colorPower;
					BOOL material_useTexture, material_alphaProp;
					uint32_t material_textureIndex;
					//used in mesh
					CKMesh* importMesh = NULL;
					VxVector mesh_3dvector;
					Vx2DVector mesh_2dvector;
					uint32_t mesh_listCount;
					std::vector<VxVector> mesh_vList, mesh_vnList;
					std::vector<Vx2DVector> mesh_vtList;
					std::vector<BM_FACE_PROTOTYPE> mesh_faceList;
					mesh_transition::MeshTransition mesh_converter(ctx);
					//used in object
					CK3dObject* importObject = NULL;
					VxMatrix object_worldMatrix;
					BOOL object_isComponent, object_isHidden;
					uint32_t object_groupListCount;
					std::string object_groupNameCache;
					uint32_t object_meshIndex;


					// read info.bm and check version first
					findex.open(temp_folder / "index.bm", std::ios_base::in | std::ios_base::binary);

					uint32_t vercmp;
					readData<uint32_t>(&findex, &vercmp);
					if (vercmp != BM_FILE_VERSION) {
						findex.close();
						pkg->error_proc->SetExecutionResult(FALSE, "Not supported BM version. Expect: %d, Gotten: %d", BM_FILE_VERSION, vercmp);
						return;
					}

					while (TRUE) {
						// chec eof
						if (findex.peek(), findex.eof()) break;

						// this allocation will be freed at the end of this function
						// via iterating obj/mesh/mtl/texture list.
						index_entryChunk = new FILE_INDEX_HELPER();

						// reading entry and distinguish them
						// then put them into different bucket via its type
						readString(&findex, &(index_entryChunk->name));
						readData<uint8_t>(&findex, &index_type);
						switch ((FILE_INDEX_TYPE)index_type) {
							case FILE_INDEX_TYPE__OBJECT:
								objectList.push_back(index_entryChunk);
								break;
							case FILE_INDEX_TYPE__MESH:
								meshList.push_back(index_entryChunk);
								break;
							case FILE_INDEX_TYPE__MATERIAL:
								materialList.push_back(index_entryChunk);
								break;
							case FILE_INDEX_TYPE__TEXTURE:
								textureList.push_back(index_entryChunk);
								break;
							default:
								break;
						}
						readData<uint64_t>(&findex, &(index_entryChunk->offset));
					}
					findex.close();

					// read texture
					ftexture.open(temp_folder / "texture.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = textureList.begin(); iter != textureList.end(); iter++) {
						// seek position accoring to entry
						ftexture.seekg((*iter)->offset);

						// create new virtools object
						importTexture = (CKTexture*)userCreateCKObject(ctx, CKCID_TEXTURE, (*iter)->name.c_str(), bm_import_window->OUT_rename_tex, &is_existed_probe);
						(*iter)->id = importTexture->GetID();
						if (is_existed_probe) continue;

						readString(&ftexture, &texture_filename);
						readBool(&ftexture, &texture_isExternal);
						if (texture_isExternal) {
							// external texture, read from ballance texture folder
							loadExternalTexture(&texture_filename, importTexture, pkg);
							importTexture->SetSaveOptions(CKTEXTURE_EXTERNAL);
						} else {
							// internal folder, read from temp folder
							texture_absolutePath = temp_texture_folder / texture_filename;
							importTexture->LoadImageA((char*)texture_absolutePath.string().c_str());
							importTexture->SetSaveOptions(CKTEXTURE_RAWDATA);
						}
						importTexture->FreeVideoMemory();


						// add into scene
						vtenv_currentScene->AddObjectToScene(importTexture);
					}
					ftexture.close();

					// read material
					fmaterial.open(temp_folder / "material.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++) {
						// seek position accoring to entry
						fmaterial.seekg((*iter)->offset);

						// create new virtools obj
						importMaterial = (CKMaterial*)userCreateCKObject(ctx, CKCID_MATERIAL, (*iter)->name.c_str(), bm_import_window->OUT_rename_mat, &is_existed_probe);
						(*iter)->id = importMaterial->GetID();
						if (is_existed_probe) continue;

						// pre-set for alpha channel
						material_color.a = 1;
#define readColor readArrayData<float>(&fmaterial, &(material_color.r), UINT32_C(3));
						readColor;
						importMaterial->SetAmbient(material_color);
						readColor;
						importMaterial->SetDiffuse(material_color);
						readColor;
						importMaterial->SetSpecular(material_color);
						readColor;
						importMaterial->SetEmissive(material_color);
#undef readColor
						readData<float>(&fmaterial, &material_colorPower);
						importMaterial->SetPower(material_colorPower);

						readBool(&fmaterial, &material_alphaProp);
						importMaterial->EnableAlphaTest(material_alphaProp);
						if (material_alphaProp) {
							importMaterial->SetAlphaFunc(VXCMP_GREATER);
							importMaterial->SetAlphaRef(1);
						}
						readBool(&fmaterial, &material_alphaProp);
						importMaterial->EnableAlphaBlend(material_alphaProp);
						if (material_alphaProp) {
							importMaterial->SetSourceBlend(VXBLEND_SRCALPHA);
							importMaterial->SetDestBlend(VXBLEND_INVSRCALPHA);
						}
						readBool(&fmaterial, &material_alphaProp);
						importMaterial->EnableZWrite(material_alphaProp);
						if (material_alphaProp) {
							importMaterial->SetZFunc(VXCMP_LESSEQUAL);
						}
						readBool(&fmaterial, &material_alphaProp);
						importMaterial->SetTwoSided(material_alphaProp);

						readBool(&fmaterial, &material_useTexture);
						readData<uint32_t>(&fmaterial, &material_textureIndex);
						if (material_useTexture) {
							importMaterial->SetTexture((CKTexture*)ctx->GetObjectA(textureList[material_textureIndex]->id));
						}


						// add into scene
						vtenv_currentScene->AddObjectToScene(importMaterial);
					}
					fmaterial.close();

					// read mesh
					fmesh.open(temp_folder / "mesh.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++) {
						// seek position accoring to entry
						fmesh.seekg((*iter)->offset);

						// create new virtools obj
						importMesh = (CKMesh*)userCreateCKObject(ctx, CKCID_MESH, (*iter)->name.c_str(), bm_import_window->OUT_rename_mesh, &is_existed_probe);
						(*iter)->id = importMesh->GetID();
						if (is_existed_probe) continue;


						// load mesh
						// load v, vn, vt
						readData<uint32_t>(&fmesh, &mesh_listCount);
						readVectorData<VxVector>(&fmesh, &mesh_vList, mesh_listCount);
						readData<uint32_t>(&fmesh, &mesh_listCount);
						readVectorData<Vx2DVector>(&fmesh, &mesh_vtList, mesh_listCount);
						readData<uint32_t>(&fmesh, &mesh_listCount);
						readVectorData<VxVector>(&fmesh, &mesh_vnList, mesh_listCount);

						// read face
						readData<uint32_t>(&fmesh, &mesh_listCount);
						readVectorData<BM_FACE_PROTOTYPE>(&fmesh, &mesh_faceList, mesh_listCount);

						// send to converter and fet converted mesh
						mesh_converter.DoMeshParse(
							importMesh,
							&mesh_vList, &mesh_vtList, &mesh_vnList,
							&mesh_faceList,
							&materialList
						);

						// add into scene
						vtenv_currentScene->AddObjectToScene(importMesh);
					}
					fmesh.close();

					// read object
					fobject.open(temp_folder / "object.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
						// seek position accoring to entry
						fobject.seekg((*iter)->offset);

						// create new virtools obj
						importObject = (CK3dObject*)userCreateCKObject(ctx, CKCID_3DOBJECT, (*iter)->name.c_str(), bm_import_window->OUT_rename_obj, &is_existed_probe);
						(*iter)->id = importObject->GetID();
						if (is_existed_probe) continue;


						// load object
						// read basic data
						readBool(&fobject, &object_isComponent);
						readBool(&fobject, &object_isHidden);
						readData<VxMatrix>(&fobject, &object_worldMatrix);
						importObject->SetWorldMatrix(object_worldMatrix);

						// read grouping message
						readData<uint32_t>(&fobject, &object_groupListCount);
						for (uint32_t i = 0; i < object_groupListCount; i++) {
							readString(&fobject, &object_groupNameCache);
							features::mapping::grouping::groupIntoWithCreation(
								ctx,
								importObject,
								object_groupNameCache.c_str()
							);
						}

						// read mesh index
						// process it differently by isComponent
						readData<uint32_t>(&fobject, &object_meshIndex);
						if (object_isComponent) loadComponenetMesh(ctx, vtenv_currentScene, importObject, mesh_converter, object_meshIndex);
						else {
							// simplely point to mesh
							importObject->SetCurrentMesh((CKMesh*)ctx->GetObjectA(meshList[object_meshIndex]->id));

							// this is a normal object, but we should check whether it is a forced no component via its name.
							// names following tools chain standard
							if (vtenv_fncgCkGroup != NULL && isComponentInStandard(&((*iter)->name))) {
								// yes, it is a forcec non-component, group it into group
								vtenv_fncgCkGroup->AddObject(importObject);
							}
						}

						// apply hidden
						importObject->Show(object_isHidden ? CKHIDE : CKSHOW);
						// add into scene
						vtenv_currentScene->AddObjectToScene(importObject);
					}
					fobject.close();

					// release all chunks allocated from index reading
					for (auto iter = textureList.begin(); iter != textureList.end(); iter++)
						delete (*iter);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++)
						delete (*iter);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++)
						delete (*iter);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
						delete (*iter);

					// delete window and return
					delete bm_import_window;
					pkg->error_proc->SetExecutionResult(TRUE);
				}

				// References
				// https://zh.cppreference.com/w/cpp/string/multibyte/c32rtomb
				void readString(std::ifstream* fs, std::string* str) {
					// alloc a buffer
					str->clear();
					std::string utf8str;
					std::u32string bmstr;

					// find length and read
					uint32_t length;
					readData<uint32_t>(fs, &length);
					bmstr.resize(length);
					fs->read((char*)bmstr.data(), (std::streamsize)length * sizeof(char32_t));
					utf8str.reserve(length * 2);	// reserve double space for trying avoiding re-alloc

					// decode
					std::mbstate_t state{};
					char mbout[MB_LEN_MAX]{};
					size_t rc;
					for (uint32_t i = 0; i < length; i++) {
						rc = c32rtomb(mbout, bmstr[i], &state);
						if (rc != (std::size_t)-1) {
							utf8str.append(mbout, rc);
						}
					}

					// for microsoft shit implementation. i need convert utf8 output into current locale output
					utils::string_helper::ConvertEncoding(&utf8str, str, CP_UTF8, CP_ACP);
				}
				void loadExternalTexture(std::string* name, CKTexture* texture, utils::ParamPackage* pkg) {
					std::filesystem::path external_folder;
					if (pkg->cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder.empty())
						return;	//empty folder, don't load it.
					external_folder = pkg->cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder;
					external_folder /= *name;

					texture->LoadImageA((char*)external_folder.string().c_str());
					texture->SetSaveOptions(CKTEXTURE_EXTERNAL);
				}
				void loadComponenetMesh(CKContext* ctx, CKScene* scene, CK3dEntity* obj, mesh_transition::MeshTransition converter, uint32_t model_index) {
					// NOTE: this code is sync with bm_import mesh creaion. if something changed, please sync them.

					// declare value
					std::filesystem::path meshfile;
					CKMesh* currentMesh;
					std::vector<VxVector> vList, vnList;
					std::vector<COMPONENT_FACE_PROTOTYPE> faceList;
					uint32_t vecCount;
					std::ifstream fmesh;
					std::string filename, meshname;

					// first, create mesh
					utils::string_helper::StdstringPrintf(&meshname, "COMP_MESH_%s", CONST_ExternalComponent[model_index]);
					BOOL existed_probe;
					currentMesh = (CKMesh*)userCreateCKObject(ctx, CKCID_MESH, meshname.c_str(), FALSE, &existed_probe);
					if (existed_probe) {
						// mesh is existed, link it and directly return.
						obj->SetCurrentMesh(currentMesh);
						return;
					}

					// then, get file
					utils::string_helper::StdstringPrintf(&filename, "%s.bin", CONST_ExternalComponent[model_index]);
					utils::win32_helper::GetVirtoolsFolder(&meshfile);
					meshfile /= L"BMMeshes";
					meshfile /= filename.c_str();
					fmesh.open(meshfile.wstring().c_str(), std::ios_base::in | std::ios_base::binary);

					// check whether file is opened
					// only loaded file can read data. if fail to load file, keep empty mesh.
					if (fmesh.is_open()) {
						// read data
						// load v and vn
						readData<uint32_t>(&fmesh, &vecCount);
						readVectorData<VxVector>(&fmesh, &vList, vecCount);
						readData<uint32_t>(&fmesh, &vecCount);
						readVectorData<VxVector>(&fmesh, &vnList, vecCount);

						// read face
						readData<uint32_t>(&fmesh, &vecCount);
						readVectorData<COMPONENT_FACE_PROTOTYPE>(&fmesh, &faceList, vecCount);

						// end of reading data
						fmesh.close();

						// push into converter and get converted mesh
						converter.DoComponentParse(
							currentMesh,
							&vList, &vnList,
							&faceList
						);

					}

					// add this new one into scene
					scene->AddObjectToScene(currentMesh);
					// apply current mesh into object
					obj->SetCurrentMesh(currentMesh);
				}
				BOOL isComponentInStandard(std::string* name) {
					// check in prefix list
					for (uint32_t i = 0; i < CONST_ExternalComponent_Length; i++) {
						if (utils::string_helper::StdstringStartsWith(*name, CONST_ExternalComponent[i])) {
							// obj is matched with tools chain's standard prefix
							// process obj as a component
							return TRUE;
						}
					}

					// couldn't find it
					return FALSE;
				}
				CKObject* userCreateCKObject(CKContext* ctx, CK_CLASSID cls, const char* name, BOOL use_rename, BOOL* is_existed) {
					if (use_rename) {
						// use rename flag to create object directly
						// use rename will get a new object definitely.
						*is_existed = FALSE;
						return ctx->CreateObject(cls, (CKSTRING)name, CK_OBJECTCREATION_RENAME, NULL);
					} else {
						// try get current node
						CKObject* obj = ctx->GetObjectByNameAndClass((CKSTRING)name, cls, NULL);
						if (obj != NULL) {
							*is_existed = TRUE;
							return obj;
						}

						// otherwise create it directly
						*is_existed = FALSE;
						return ctx->CreateObject(cls, (CKSTRING)name, CK_OBJECTCREATION_NONAMECHECK, NULL);
					}
				}
			}
		}
	}
}