#include "bmfile.h"
#include "grouping.h"
#include "../../utils/zip_helper.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"
#include <cuchar>

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {
			
				void ImportBM(utils::ParamPackage* pkg) {
					// ====================================== decompress
					// get file
					std::string filepath;
					std::filesystem::path file, temp, tempTexture;
					if (!utils::win32_helper::OpenFileDialog(&filepath, "BM file(*.bmx)\0*.bmx\0", "bmx", TRUE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected BM file.");
						return;
					}
					file = filepath;
					// get temp folder
					utils::win32_helper::GetTempFolder(&temp);
					temp /= "9d2aa26133b94afaa2edcaf580c71e86";	// 9d2aa26133b94afaa2edcaf580c71e86 is guid
					tempTexture = temp / "Texture";
					//clean temp folder
					std::filesystem::remove_all(temp);
					std::filesystem::create_directory(temp);
					// decompress it
					utils::zip_helper::Decompress(&file, &temp);


					// ====================================== read
					std::ifstream findex, fobject, fmesh, fmaterial, ftexture;
					std::vector<FILE_INDEX_HELPER*> objectList, meshList, materialList, textureList;
					// ckcontext ensurance
					CKContext* ctx = pkg->plgif->GetCKContext();
					CKScene* curScene = NULL;
					XObjectPointerArray tmpArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
					int count = tmpArray.Size();
					if (ctx->GetCurrentLevel() == NULL) {
						//no level, add it and add all object into it.
						CKLevel* levels = (CKLevel*)ctx->CreateObject(CKCID_LEVEL);
						for (unsigned int i = 0; i < count; i++)
							levels->AddObject(tmpArray.GetObjectA(i));
						ctx->SetCurrentLevel(levels);
					}
					curScene = ctx->GetCurrentLevel()->GetLevelScene();

					// forced no component group
					CKGroup* forcedNoComponentGroup = NULL;
					if (!pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.empty()) {
						if ((forcedNoComponentGroup = (CKGroup*)ctx->GetObjectByNameAndClass((char*)pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str(), CKCID_GROUP, NULL)) == NULL) {
							// no needed group. generate one
							forcedNoComponentGroup = (CKGroup*)ctx->CreateObject(CKCID_GROUP, (char*)pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str(), CK_OBJECTCREATION_RENAME);
							curScene->AddObjectToScene(forcedNoComponentGroup);
						}
					}

					//used in index
					FILE_INDEX_HELPER* helper_struct = NULL;
					uint8_t index_type;
					//used in texture
					std::string texture_filename;
					CKTexture* currenrTexture = NULL;
					BOOL is_external;
					std::filesystem::path texture_file;
					//used in material
					CKMaterial* currentMaterial = NULL;
					VxColor color;
					float colorPower;
					BOOL is_texture;
					uint32_t texture_index;
					//used in mesh
					CKMesh* currentMesh = NULL;
					VxVector vector;
					uint32_t vecCount;
					std::vector<VxVector> vList, vnList, vtList; // vt only use xy to store uv
					uint32_t face_data[9];
					BOOL is_material;
					uint32_t material_index;
					//used in object
					CK3dObject* currentObject = NULL;
					VxMatrix world_matrix;
					BOOL is_component, is_forced_no_component, is_hidden;
					uint32_t group_list_count;
					std::string group_name_cache;
					uint32_t mesh_index;


					// read info.bm and check version first
					findex.open(temp / "index.bm", std::ios_base::in | std::ios_base::binary);

					uint32_t vercmp;
					readInt(&findex, &vercmp);
					if (vercmp != BM_FILE_VERSION) {
						findex.close();
						pkg->error_proc->SetExecutionResult(FALSE, "Not supported BM version. Need: %d, Got: %d", BM_FILE_VERSION, vercmp);
						return;
					}

					while (TRUE) {
						if (findex.peek(), findex.eof()) break;

						helper_struct = new FILE_INDEX_HELPER();
						readString(&findex, &(helper_struct->name));
						readInt(&findex, &index_type);
						switch ((FILE_INDEX_TYPE)index_type) {
							case FILE_INDEX_TYPE__OBJECT:
								objectList.push_back(helper_struct);
								break;
							case FILE_INDEX_TYPE__MESH:
								meshList.push_back(helper_struct);
								break;
							case FILE_INDEX_TYPE__MATERIAL:
								materialList.push_back(helper_struct);
								break;
							case FILE_INDEX_TYPE__TEXTURE:
								textureList.push_back(helper_struct);
								break;
							default:
								;
						}
						readInt(&findex, &(helper_struct->offset));
					}

					findex.close();

					// read texture
					ftexture.open(temp / "texture.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = textureList.begin(); iter != textureList.end(); iter++) {
						ftexture.seekg((*iter)->offset);

						readString(&ftexture, &texture_filename);
						currenrTexture = (CKTexture*)ctx->CreateObject(CKCID_TEXTURE, (char*)(*iter)->name.c_str(), CK_OBJECTCREATION_RENAME);
						(*iter)->id = currenrTexture->GetID();
						readBool(&ftexture, &is_external);
						if (is_external) {
							loadExternalTexture(&texture_filename, currenrTexture, pkg);
							currenrTexture->SetSaveOptions(CKTEXTURE_EXTERNAL);
						} else {
							texture_file = tempTexture / texture_filename;
							currenrTexture->LoadImageA((char*)texture_file.string().c_str());
							currenrTexture->SetSaveOptions(CKTEXTURE_RAWDATA);
						}
						currenrTexture->FreeVideoMemory();
					}
					ftexture.close();

					// read material
					fmaterial.open(temp / "material.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++) {
						fmaterial.seekg((*iter)->offset);

						color.a = 1;
						currentMaterial = (CKMaterial*)ctx->CreateObject(CKCID_MATERIAL, (char*)(*iter)->name.c_str(), CK_OBJECTCREATION_RENAME);
						(*iter)->id = currentMaterial->GetID();
#define readColor readFloat(&fmaterial, &(color.r)); readFloat(&fmaterial, &(color.g)); readFloat(&fmaterial, &(color.b));
						readColor;
						currentMaterial->SetAmbient(color);
						readColor;
						currentMaterial->SetDiffuse(color);
						readColor;
						currentMaterial->SetSpecular(color);
						readColor;
						currentMaterial->SetEmissive(color);
#undef readColor
						readFloat(&fmaterial, &colorPower);
						currentMaterial->SetPower(colorPower);

						readBool(&fmaterial, &is_texture);
						readInt(&fmaterial, &texture_index);
						if (is_texture) {
							currentMaterial->SetTexture((CKTexture*)ctx->GetObjectA(textureList[texture_index]->id));
						}
					}
					fmaterial.close();

					// read mesh
					fmesh.open(temp / "mesh.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++) {
						fmesh.seekg((*iter)->offset);

						currentMesh = (CKMesh*)ctx->CreateObject(CKCID_MESH, (char*)(*iter)->name.c_str(), CK_OBJECTCREATION_RENAME);
						(*iter)->id = currentMesh->GetID();
						vList.clear(); vnList.clear(); vtList.clear();
						readInt(&fmesh, &vecCount);
						// lazy load v
						for (int i = 0; i < vecCount; i++) {
							readFloat(&fmesh, &(vector.x));
							readFloat(&fmesh, &(vector.y));
							readFloat(&fmesh, &(vector.z));
							vList.push_back(vector);
						}
						// vn and vt need stored in vector
						readInt(&fmesh, &vecCount);
						vector.z = 0;
						for (int i = 0; i < vecCount; i++) {
							readFloat(&fmesh, &(vector.x));
							readFloat(&fmesh, &(vector.y));
							vtList.push_back(vector);
						}
						readInt(&fmesh, &vecCount);
						for (int i = 0; i < vecCount; i++) {
							readFloat(&fmesh, &(vector.x));
							readFloat(&fmesh, &(vector.y));
							readFloat(&fmesh, &(vector.z));
							vnList.push_back(vector);
						}
						// read face
						readInt(&fmesh, &vecCount);
						// init vector and face count
						currentMesh->SetVertexCount(vecCount * 3);
						currentMesh->SetFaceCount(vecCount);
						for (int i = 0; i < vecCount; i++) {
							for (int j = 0; j < 9; j++)
								readInt(&fmesh, &(face_data[j]));

							for (int j = 0; j < 9; j += 3) {
								vector = vList[face_data[j]];
								currentMesh->SetVertexPosition(i * 3 + j / 3, &vector);
								vector = vtList[face_data[j + 1]];
								currentMesh->SetVertexTextureCoordinates(i * 3 + j / 3, vector.x, vector.y);
								vector = vnList[face_data[j + 2]];
								currentMesh->SetVertexNormal(i * 3 + j / 3, &vector);
							}

							currentMesh->SetFaceVertexIndex(i, i * 3, i * 3 + 1, i * 3 + 2);
							readBool(&fmesh, &is_material);
							readInt(&fmesh, &material_index);
							if (is_material) {
								currentMesh->SetFaceMaterial(i, (CKMaterial*)ctx->GetObjectA(materialList[material_index]->id));
							}
						}
					}
					fmesh.close();

					// read object
					fobject.open(temp / "object.bm", std::ios_base::in | std::ios_base::binary);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
						fobject.seekg((*iter)->offset);

						currentObject = (CK3dObject*)ctx->CreateObject(CKCID_3DOBJECT, (char*)(*iter)->name.c_str(), CK_OBJECTCREATION_RENAME);
						(*iter)->id = currentObject->GetID();
						readBool(&fobject, &is_component);
						readBool(&fobject, &is_forced_no_component);
						readBool(&fobject, &is_hidden);
						for (int i = 0; i < 4; i++)
							for (int j = 0; j < 4; j++)
								readFloat(&fobject, &(world_matrix[i][j]));
						currentObject->SetWorldMatrix(world_matrix);

						readInt(&fobject, &group_list_count);
						for (uint32_t i = 0; i < group_list_count; i++) {
							readString(&fobject, &group_name_cache);
							features::mapping::grouping::groupIntoWithCreation(
								ctx,
								currentObject,
								group_name_cache.c_str()
							);
						}

						readInt(&fobject, &mesh_index);
						if (is_component) loadComponenetMesh(currentObject, ctx, mesh_index);
						else currentObject->SetCurrentMesh((CKMesh*)ctx->GetObjectA(meshList[mesh_index]->id));

						// apply hidden
						currentObject->Show(is_hidden ? CKHIDE : CKSHOW);

						//apply component group
						if ((!is_component) && is_forced_no_component && (forcedNoComponentGroup != NULL)) {
							// current object should be grouped into forced no component group
							forcedNoComponentGroup->AddObject(currentObject);
						}

						// all object should add into current scene with dependency
						curScene->AddObjectToScene(currentObject);
					}
					fobject.close();

					// release all list data
					for (auto iter = textureList.begin(); iter != textureList.end(); iter++)
						delete (*iter);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++)
						delete (*iter);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++)
						delete (*iter);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
						delete (*iter);

					pkg->error_proc->SetExecutionResult(TRUE);
				}

				// WARNING: all following `Read` func are based on current OS is little-endian.
				void readBool(std::ifstream* fs, BOOL* boolean) {
					uint8_t num;
					readInt(fs, &num);
					*boolean = num ? TRUE : FALSE;
				}
				void readInt(std::ifstream* fs, uint8_t* num) {
					fs->read((char*)num, sizeof(uint8_t));
				}
				void readInt(std::ifstream* fs, uint32_t* num) {
					fs->read((char*)num, sizeof(uint32_t));
				}
				void readInt(std::ifstream* fs, uint64_t* num) {
					fs->read((char*)num, sizeof(uint64_t));
				}
				void readFloat(std::ifstream* fs, float* num) {
					fs->read((char*)num, sizeof(float));
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
					readInt(fs, &length);
					bmstr.resize(length);
					fs->read((char*)bmstr.data(), length * sizeof(char32_t));

					// decode
					utf8str.reserve(length * 2);	// reserve double dpace for trying avoiding re-alloc
					std::mbstate_t state{};
					char mbout[MB_LEN_MAX]{};
					size_t convCount;
					for (uint32_t i = 0; i < length; i++) {
						convCount = c32rtomb(mbout, bmstr[i], &state);
						if (convCount == -1) continue;
						else {
							utf8str.append(mbout, convCount);
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
				void loadComponenetMesh(CK3dEntity* obj, CKContext* ctx, uint32_t index) {
					// NOTE: this code is sync with bm_import mesh creaion. if something changed, please sync them.

					// declare value
					std::filesystem::path meshfile;
					CKMesh* currentMesh;
					std::vector<VxVector> vList, vnList;
					VxVector vector;
					uint32_t vecCount;
					uint32_t face_data[6];
					std::ifstream fmesh;
					std::string filename, meshname;

					// first, get file
					utils::string_helper::StdstringPrintf(&filename, "%s.bin", CONST_ExternalComponent[index]);
					utils::win32_helper::GetVirtoolsFolder(&meshfile);
					meshfile /= "BMMeshes";
					meshfile /= filename.c_str();
					fmesh.open(meshfile, std::ios_base::in | std::ios_base::binary);

					// then, create mesh
					utils::string_helper::StdstringPrintf(&meshname, "%s_MESH", CONST_ExternalComponent[index]);
					currentMesh = (CKMesh*)ctx->CreateObject(CKCID_MESH, (CKSTRING)meshname.c_str(), CK_OBJECTCREATION_RENAME);

					// read data
					readInt(&fmesh, &vecCount);
					// lazy load v
					for (int i = 0; i < vecCount; i++) {
						readFloat(&fmesh, &(vector.x));
						readFloat(&fmesh, &(vector.y));
						readFloat(&fmesh, &(vector.z));
						vList.push_back(vector);
					}
					readInt(&fmesh, &vecCount);
					for (int i = 0; i < vecCount; i++) {
						readFloat(&fmesh, &(vector.x));
						readFloat(&fmesh, &(vector.y));
						readFloat(&fmesh, &(vector.z));
						vnList.push_back(vector);
					}
					// read face
					readInt(&fmesh, &vecCount);
					// init vector and face count
					currentMesh->SetVertexCount(vecCount * 3);
					currentMesh->SetFaceCount(vecCount);
					for (int i = 0; i < vecCount; i++) {
						for (int j = 0; j < 6; j++)
							readInt(&fmesh, &(face_data[j]));

						for (int j = 0; j < 6; j += 2) {
							vector = vList[face_data[j]];
							currentMesh->SetVertexPosition(i * 3 + j / 2, &vector);
							vector = vnList[face_data[j + 1]];
							currentMesh->SetVertexNormal(i * 3 + j / 2, &vector);
						}

						currentMesh->SetFaceVertexIndex(i, i * 3, i * 3 + 1, i * 3 + 2);
					}

					fmesh.close();

					// apply current mesh into object
					obj->SetCurrentMesh(currentMesh);
				}

			}
		}
	}
}