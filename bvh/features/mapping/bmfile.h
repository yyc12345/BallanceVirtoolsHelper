#pragma once

#include "../../../stdafx.h"
#include "../../utils/param_package.h"
#include <fstream>
#include <unordered_set>
#include <vector>

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
					CK_ID id;
					uint64_t offset;
				};

				extern const uint32_t CONST_ExternalTextureList_Length;
				extern const char* CONST_ExternalTextureList[];
				extern const uint32_t CONST_ExternalComponent_Length;
				extern const char* CONST_ExternalComponent[];

				void ImportBM(utils::ParamPackage* pkg);
				void readBool(std::ifstream* fs, BOOL* boolean);
				void readInt(std::ifstream* fs, uint8_t* num);
				void readInt(std::ifstream* fs, uint32_t* num);
				void readInt(std::ifstream* fs, uint64_t* num);
				void readFloat(std::ifstream* fs, float* num);
				void readString(std::ifstream* fs, std::string* str);
				void loadExternalTexture(std::string* name, CKTexture* texture, utils::ParamPackage* pkg);
				void loadComponenetMesh(CK3dEntity* obj, CKContext* ctx, uint32_t index);
				BOOL isComponentInStandard(std::string* name);

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
