#include "bmfile.h"
#include "../../utils/zip_helper.h"
#include "../../mfcwindows/bmfile_export.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"
#include <unordered_map>
#include <cuchar>

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {

				void ExportBM(utils::ParamPackage* pkg) {
					// ============================================get file
					std::string filepath;
					std::filesystem::path file, temp, tempTexture;
					mfcwindows::BMFileExport* bm_export_window = new mfcwindows::BMFileExport(pkg);
					if (bm_export_window->DoModal() != IDOK) {
						delete bm_export_window;
						pkg->error_proc->SetExecutionResult(FALSE, "You cancel this process.");
						return;
					}
					file = bm_export_window->OUT_File;
					// get temp folder
					utils::win32_helper::GetTempFolder(&temp);
					temp /= "a6694fa9ca1c46588cf4b6e6d376c3bd";	// a6694fa9ca1c46588cf4b6e6d376c3bd is guid
					tempTexture = temp / "Texture";
					//clean temp folder
					std::filesystem::remove_all(temp);
					std::filesystem::create_directory(temp);
					std::filesystem::create_directory(tempTexture);

					// ============================================write file
					CKContext* ctx = pkg->plgif->GetCKContext();
					std::vector<CK_ID> objectList, meshList, materialList, textureList;
					std::ofstream findex, fobject, fmesh, fmaterial, ftexture;

					//used by grouping catch
					CKGroup* grouping_data_target;
					std::string* grouping_data_name;
					int grouping_group_count;
					//used by filter
					std::string filter_name;
					CKGroup* filter_group;
					CKBeObject* filter_groupItem;
					CK3dEntity* filter_3dentity;
					int groupCount;
					//used by index
					uint32_t bm_version;
					uint64_t index_offset;
					FILE_INDEX_TYPE index_type;
					std::string index_name;
					//used by object
					CK3dEntity* exportObject;
					BOOL object_isComponent, object_isForcedNoComponent, object_isHidden;
					uint32_t object_meshIndex;
					VxMatrix object_worldMatrix;
					uint64_t object_filepointer_start, object_filepointer_end;
					uint32_t object_group_list_count;
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
					CKTexture* material_tryTexture;
					BOOL material_useTexture;
					uint32_t material_textureIndex;
					//used by texture
					CKTexture* exportTexture;
					std::filesystem::path texture_origin, texture_target;
					std::string texture_filename;
					BOOL texture_isExternal;

					// forced no component group
					CKGroup* ck_instance_forcedNoComponentGroup = NULL;
					if (!pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.empty()) {
						// if no needed group. just skip this. it will be set as NULL;
						ck_instance_forcedNoComponentGroup = (CKGroup*)ctx->GetObjectByNameAndClass((CKSTRING)pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str(), CKCID_GROUP, NULL);
					}
					std::vector<CK_ID> forcedNoComponentGroup;
					if (ck_instance_forcedNoComponentGroup != NULL) {
						for (int i = 0, count = ck_instance_forcedNoComponentGroup->GetObjectCount(); i < count; i++) {
							forcedNoComponentGroup.push_back(ck_instance_forcedNoComponentGroup->GetObjectA(i)->GetID());
						}
					}

					// get all grouping data
					std::unordered_multimap<CK_ID, std::string*> grouping_data;
					std::vector<std::string*> alloced_grouping_string;
					XObjectPointerArray groupArray = ctx->GetObjectListByType(CKCID_GROUP, FALSE);
					for (auto item = groupArray.Begin(); item != groupArray.End(); ++item) {
						grouping_data_target = (CKGroup*)(*item);
						if (grouping_data_target->GetName() == NULL) continue; // skip group with blank name
						if ((!pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.empty()) &&
							pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName == grouping_data_target->GetName())
							continue; // we don't need NoComponentGroup data

						grouping_data_name = new std::string();
						*grouping_data_name = grouping_data_target->GetName();

						// add into alloc vector
						alloced_grouping_string.push_back(grouping_data_name);

						// iterate group object
						grouping_group_count = grouping_data_target->GetObjectCount();
						for (int i = 0; i < grouping_group_count; ++i)
							grouping_data.insert({ grouping_data_target->GetObject(i)->GetID(), grouping_data_name });

					}


					// filter obj first
					switch (bm_export_window->OUT_Mode) {
						case 0:
							// obj
							objectList.push_back(bm_export_window->OUT_Target);
							break;
						case 1:
							// group
							filter_group = (CKGroup*)ctx->GetObjectA(bm_export_window->OUT_Target);
							groupCount = filter_group->GetObjectCount();
							for (int i = 0; i < groupCount; i++) {
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

					findex.open(temp / "index.bm", std::ios_base::out | std::ios_base::binary);

					//write version header
					bm_version = BM_FILE_VERSION;
					writeInt(&findex, &bm_version);

					//write object
					fobject.open(temp / "object.bm", std::ios_base::out | std::ios_base::binary);
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
						getComponent(&forcedNoComponentGroup, exportObject->GetID(), &index_name, &object_isComponent, &object_isForcedNoComponent, &object_meshIndex);
						writeBool(&fobject, &object_isComponent);
						writeBool(&fobject, &object_isForcedNoComponent);
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
						object_group_list_count = 0;
						object_filepointer_start = fobject.tellp();
						writeInt(&fobject, &object_group_list_count);
						auto grouping_data_range = grouping_data.equal_range(exportObject->GetID());
						for (auto it = grouping_data_range.first; it != grouping_data_range.second; ++it) {
							writeString(&fobject, it->second);
							++object_group_list_count;
						}
						// backups current position, back to count position, write it and back to current position
						object_filepointer_end = fobject.tellp();
						fobject.seekp(object_filepointer_start);
						writeInt(&fobject, &object_group_list_count);
						fobject.seekp(object_filepointer_end);

						if (!object_isComponent) {
							object_meshIndex = tryAddWithIndex(&meshList, exportObject->GetMesh(0)->GetID());
						}
						writeInt(&fobject, &object_meshIndex);
					}
					fobject.close();

					fmesh.open(temp / "mesh.bm", std::ios_base::out | std::ios_base::binary);
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

					fmaterial.open(temp / "material.bm", std::ios_base::out | std::ios_base::binary);
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

						material_tryTexture = exportMaterial->GetTexture();
						material_useTexture = material_tryTexture != NULL && material_tryTexture->GetSlotFileName(0) != NULL;
						writeBool(&fmaterial, &material_useTexture);
						if (material_useTexture) {
							material_textureIndex = tryAddWithIndex(&textureList, material_tryTexture->GetID());
						} else material_textureIndex = 0;
						writeInt(&fmaterial, &material_textureIndex);
					}
					fmaterial.close();

					ftexture.open(temp / "texture.bm", std::ios_base::out | std::ios_base::binary);
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
						texture_origin = exportTexture->GetSlotFileName(0);
						texture_filename = texture_origin.filename().string();
						texture_target = tempTexture;
						texture_target /= texture_filename;
						writeString(&ftexture, &texture_filename);

						texture_isExternal = isExternalTexture(ctx, exportTexture, &texture_filename);
						writeBool(&ftexture, &texture_isExternal);
						if (!texture_isExternal) {
							//try copy original file. if fail, use virtools internal save function.
							if (!CopyFile(texture_origin.string().c_str(), texture_target.string().c_str(), FALSE)) {
								exportTexture->SaveImage((char*)texture_target.string().c_str(), 0, FALSE);
							}
						}
					}
					ftexture.close();
					findex.close();

					// release alloced grouping data string
					for (auto it = alloced_grouping_string.begin(); it != alloced_grouping_string.end(); it++) {
						delete ((std::string*)(*it));
					}

					// ============================================write zip
					utils::zip_helper::Compress(&file, &temp);
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
					size_t convCount = 0, convPos = 0, convAll = strlen(utf8str.c_str());
					uint32_t length = 0;
					std::u32string bmstr;
					char32_t convedchar;
					bmstr.reserve(utf8str.size());	// pre-alloc for avoiding mem realloc

					while (convPos < convAll) {
						convCount = mbrtoc32(&convedchar, &(utf8str[convPos]), convAll - convPos, &state);
						if (convPos == -1) continue;
						bmstr.append(1, convedchar);
						convPos += convCount;
						length++;
					}
					
					// write length
					writeInt(fs, &length);

					// write data
					fs->write((char*)bmstr.data(), length * sizeof(char32_t));

				}
				BOOL isValidObject(CK3dEntity* obj) {
					CKMesh* mesh = obj->GetCurrentMesh();
					if (mesh == NULL) return FALSE;					//no mesh
					if (mesh->GetFaceCount() == 0) return FALSE;	//no face
					return TRUE;
				}
				void getComponent(std::vector<CK_ID>* grp, CK_ID objId, std::string* name, BOOL* is_component, BOOL* is_forced_no_component, uint32_t* gottten_id) {
					*is_component = FALSE;
					*is_forced_no_component = FALSE;
					*gottten_id = 0;
					for (uint32_t i = 0; i < CONST_ExternalComponent_Length; i++) {
						if (utils::string_helper::StdstringStartsWith(*name, CONST_ExternalComponent[i])) {
							//comfirm component
							*is_component = TRUE;
							*gottten_id = i;
							break;
						}
					}

					if (std::find(grp->begin(), grp->end(), objId) != grp->end()) {
						// change it to forced no component
						*is_component = FALSE;
						*is_forced_no_component = TRUE;
					}
				}
				BOOL isExternalTexture(CKContext* ctx, CKTexture* texture, std::string* name) {
					CK_TEXTURE_SAVEOPTIONS options;
					if ((options = texture->GetSaveOptions()) == CKTEXTURE_USEGLOBAL)
						options = ctx->GetGlobalImagesSaveOptions();
					if (options != CKTEXTURE_EXTERNAL) return FALSE;

					for (int i = 0; i < CONST_ExternalTextureList_Length; i++) {
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