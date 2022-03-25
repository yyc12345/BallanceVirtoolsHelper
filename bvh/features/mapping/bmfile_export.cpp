#include "bmfile.h"
#include "../../utils/zip_helper.h"
#include "../../mfcwindows/bmfile_export.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"
#include <unordered_map>
#include <unordered_set>
#include <cuchar>

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {

				void ExportBM(utils::ParamPackage* pkg) {
					// ============================================ 
					// preparing temp folder and decompress file
					// allocate file path variables
					std::filesystem::path bmx_file, temp_folder, temp_texture_folder;
					// call export windows to get exported file path
					mfcwindows::BMFileExport* bm_export_window = new mfcwindows::BMFileExport(pkg);
					if (bm_export_window->DoModal() != IDOK) {
						delete bm_export_window;
						pkg->error_proc->SetExecutionResult(FALSE, "You cancel this process.");
						return;
					}
					bmx_file = bm_export_window->OUT_File;
					// get temp folder path
					utils::win32_helper::GetTempFolder(&temp_folder);
					temp_folder /= "a6694fa9ca1c46588cf4b6e6d376c3bd";	// a6694fa9ca1c46588cf4b6e6d376c3bd is guid
					temp_texture_folder = temp_folder / "Texture";
					// clean temp folder
					std::filesystem::remove_all(temp_folder);
					std::filesystem::create_directory(temp_folder);
					std::filesystem::create_directory(temp_texture_folder);

					// ============================================
					// allocate some variables and preparing exporting work
					CKContext* ctx = pkg->plgif->GetCKContext();
					std::vector<CK_ID> objectList, meshList, materialList, textureList;
					std::ofstream findex, fobject, fmesh, fmaterial, ftexture;

					// used by forced non-component group
					// fncg stands for `forced non-component group`
					std::unordered_set<CK_ID> utils_fncgSet;
					CKGroup* utils_fncgCkGroup;
					//used by grouping info catch
					std::unordered_multimap<CK_ID, std::string*> grouping_data;
					std::vector<std::string*> grouping_dataNeedFree;
					CKGroup* grouping_ckGroupPtr;
					std::string* grouping_ckGroupNamePtr;
					int grouping_ckGroupLength;
					//used by filter
					std::string filter_name;
					CKGroup* filter_group;
					CKBeObject* filter_groupItem;
					CK3dEntity* filter_3dentity;
					int filter_groupCount;
					//used by index
					uint32_t index_bmVersion;
					uint64_t index_offset;
					FILE_INDEX_TYPE index_type;
					std::string index_name;
					//used by object
					CK3dEntity* exportObject;
					BOOL object_isComponent, object_isHidden;
					uint32_t object_meshIndex;
					VxMatrix object_worldMatrix;
					uint64_t object_chunkPtrHeader, object_chunkPtrTail;
					uint32_t object_groupListCount;
					//used by mesh
					CKMesh* exportMesh;
					VxVector mesh_vector;
					uint32_t mesh_count;
					uint32_t mesh_indexValue;
					float mesh_uvU, mesh_uvV;
					WORD* mesh_faceIndices;
					BOOL mesh_useMaterial;
					CKMaterial* mesh_faceMaterial;
					//used by material
					CKMaterial* exportMaterial;
					VxColor material_color;
					float material_value;
					CKTexture* material_tryGottonTexture;
					BOOL material_useTexture;
					uint32_t material_textureIndex;
					//used by texture
					CKTexture* exportTexture;
					std::filesystem::path texture_absoluteTexturePath, texture_tempTexturePath;
					std::string texture_filename;
					BOOL texture_isExternal;

					// collecting forced non component group data
					utils_fncgCkGroup = NULL;
					if (!pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.empty()) {
						// if no needed group. just skip this. it will be set as NULL;
						utils_fncgCkGroup = (CKGroup*)ctx->GetObjectByNameAndClass((CKSTRING)pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str(), CKCID_GROUP, NULL);
					}
					if (utils_fncgCkGroup != NULL) {
						for (int i = 0, count = utils_fncgCkGroup->GetObjectCount(); i < count; i++) {
							utils_fncgSet.insert(utils_fncgCkGroup->GetObjectA(i)->GetID());
						}
					}

					// pre-collect grouping data from all groups
					// and put infos into unordered_multimap
					// then we can quickly get grouping info
					// in following processing step
					XObjectPointerArray groupArray = ctx->GetObjectListByType(CKCID_GROUP, FALSE);
					for (auto item = groupArray.Begin(); item != groupArray.End(); ++item) {
						grouping_ckGroupPtr = (CKGroup*)(*item);
						if (grouping_ckGroupPtr->GetName() == NULL) continue; // skip group with blank name
						if ((!pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.empty()) &&
							pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName == grouping_ckGroupPtr->GetName()) {
							continue; // we don't need NoComponentGroup data
						}
						
						// allocate new string from heap to store group name
						grouping_ckGroupNamePtr = new std::string();
						*grouping_ckGroupNamePtr = grouping_ckGroupPtr->GetName();
						// add into free list immediately for future's release
						grouping_dataNeedFree.push_back(grouping_ckGroupNamePtr);

						// iterate group object and 
						// record all items
						grouping_ckGroupLength = grouping_ckGroupPtr->GetObjectCount();
						for (int i = 0; i < grouping_ckGroupLength; ++i) {
							grouping_data.insert({ grouping_ckGroupPtr->GetObject(i)->GetID(), grouping_ckGroupNamePtr });
						}
						
					}

					// collecting exported objects data
					// according to user specific config 
					// from export window
					switch (bm_export_window->OUT_Mode) {
						case 0:
							// obj
							objectList.push_back(bm_export_window->OUT_Target);
							break;
						case 1:
							// group
							filter_group = (CKGroup*)ctx->GetObjectA(bm_export_window->OUT_Target);
							filter_groupCount = filter_group->GetObjectCount();
							for (int i = 0; i < filter_groupCount; i++) {
								filter_groupItem = filter_group->GetObjectA(i);
								switch (filter_groupItem->GetClassID()) {
									case CKCID_3DENTITY:
									case CKCID_3DOBJECT:
										if (isValidObject((CK3dEntity*)filter_groupItem))
											objectList.push_back(filter_groupItem->GetID());
										break;
									default:
										break;	// do nothing
								}
							}
							break;
						case 2:
							// all
							XObjectPointerArray objArray = ctx->GetObjectListByType(CKCID_3DENTITY, TRUE);
							for (auto item = objArray.Begin(); item != objArray.End(); ++item) {
								filter_3dentity = (CK3dEntity*)(*item);
								if (isValidObject(filter_3dentity))
									objectList.push_back(filter_3dentity->GetID());
							}
							break;
					}

					// ============================================
					// write bmx internal file in temp folder
					
					// now we open `index.bm` first
					// this file should be opened in the whole of exporting
					// because it store all index infomations
					findex.open(temp_folder / "index.bm", std::ios_base::out | std::ios_base::binary);
					//write version header
					index_bmVersion = BM_FILE_VERSION;
					writeInt(&findex, &index_bmVersion);

					//write object
					fobject.open(temp_folder / "object.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
						exportObject = (CK3dEntity*)ctx->GetObjectA(*iter);
						//write index first
						safelyGetName(exportObject, &index_name);
						writeString(&findex, &index_name);
						index_type = FILE_INDEX_TYPE__OBJECT;
						writeInt(&findex, (uint8_t*)&index_type);
						index_offset = fobject.tellp();
						writeInt(&findex, &index_offset);

						//write object
						getComponent(&utils_fncgSet, exportObject->GetID(), &index_name, &object_isComponent, &object_meshIndex);
						writeBool(&fobject, &object_isComponent);
						object_isHidden = exportObject->IsVisible() == CKHIDE;
						writeBool(&fobject, &object_isHidden);
						object_worldMatrix = exportObject->GetWorldMatrix();
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								writeFloat(&fobject, &object_worldMatrix[i][j]);
							}
						}

						// write group list
						// record length position and ready back to there to write count
						object_groupListCount = 0;
						object_chunkPtrHeader = fobject.tellp();
						writeInt(&fobject, &object_groupListCount);
						auto grouping_data_range = grouping_data.equal_range(exportObject->GetID());
						for (auto it = grouping_data_range.first; it != grouping_data_range.second; ++it) {
							writeString(&fobject, it->second);
							++object_groupListCount;
						}
						// backups current position, back to count position, write it and back to current position
						object_chunkPtrTail = fobject.tellp();
						fobject.seekp(object_chunkPtrHeader);
						writeInt(&fobject, &object_groupListCount);
						fobject.seekp(object_chunkPtrTail);

						// if this object is normal object, we need to register it in mesh list
						// and write corresponding index
						// otherwise, write corresponding component id gotten from
						// previous function calling
						if (!object_isComponent) {
							object_meshIndex = tryAddWithIndex(&meshList, exportObject->GetMesh(0)->GetID());
						}
						writeInt(&fobject, &object_meshIndex);
					}
					fobject.close();

					// write mesh
					fmesh.open(temp_folder / "mesh.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++) {
						exportMesh = (CKMesh*)ctx->GetObjectA(*iter);
						//write index first
						safelyGetName(exportMesh, &index_name);
						writeString(&findex, &index_name);
						index_type = FILE_INDEX_TYPE__MESH;
						writeInt(&findex, (uint8_t*)&index_type);
						index_offset = fmesh.tellp();
						writeInt(&findex, &index_offset);

						//write mesh
						mesh_count = exportMesh->GetVertexCount();
						writeInt(&fmesh, &mesh_count);
						for (uint32_t i = 0; i < mesh_count; i++) {
							exportMesh->GetVertexPosition(i, &mesh_vector);
							writeFloat(&fmesh, &mesh_vector.x);
							writeFloat(&fmesh, &mesh_vector.y);
							writeFloat(&fmesh, &mesh_vector.z);
						}
						writeInt(&fmesh, &mesh_count);
						for (uint32_t i = 0; i < mesh_count; i++) {
							exportMesh->GetVertexTextureCoordinates(i, &mesh_uvU, &mesh_uvV);
							writeFloat(&fmesh, &mesh_uvU);
							writeFloat(&fmesh, &mesh_uvV);
						}
						writeInt(&fmesh, &mesh_count);
						for (uint32_t i = 0; i < mesh_count; i++) {
							exportMesh->GetVertexNormal(i, &mesh_vector);
							writeFloat(&fmesh, &mesh_vector.x);
							writeFloat(&fmesh, &mesh_vector.y);
							writeFloat(&fmesh, &mesh_vector.z);
						}

						mesh_count = exportMesh->GetFaceCount();
						mesh_faceIndices = exportMesh->GetFacesIndices();
						writeInt(&fmesh, &mesh_count);
						for (uint32_t i = 0; i < mesh_count; i++) {
							mesh_indexValue = mesh_faceIndices[i * 3];
							writeInt(&fmesh, &mesh_indexValue);
							writeInt(&fmesh, &mesh_indexValue);
							writeInt(&fmesh, &mesh_indexValue);
							mesh_indexValue = mesh_faceIndices[i * 3 + 1];
							writeInt(&fmesh, &mesh_indexValue);
							writeInt(&fmesh, &mesh_indexValue);
							writeInt(&fmesh, &mesh_indexValue);
							mesh_indexValue = mesh_faceIndices[i * 3 + 2];
							writeInt(&fmesh, &mesh_indexValue);
							writeInt(&fmesh, &mesh_indexValue);
							writeInt(&fmesh, &mesh_indexValue);

							mesh_faceMaterial = exportMesh->GetFaceMaterial(i);
							mesh_useMaterial = mesh_faceMaterial != NULL;
							writeBool(&fmesh, &mesh_useMaterial);
							if (mesh_useMaterial) {
								mesh_indexValue = tryAddWithIndex(&materialList, mesh_faceMaterial->GetID());
							} else mesh_indexValue = 0;
							writeInt(&fmesh, &mesh_indexValue);
						}
					}
					fmesh.close();

					// write material
					fmaterial.open(temp_folder / "material.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++) {
						exportMaterial = (CKMaterial*)ctx->GetObjectA(*iter);
						//write index first
						safelyGetName(exportMaterial, &index_name);
						writeString(&findex, &index_name);
						index_type = FILE_INDEX_TYPE__MATERIAL;
						writeInt(&findex, (uint8_t*)&index_type);
						index_offset = fmaterial.tellp();
						writeInt(&findex, &index_offset);

						//write material
#define writeColor writeFloat(&fmaterial, &material_color.r);writeFloat(&fmaterial, &material_color.g);writeFloat(&fmaterial, &material_color.b);
						material_color = exportMaterial->GetAmbient();
						writeColor;
						material_color = exportMaterial->GetDiffuse();
						writeColor;
						material_color = exportMaterial->GetSpecular();
						writeColor;
						material_color = exportMaterial->GetEmissive();
						writeColor;
#undef writeColor
						material_value = exportMaterial->GetPower();
						writeFloat(&fmaterial, &material_value);

						// try get material
						material_tryGottonTexture = exportMaterial->GetTexture();
						material_useTexture = material_tryGottonTexture != NULL && material_tryGottonTexture->GetSlotFileName(0) != NULL;
						writeBool(&fmaterial, &material_useTexture);
						if (material_useTexture) {
							material_textureIndex = tryAddWithIndex(&textureList, material_tryGottonTexture->GetID());
						} else material_textureIndex = 0;
						writeInt(&fmaterial, &material_textureIndex);
					}
					fmaterial.close();

					// write texture
					ftexture.open(temp_folder / "texture.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = textureList.begin(); iter != textureList.end(); iter++) {
						exportTexture = (CKTexture*)ctx->GetObjectA(*iter);
						//write index first
						safelyGetName(exportTexture, &index_name);
						writeString(&findex, &index_name);
						index_type = FILE_INDEX_TYPE__TEXTURE;
						writeInt(&findex, (uint8_t*)&index_type);
						index_offset = ftexture.tellp();
						writeInt(&findex, &index_offset);

						//write texture
						texture_absoluteTexturePath = exportTexture->GetSlotFileName(0);
						texture_filename = texture_absoluteTexturePath.filename().string();
						texture_tempTexturePath = temp_texture_folder / texture_filename;
						writeString(&ftexture, &texture_filename);

						texture_isExternal = isExternalTexture(ctx, exportTexture, &texture_filename);
						writeBool(&ftexture, &texture_isExternal);
						if (!texture_isExternal) {
							//try copy original file. if function failed, use virtools internal save function.
							if (!CopyFile(texture_absoluteTexturePath.string().c_str(), texture_tempTexturePath.string().c_str(), FALSE)) {
								exportTexture->SaveImage((char*)texture_tempTexturePath.string().c_str(), 0, FALSE);
							}
						}
					}
					ftexture.close();

					// close `index.bm` here
					findex.close();

					// clear pre-collected grouping data
					grouping_data.clear();
					// and free all allocated std::string variables
					for (auto it = grouping_dataNeedFree.begin(); it != grouping_dataNeedFree.end(); it++) {
						delete ((std::string*)(*it));
					}

					// ============================================
					// compress temp folder and write zip
					utils::zip_helper::Compress(&bmx_file, &temp_folder);
					delete bm_export_window;
					pkg->error_proc->SetExecutionResult(TRUE);
				}

				// WARNING: all following `Write` func are based on current OS is little-endian.
				void writeBool(std::ofstream* fs, BOOL* boolean) {
					uint8_t num = *boolean ? 1 : 0;
					writeInt(fs, &num);
				}
				void writeInt(std::ofstream* fs, uint8_t* num) {
					fs->write((char*)num, sizeof(uint8_t));
				}
				void writeInt(std::ofstream* fs, uint32_t* num) {
					fs->write((char*)num, sizeof(uint32_t));
				}
				void writeInt(std::ofstream* fs, uint64_t* num) {
					fs->write((char*)num, sizeof(uint64_t));
				}
				void writeFloat(std::ofstream* fs, float* num) {
					fs->write((char*)num, sizeof(float));
				}
				// References
				// https://zh.cppreference.com/w/cpp/string/multibyte/mbrtoc32
				void writeString(std::ofstream* fs, std::string* str) {
					// for microsoft shit implementation. i need convert utf8 output into current locale output
					std::string utf8str;
					utils::string_helper::ConvertEncoding(str, &utf8str, CP_ACP, CP_UTF8);

					// start convert
					std::mbstate_t state{};
					char32_t c32;
					const char* ptr = utf8str.c_str(), * end = utf8str.c_str() + utf8str.size() + 1;
					uint32_t c32Length = 0;
					std::u32string bmstr;
					bmstr.reserve(utf8str.size());	// pre-alloc for avoiding mem realloc

					while (std::size_t rc = std::mbrtoc32(&c32, ptr, end - ptr, &state)) {
						if (rc == (std::size_t)-3) {
							throw std::logic_error("writeString(): no surrogates in UTF-32.");
						}
						
						if (rc <= ((std::size_t)-1) / 2) {
							bmstr.append(1, c32);
							ptr += rc;
							c32Length++;
						} else break;
					}
					
					// write length
					writeInt(fs, &c32Length);

					// write data
					fs->write((char*)bmstr.data(), c32Length * sizeof(char32_t));

				}
				BOOL isValidObject(CK3dEntity* obj) {
					CKMesh* mesh = obj->GetCurrentMesh();
					if (mesh == NULL) return FALSE;					//no mesh
					if (mesh->GetFaceCount() == 0) return FALSE;	//no face
					return TRUE;
				}
				void getComponent(std::unordered_set<CK_ID>* fncgSet, CK_ID objId, std::string* name, BOOL* is_component, uint32_t* gottten_id) {
					// we check it whether in FNCG first
					if (fncgSet->find(objId) != fncgSet->end()) {
						// got, return it as normal object immediately
						*is_component = FALSE;
						*gottten_id = 0;
						return;
					}

					// if it is not in FNCG, we should check it by tools chain standard prefix
					for (uint32_t i = 0; i < CONST_ExternalComponent_Length; i++) {
						if (utils::string_helper::StdstringStartsWith(*name, CONST_ExternalComponent[i])) {
							// obj is matched with tools chain's standard prefix
							// process obj as a component
							*is_component = TRUE;
							*gottten_id = i;
							return;
						}
					}

					// this object do not in FNCG
					// also do not matched with any prefix
					// so it is a normal object
					*is_component = FALSE;
					*gottten_id = 0;
				}
				BOOL isExternalTexture(CKContext* ctx, CKTexture* texture, std::string* name) {
					CK_TEXTURE_SAVEOPTIONS options;
					if ((options = texture->GetSaveOptions()) == CKTEXTURE_USEGLOBAL)
						options = ctx->GetGlobalImagesSaveOptions();
					if (options != CKTEXTURE_EXTERNAL) return FALSE;

					for (int i = 0; (uint32_t)i < CONST_ExternalTextureList_Length; i++) {
						if (*name == CONST_ExternalTextureList[i])
							return TRUE;
					}
					return FALSE;
				}
				void safelyGetName(CKObject* obj, std::string* name) {
					if (obj->GetName() != NULL) {
						*name = obj->GetName();
						return;
					}

					utils::string_helper::StdstringPrintf(name, "noname_%d", obj->GetID());
				}
				uint32_t tryAddWithIndex(std::vector<CK_ID>* list, CK_ID newValue) {
					auto gotten = std::find(list->begin(), list->end(), newValue);
					uint32_t res = 0;
					if (gotten == list->end()) {
						res = list->size();
						list->push_back(newValue);
					} else res = gotten - list->begin();
					return res;
				}

			}
		}
	}
}