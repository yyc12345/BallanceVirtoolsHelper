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
					bmx_file = bm_export_window->OUT_File.c_str();
					// get temp folder path
					utils::win32_helper::GetTempFolder(pkg->plgif->GetCKContext(), &temp_folder);
					temp_folder /= "a6694fa9ca1c46588cf4b6e6d376c3bd";	// a6694fa9ca1c46588cf4b6e6d376c3bd is guid
					temp_texture_folder = temp_folder / "Texture";
					// clean temp folder
					std::filesystem::remove_all(temp_folder);
					std::filesystem::create_directory(temp_folder);
					std::filesystem::create_directory(temp_texture_folder);

					// ============================================
					// allocate some variables and preparing exporting work
					CKContext* ctx = pkg->plgif->GetCKContext();
					std::vector<CK3dEntity*> objectList;
					std::vector<CKMesh*> meshList;
					std::vector<CKMaterial*> materialList;
					std::vector<CKTexture*> textureList;
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
					//used by object
					CK3dEntity* exportObject;
					BOOL object_isComponent, object_isHidden;
					uint32_t object_meshIndex;
					VxMatrix object_worldMatrix;
					uint64_t object_chunkPtrHeader, object_chunkPtrTail;
					uint32_t object_groupListCount;
					//used by mesh
					CKMesh* exportMesh;
					CKDWORD mesh_scanStride;
					VxVector* mesh_p3dVector;
					Vx2DVector* mesh_p2dVector;
					std::vector<VxVector> mesh_3dVectorList;
					std::vector<Vx2DVector> mesh_2dVectorList;
					std::vector<BM_FACE_PROTOTYPE> mesh_faceList;
					WORD* mesh_faceIndices;
					uint32_t mesh_count;
					CKMaterial* mesh_faceMaterial;
					//used by material
					CKMaterial* exportMaterial;
					VxColor material_color;
					float material_value;
					CKTexture* material_tryGottonTexture;
					BOOL material_useTexture, material_alphaProp;
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
							objectList.push_back((CK3dObject*)ctx->GetObjectA(bm_export_window->OUT_Target));
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
											objectList.push_back((CK3dEntity*)filter_groupItem);
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
									objectList.push_back(filter_3dentity);
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
					writeData<uint32_t>(&findex, &index_bmVersion);

					//write object
					fobject.open(temp_folder / "object.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
						exportObject = *iter;
						//write index first
						putIndexHeader(&findex, exportObject, FILE_INDEX_TYPE__OBJECT, fobject.tellp());

						//write object
						getComponent(&utils_fncgSet, exportObject, &object_isComponent, &object_meshIndex);
						writeBool(&fobject, &object_isComponent);
						object_isHidden = exportObject->IsVisible() == CKHIDE;
						writeBool(&fobject, &object_isHidden);
						object_worldMatrix = exportObject->GetWorldMatrix();
						writeData<VxMatrix>(&fobject, &object_worldMatrix);

						// write group list
						// record length position and ready back to there to write count
						object_groupListCount = 0;
						object_chunkPtrHeader = fobject.tellp();
						writeData<uint32_t>(&fobject, &object_groupListCount);
						auto grouping_data_range = grouping_data.equal_range(exportObject->GetID());
						for (auto& it = grouping_data_range.first; it != grouping_data_range.second; ++it) {
							writeString(&fobject, it->second);
							++object_groupListCount;
						}
						// backups current position, back to count position, write it and back to current position
						object_chunkPtrTail = fobject.tellp();
						fobject.seekp(object_chunkPtrHeader);
						writeData<uint32_t>(&fobject, &object_groupListCount);
						fobject.seekp(object_chunkPtrTail);

						// if this object is normal object, we need to register it in mesh list
						// and write corresponding index
						// otherwise, write corresponding component id gotten from
						// previous function calling
						if (!object_isComponent) {
							object_meshIndex = tryAddWithPtr<CKMesh>(&meshList, exportObject->GetMesh(0));
						}
						writeData<uint32_t>(&fobject, &object_meshIndex);
					}
					fobject.close();

					// write mesh
					fmesh.open(temp_folder / "mesh.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = meshList.begin(); iter != meshList.end(); iter++) {
						exportMesh = *iter;
						//write index first
						putIndexHeader(&findex, exportMesh, FILE_INDEX_TYPE__MESH, fmesh.tellp());

						//write mesh
						mesh_count = exportMesh->GetVertexCount();
						mesh_3dVectorList.resize(mesh_count);
						mesh_2dVectorList.resize(mesh_count);

						writeData<uint32_t>(&fmesh, &mesh_count);
						mesh_p3dVector = (VxVector*)exportMesh->GetPositionsPtr(&mesh_scanStride);
						for (uint32_t i = 0; i < mesh_count; ++i) {
							memcpy(mesh_3dVectorList.data() + i, mesh_p3dVector, sizeof(VxVector));
							mesh_p3dVector = (VxVector*)((char*)mesh_p3dVector + mesh_scanStride);
						}
						writeVectorData<VxVector>(&fmesh, &mesh_3dVectorList);

						writeData<uint32_t>(&fmesh, &mesh_count);
						mesh_p2dVector = (Vx2DVector*)exportMesh->GetTextureCoordinatesPtr(&mesh_scanStride);
						for (uint32_t i = 0; i < mesh_count; ++i) {
							memcpy(mesh_2dVectorList.data() + i, mesh_p2dVector, sizeof(Vx2DVector));
							mesh_p2dVector = (Vx2DVector*)((char*)mesh_p2dVector + mesh_scanStride);
						}
						writeVectorData<Vx2DVector>(&fmesh, &mesh_2dVectorList);

						writeData<uint32_t>(&fmesh, &mesh_count);
						mesh_p3dVector = (VxVector*)exportMesh->GetNormalsPtr(&mesh_scanStride);
						for (uint32_t i = 0; i < mesh_count; ++i) {
							memcpy(mesh_3dVectorList.data() + i, mesh_p3dVector, sizeof(VxVector));
							mesh_p3dVector = (VxVector*)((char*)mesh_p3dVector + mesh_scanStride);
						}
						writeVectorData<VxVector>(&fmesh, &mesh_3dVectorList);

						mesh_count = exportMesh->GetFaceCount();
						mesh_faceList.resize(mesh_count);
						writeData<uint32_t>(&fmesh, &mesh_count);

						mesh_faceIndices = exportMesh->GetFacesIndices();
						for (uint32_t i = 0; i < mesh_count; i++) {
							BM_FACE_PROTOTYPE* pFace = mesh_faceList.data() + i;

							// indices
							pFace->indices.data.v1 = pFace->indices.data.vt1 = pFace->indices.data.vn1 = mesh_faceIndices[0];
							pFace->indices.data.v2 = pFace->indices.data.vt2 = pFace->indices.data.vn2 = mesh_faceIndices[1];
							pFace->indices.data.v3 = pFace->indices.data.vt3 = pFace->indices.data.vn3 = mesh_faceIndices[2];
							
							// mtl data
							mesh_faceMaterial = exportMesh->GetFaceMaterial(i);
							pFace->use_material = mesh_faceMaterial != NULL;
							
							if (pFace->use_material) {
								pFace->material_index = tryAddWithPtr<CKMaterial>(&materialList, mesh_faceMaterial);
							} else pFace->material_index = UINT32_C(0);
							

							// inc self
							mesh_faceIndices = mesh_faceIndices + 3;
						}
						writeVectorData<BM_FACE_PROTOTYPE>(&fmesh, &mesh_faceList);

					}
					fmesh.close();

					// write material
					fmaterial.open(temp_folder / "material.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = materialList.begin(); iter != materialList.end(); iter++) {
						exportMaterial = *iter;
						//write index first
						putIndexHeader(&findex, exportMaterial, FILE_INDEX_TYPE__MATERIAL, fmaterial.tellp());

						//write material
#define writeColor writeArrayData<float>(&fmaterial, &(material_color.r), UINT32_C(3));
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
						writeData<float>(&fmaterial, &material_value);

						material_alphaProp = exportMaterial->AlphaTestEnabled();
						writeBool(&fmaterial, &material_alphaProp);
						material_alphaProp = exportMaterial->AlphaBlendEnabled();
						writeBool(&fmaterial, &material_alphaProp);
						material_alphaProp = exportMaterial->ZWriteEnabled();
						writeBool(&fmaterial, &material_alphaProp);
						material_alphaProp = exportMaterial->IsTwoSided();
						writeBool(&fmaterial, &material_alphaProp);

						// try get texture
						material_tryGottonTexture = exportMaterial->GetTexture();
						material_useTexture = material_tryGottonTexture != NULL && material_tryGottonTexture->GetSlotFileName(0) != NULL;
						writeBool(&fmaterial, &material_useTexture);

						if (material_useTexture) {
							material_textureIndex = tryAddWithPtr<CKTexture>(&textureList, material_tryGottonTexture);
						} else material_textureIndex = 0;
						writeData<uint32_t>(&fmaterial, &material_textureIndex);
					}
					fmaterial.close();

					// write texture
					ftexture.open(temp_folder / "texture.bm", std::ios_base::out | std::ios_base::binary);
					for (auto iter = textureList.begin(); iter != textureList.end(); iter++) {
						exportTexture = *iter;
						//write index first
						putIndexHeader(&findex, exportTexture, FILE_INDEX_TYPE__TEXTURE, ftexture.tellp());

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
					writeData<uint32_t>(fs, &c32Length);

					// write data
					fs->write((char*)bmstr.data(), (std::streamsize)c32Length * sizeof(char32_t));

				}
				BOOL isValidObject(CK3dEntity* obj) {
					CKMesh* mesh = obj->GetCurrentMesh();
					if (mesh == NULL) return FALSE;					//no mesh
					if (mesh->GetFaceCount() == 0) return FALSE;	//no face
					return TRUE;
				}
				void getComponent(std::unordered_set<CK_ID>* fncgSet, CK3dEntity* obj, BOOL* is_component, uint32_t* gottten_id) {
					// get id and name first
					CK_ID objId = obj->GetID();
					std::string name;
					safelyGetName(obj, &name);

					// we check it whether in FNCG first
					if (fncgSet->find(objId) != fncgSet->end()) {
						// got, return it as normal object immediately
						*is_component = FALSE;
						*gottten_id = 0;
						return;
					}

					// if it is not in FNCG, we should check it by tools chain standard prefix
					for (uint32_t i = 0; i < CONST_ExternalComponent_Length; i++) {
						if (utils::string_helper::StdstringStartsWith(name, CONST_ExternalComponent[i])) {
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
				void putIndexHeader(std::ofstream* findex, CKObject* data, FILE_INDEX_TYPE t, uint64_t offset) {
					// name
					std::string obj_name;
					safelyGetName(data, &obj_name);
					writeString(findex, &obj_name);
					// type
					uint8_t index_type = (uint8_t)t;
					writeData<uint8_t>(findex, (uint8_t*)&index_type);
					// offset
					writeData<uint64_t>(findex, &offset);
				}
				template<typename T>
				uint32_t tryAddWithPtr(std::vector<T*>* list, T* newValue) {
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