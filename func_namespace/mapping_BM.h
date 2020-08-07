#if !defined(_YYCDLL_MAPPING_BM_H__IMPORTED_)
#define _YYCDLL_MAPPING_BM_H__IMPORTED_

#include "../stdafx.h"
#include <filesystem>
#include <string>
#include <fstream>
#include <unzip.h>
#include <zip.h>

#define BM_FILE_VERSION 11

namespace func_namespace {
	namespace mapping {
		namespace BM {

#pragma region struct defination

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
			
#pragma endregion

			BOOL ImportBM();
			void ReadBool(std::ifstream* fs, BOOL* boolean);
			void ReadInt(std::ifstream* fs, uint8_t* num);
			void ReadInt(std::ifstream* fs, uint32_t* num);
			void ReadInt(std::ifstream* fs, uint64_t* num);
			void ReadFloat(std::ifstream* fs, float* num);
			void ReadString(std::ifstream* fs, std::string* str);
			void LoadExternalTexture(std::string* name, CKTexture* texture);
			void LoadComponenetMesh(CK3dEntity* obj, CKContext* ctx, uint32_t index);

			BOOL ExportBM();
			void WriteBool(std::ofstream* fs, BOOL* boolean);
			void WriteInt(std::ofstream* fs, uint8_t* num);
			void WriteInt(std::ofstream* fs, uint32_t* num);
			void WriteInt(std::ofstream* fs, uint64_t* num);
			void WriteFloat(std::ofstream* fs, float* num);
			void WriteString(std::ofstream* fs, std::string* str);
			BOOL IsValidObject(CK3dEntity* obj);
			void GetComponent(std::vector<CK_ID>* grp, CK_ID objId, std::string* name, BOOL* is_component, BOOL* is_forced_no_component, uint32_t* gottten_id);
			BOOL IsExternalTexture(CKContext* ctx, CKTexture* texture, std::string* name);
			void SafeGetName(CKObject* obj, std::string* name);
			uint32_t TryAddWithIndex(std::vector<CK_ID>* list, CK_ID newValue);

			BOOL FixBlenderTexture();

			namespace zip_handle {
				void Compress(std::filesystem::path* filepath, std::filesystem::path* folder);
				BOOL do_compress_currentFolder(zipFile* zip_file, std::filesystem::path* basefolder, std::filesystem::path* nowFolder);

				void Decompress(std::filesystem::path* filepath, std::filesystem::path* folder);
				BOOL do_extract_currentfile(unzFile* zip_file, std::string* inner_name, std::filesystem::path* folder);
			}
		}
	}
}

#endif