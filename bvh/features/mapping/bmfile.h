#pragma once

#include "../../../stdafx.h"
#include "../../utils/param_package.h"
#include <fstream>
#include <unordered_set>
#include <map>
#include <vector>
#include <Ge2Virtools.h>

#define BM_FILE_VERSION 14

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {

				enum FILE_INDEX_TYPE {
					FILE_INDEX_TYPE__OBJECT = 0,
					FILE_INDEX_TYPE__MESH = 1,
					FILE_INDEX_TYPE__MATERIAL = 2,
					FILE_INDEX_TYPE__TEXTURE = 3
				};

				struct FILE_INDEX_HELPER {
					std::string name;
					CKObject* obj;
					uint64_t offset;
				};

#pragma pack(1)
				union COMPONENT_FACE_PROTOTYPE {
					struct COMPONENT_FACE_PROTOTYPE_DATA {
						uint32_t v1, vn1;
						uint32_t v2, vn2;
						uint32_t v3, vn3;
					}data;
					uint32_t vbin[sizeof(COMPONENT_FACE_PROTOTYPE_DATA) / sizeof(uint32_t)];
				};
				struct BM_FACE_PROTOTYPE {
					union {
						struct BM_FACE_PROTOTYPE_DATA_V {
							uint32_t v1, vt1, vn1;
							uint32_t v2, vt2, vn2;
							uint32_t v3, vt3, vn3;
						}data;
						uint32_t vbin[sizeof(BM_FACE_PROTOTYPE_DATA_V) / sizeof(uint32_t)];
					}indices;

					uint8_t use_material;
					uint32_t material_index;
				};
#pragma pack()

				extern const uint32_t CONST_ExternalTextureList_Length;
				extern const char* CONST_ExternalTextureList[];
				extern const uint32_t CONST_ExternalComponent_Length;
				extern const char* CONST_ExternalComponent[];

				void ImportBM(utils::ParamPackage* pkg);
				template<typename T>
				inline void readData(std::ifstream* fs, T* data) {
					fs->read((char*)data, (std::streamsize)sizeof(T));
				}
				template<typename T>
				inline void readArrayData(std::ifstream* fs, T* arr, uint32_t count) {
					fs->read((char*)arr, ((std::streamsize)sizeof(T)) * ((std::streamsize)count));
				}
				template<typename T>
				inline void readVectorData(std::ifstream* fs, std::vector<T>* vec, uint32_t count) {
					vec->resize(count);
					fs->read((char*)vec->data(), ((std::streamsize)sizeof(T)) * ((std::streamsize)count));
				}
				void readBool(std::ifstream* fs, BOOL* boolean);
				void readString(std::ifstream* fs, std::string* str);
				void loadExternalTexture(std::string* name, CKTexture* texture, utils::ParamPackage* pkg);
				CKMesh* loadComponenetMesh(CKContext* ctx, Export2Virtools* exporter, VirtoolsTransitionMesh* converter, uint32_t model_index);
				BOOL isComponentInStandard(std::string* name);
				CKObject* userCreateCKObject(CKContext* ctx, Export2Virtools* exporter, FILE_INDEX_TYPE cls, const char* name, void* pkey, void* extraData, BOOL use_rename, BOOL* is_existed);

				void ExportBM(utils::ParamPackage* pkg);
				void writeBool(std::ofstream* fs, BOOL* boolean);
				void writeInt(std::ofstream* fs, uint8_t* num);
				void writeInt(std::ofstream* fs, uint32_t* num);
				void writeInt(std::ofstream* fs, uint64_t* num);
				void writeFloat(std::ofstream* fs, float* num);
				void writeString(std::ofstream* fs, std::string* str);
				BOOL isValidObject(CK3dEntity* obj);
				void getComponent(std::unordered_set<CK_ID>* grp, CK_ID objId, std::string* name, BOOL* is_component, uint32_t* gottten_id);
				BOOL isExternalTexture(CKContext* ctx, CKTexture* texture, std::string* name);
				void safelyGetName(CKObject* obj, std::string* name);
				uint32_t tryAddWithIndex(std::vector<CK_ID>* list, CK_ID newValue);

				void FixTexture(utils::ParamPackage* pkg);

			}
		}
	}
}
