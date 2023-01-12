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
					// check requirements
					if (pkg->cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder.empty()) {
						pkg->error_proc->SetExecutionResult(FALSE, "You should set External Texture Folder in Settings at first.");
						return;
					}

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
					if (ctx->GetCurrentLevel() == NULL) {
						//no level, add it and add all object into it.
						XObjectPointerArray vtenv_tempObjArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
						int count = vtenv_tempObjArray.Size();

						CKLevel* vtenv_ckLevel = (CKLevel*)ctx->CreateObject(CKCID_LEVEL);
						for (unsigned int i = 0; i < (unsigned int)count; i++)
							vtenv_ckLevel->AddObject(vtenv_tempObjArray.GetObjectA(i));
						ctx->SetCurrentLevel(vtenv_ckLevel);
					}
					vtenv_currentScene = ctx->GetCurrentLevel()->GetLevelScene();


					// ====================================== 
					// core exporter and mesh transition instance
					Export2Virtools* vtExporter = new Export2Virtools(ctx);
					vtExporter->SetObjectsCreationOptions(CK_OBJECTCREATION_RENAME);
					VirtoolsTransitionMesh* vtMeshConverter = new VirtoolsTransitionMesh(vtExporter, FALSE);

					// process forced no component group
					// vtenv stands for virtools environment
					// fncg stands for forced non-component group
					std::string fncgCKGroupKey;
					fncgCKGroupKey = pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str();

					CKGroup* vtenv_fncgCkGroup = NULL;
					if (!pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.empty()) {
						if ((vtenv_fncgCkGroup = (CKGroup*)ctx->GetObjectByNameAndClass((char*)fncgCKGroupKey.c_str(), CKCID_GROUP, NULL)) == NULL) {
							// no forced no component group, generate one
							vtenv_fncgCkGroup = vtExporter->AddGroup(fncgCKGroupKey.c_str(), &fncgCKGroupKey);
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
					std::string texture_loadPath;
					std::filesystem::path texture_internalPath, texture_externalPath;
					//used in material
					CKMaterial* importMaterial = NULL;
					float material_rawColor[3];
					float material_colorPower;
					BOOL material_useTexture, material_alphaProp;
					uint32_t material_textureIndex;
					//used in mesh
					CKMesh* importMesh = NULL;
					uint32_t mesh_listCount;
					std::vector<VxVector> mesh_vList, mesh_vnList;
					std::vector<Vx2DVector> mesh_vtList;
					std::vector<BM_FACE_PROTOTYPE> mesh_faceList;
					CKMaterial* mesh_faceMtl;
					VxUV mesh_faceUvData;
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
						index_entryChunk->obj = NULL;

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

						readString(&ftexture, &texture_filename);
						readBool(&ftexture, &texture_isExternal);
						if (texture_isExternal) {
							// external texture, read from ballance texture folder
							texture_externalPath = pkg->cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder.c_str();
							texture_externalPath /= texture_filename;
							texture_loadPath = texture_externalPath.string().c_str();
						} else {
							// internal folder, read from temp folder
							texture_internalPath = temp_texture_folder / texture_filename;
							texture_loadPath = texture_internalPath.string().c_str();
						}

						// create new virtools object
						importTexture = (CKTexture*)userCreateCKObject(
							ctx, vtExporter, FILE_INDEX_TYPE__TEXTURE,
							(*iter)->name.c_str(), &(*iter), (void*)texture_loadPath.c_str(),
							bm_import_window->OUT_rename_tex, &is_existed_probe
						);
						(*iter)->obj = importTexture;
						if (is_existed_probe) continue;

						// process extra fields
						if (texture_isExternal) {
							importTexture->SetSaveOptions(CKTEXTURE_EXTERNAL);
						} else {
							importTexture->SetSaveOptions(CKTEXTURE_RAWDATA);
						}
						importTexture->FreeVideoMemory();

					}
					ftexture.close();

					// read material
					fmaterial.open(temp_folder / "material.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++) {
						// seek position accoring to entry
						fmaterial.seekg((*iter)->offset);

						// setup vt mat
						VirtoolsMaterial vtmat;

						// pre-set for alpha channel
#define readColor readArrayData<float>(&fmaterial, material_rawColor, UINT32_C(3));
#define deliverColor VxColor(material_rawColor[0], material_rawColor[1], material_rawColor[2])
						readColor;
						vtmat.m_Ambient = deliverColor;
						readColor;
						vtmat.m_Diffuse = deliverColor;
						readColor;
						vtmat.m_Specular = deliverColor;
						readColor;
						vtmat.m_Emissive = deliverColor;
#undef readColor
#undef deliverColor
						readData<float>(&fmaterial, &material_colorPower);
						vtmat.m_SpecularPower = material_colorPower;

						// create new virtools obj
						importMaterial = (CKMaterial*)userCreateCKObject(
							ctx, vtExporter, FILE_INDEX_TYPE__MATERIAL,
							(*iter)->name.c_str(), &(*iter), (void*)&vtmat,
							bm_import_window->OUT_rename_mat, &is_existed_probe
						);
						(*iter)->obj = importMaterial;
						if (is_existed_probe) continue;

						// all alpha props should be applied after opaque props
						// because VirtoolsMaterial do not provide interface
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
							importMaterial->SetTexture((CKTexture*)textureList[material_textureIndex]->obj);
						}

					}
					fmaterial.close();

					// read mesh
					fmesh.open(temp_folder / "mesh.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++) {
						// seek position accoring to entry
						fmesh.seekg((*iter)->offset);

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

						// push into mesh transition class
						vtMeshConverter->Reset();
						// add position
						vtMeshConverter->m_Vertices.Resize(mesh_vList.size());
						memcpy(vtMeshConverter->m_Vertices.Begin(), mesh_vList.data(), sizeof(VxVector) * mesh_vList.size());
						// add vt vn and face according to face data
						for (size_t i = 0, faceid = 0; i < mesh_faceList.size(); (++i), (faceid += 3)) {
							BM_FACE_PROTOTYPE* pFace = &(mesh_faceList[i]);

							// get face mtl
							if (pFace->use_material) {
								mesh_faceMtl = (CKMaterial*)materialList[pFace->material_index]->obj;
							} else {
								mesh_faceMtl = NULL;
							}
							// add face
							vtMeshConverter->AddFace(pFace->indices.data.v1, pFace->indices.data.v2, pFace->indices.data.v3, mesh_faceMtl);

							// add uv
#define cpyVx2dToVxUv(vx2d, vxuv) (vxuv).u = (vx2d).x; (vxuv).v = (vx2d).y;
							cpyVx2dToVxUv(mesh_vtList[pFace->indices.data.vt1], mesh_faceUvData);
							vtMeshConverter->AddUv(mesh_faceUvData);
							cpyVx2dToVxUv(mesh_vtList[pFace->indices.data.vt2], mesh_faceUvData);
							vtMeshConverter->AddUv(mesh_faceUvData);
							cpyVx2dToVxUv(mesh_vtList[pFace->indices.data.vt3], mesh_faceUvData);
							vtMeshConverter->AddUv(mesh_faceUvData);
#undef cpyVx2dToVxUv
							vtMeshConverter->AddUVFace(faceid, faceid + 1, faceid + 2);

							// add normals
							vtMeshConverter->m_Normals.PushBack(mesh_vnList[pFace->indices.data.vn1]);
							vtMeshConverter->m_Normals.PushBack(mesh_vnList[pFace->indices.data.vn2]);
							vtMeshConverter->m_Normals.PushBack(mesh_vnList[pFace->indices.data.vn3]);
							vtMeshConverter->AddNormalFace(faceid, faceid + 1, faceid + 2);
						}

						// generate mesh transition data
						vtMeshConverter->GenerateVirtoolsData();

						// create new virtools obj
						importMesh = (CKMesh*)userCreateCKObject(
							ctx, vtExporter, FILE_INDEX_TYPE__MESH,
							(*iter)->name.c_str(), &(*iter), (void*)vtMeshConverter,
							bm_import_window->OUT_rename_mesh, &is_existed_probe
						);
						(*iter)->obj = importMesh;
						if (is_existed_probe) continue;

					}
					fmesh.close();

					// read object
					fobject.open(temp_folder / "object.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
						// seek position accoring to entry
						fobject.seekg((*iter)->offset);

						// create new virtools obj
						importObject = (CK3dObject*)userCreateCKObject(
							ctx, vtExporter, FILE_INDEX_TYPE__OBJECT,
							(*iter)->name.c_str(), &(*iter), NULL,
							bm_import_window->OUT_rename_obj, &is_existed_probe
						);
						(*iter)->obj = importObject;
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
						if (object_isComponent) {
							importObject->SetCurrentMesh(loadComponenetMesh(ctx, vtExporter, vtMeshConverter, object_meshIndex));
						} else {
							// simplely point to mesh
							importObject->SetCurrentMesh((CKMesh*)meshList[object_meshIndex]->obj);

							// this is a normal object, but we should check whether it is a forced no component via its name.
							// names following tools chain standard
							if (vtenv_fncgCkGroup != NULL && isComponentInStandard(&((*iter)->name))) {
								// yes, it is a forcec non-component, group it into group
								vtenv_fncgCkGroup->AddObject(importObject);
							}
						}

						// apply hidden
						importObject->Show(object_isHidden ? CKHIDE : CKSHOW);
						//// add into scene
						//vtenv_currentScene->AddObjectToScene(importObject);
					}
					fobject.close();

					// order exporter output data
					CKObjectArray* allImportedObjects = CreateCKObjectArray();
					vtExporter->GenerateObjects(allImportedObjects);
					// add into scene all of them
					for (allImportedObjects->Reset(); !allImportedObjects->EndOfList(); allImportedObjects->Next()) {
						CKObject* tmp = allImportedObjects->GetData(ctx);
						switch (tmp->GetClassID()) {
							case CKCID_TEXTURE:
							case CKCID_MATERIAL:
							case CKCID_MESH:
							case CKCID_3DOBJECT:
								vtenv_currentScene->AddObjectToScene((CKSceneObject*)tmp);
							default:
								break;
						}
					}

					// release all chunks allocated from index reading
					for (auto iter = textureList.begin(); iter != textureList.end(); iter++)
						delete (*iter);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++)
						delete (*iter);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++)
						delete (*iter);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
						delete (*iter);

					// delete exporter
					delete vtMeshConverter;
					delete vtExporter;

					// delete window and return
					delete bm_import_window;
					pkg->error_proc->SetExecutionResult(TRUE);
				}

				// WARNING: all following `Read` func are based on current OS is little-endian.
				void readBool(std::ifstream* fs, BOOL* boolean) {
					uint8_t num;
					readData<uint8_t>(fs, &num);
					*boolean = num ? TRUE : FALSE;
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
				CKMesh* loadComponenetMesh(CKContext* ctx, Export2Virtools* exporter, VirtoolsTransitionMesh* converter, uint32_t model_index) {
					// NOTE: this code is sync with bm_import mesh creaion. if something changed, please sync them.

					// declare value
					std::filesystem::path meshfile;
					CKMesh* currentMesh = NULL;
					std::vector<VxVector> vList, vnList;
					std::vector<COMPONENT_FACE_PROTOTYPE> faceList;
					uint32_t vecCount;
					std::ifstream fmesh;
					std::string filename, meshname;


					// then, get file
					utils::string_helper::StdstringPrintf(&filename, "%s.bin", CONST_ExternalComponent[model_index]);
					utils::win32_helper::GetVirtoolsFolder(&meshfile);
					meshfile /= L"BMMeshes";
					meshfile /= filename.c_str();
					fmesh.open(meshfile.wstring().c_str(), std::ios_base::in | std::ios_base::binary);

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

					// push into mesh transition class
					converter->Reset();
					// add position
					converter->m_Vertices.Resize(vList.size());
					memcpy(converter->m_Vertices.Begin(), vList.data(), sizeof(VxVector) * vList.size());
					// add vt vn and face according to face data
					for (size_t i = 0, faceid = 0; i < faceList.size(); (++i), (faceid += 3)) {
						COMPONENT_FACE_PROTOTYPE* pFace = &(faceList[i]);

						// add face
						converter->AddFace(pFace->data.v1, pFace->data.v2, pFace->data.v3, NULL);

						// add normals
						converter->m_Normals.PushBack(vnList[pFace->data.vn1]);
						converter->m_Normals.PushBack(vnList[pFace->data.vn2]);
						converter->m_Normals.PushBack(vnList[pFace->data.vn3]);
						converter->AddNormalFace(faceid, faceid + 1, faceid + 2);
					}

					// generate mesh transition data
					converter->GenerateVirtoolsData();

					// create mesh
					utils::string_helper::StdstringPrintf(&meshname, "COMP_MESH_%s", CONST_ExternalComponent[model_index]);
					BOOL existed_probe;
					return (CKMesh*)userCreateCKObject(
						ctx, exporter, FILE_INDEX_TYPE__MESH,
						meshname.c_str(), &CONST_ExternalComponent[model_index], (void*)converter,
						TRUE, &existed_probe
					);
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
				CKObject* userCreateCKObject(CKContext* ctx, Export2Virtools* exporter, FILE_INDEX_TYPE cls, const char* name, void* pkey, void* extraData, BOOL use_rename, BOOL* is_existed) {
					static CK_CLASSID clsid[] = {
						CKCID_3DOBJECT, CKCID_MESH, CKCID_MATERIAL, CKCID_TEXTURE
					};

					if (!use_rename) {
						// try get current node
						CKObject* obj = ctx->GetObjectByNameAndClass((CKSTRING)name, clsid[cls], NULL);
						if (obj != NULL) {
							*is_existed = TRUE;
							return obj;
						}
					}

					// create new one
					*is_existed = FALSE;
					switch (cls) {
						case FILE_INDEX_TYPE__OBJECT:
							return exporter->Add3dObject(name, pkey);
						case FILE_INDEX_TYPE__MESH:
							return exporter->AddMesh((VirtoolsTransitionMesh*)extraData, name, pkey);
						case FILE_INDEX_TYPE__MATERIAL:
							return exporter->AddMaterial((VirtoolsMaterial*)extraData, name, pkey);
						case FILE_INDEX_TYPE__TEXTURE:
							return exporter->AddTexture((const char*)extraData, name, pkey);
						default:
							return NULL;
					}

				}
			}
		}
	}
}