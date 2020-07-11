#if !defined(_YYCDLL_MAPPING_BM_H__IMPORTED_)
#define _YYCDLL_MAPPING_BM_H__IMPORTED_

#include "../stdafx.h"
#include <filesystem>
#include <string>
#include <fstream>
#include <unzip.h>

#define BM_FILE_VERSION 10

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
			void ReadInt(std::ifstream* fs, uint32_t* num);
			void ReadInt(std::ifstream* fs, uint64_t* num);
			void ReadFloat(std::ifstream* fs, float* num);
			void ReadString(std::ifstream* fs, std::string* str);

			BOOL ExportBM();


			namespace zip_handle {
				void Compress(std::filesystem::path* filepath, std::filesystem::path* folder);


				void Decompress(std::filesystem::path* filepath, std::filesystem::path* folder);
				BOOL do_extract_currentfile(unzFile* zipfile, std::string* inner_name, std::filesystem::path* folder);
			}
		}
	}
}

#endif