#if !defined(_YYCDLL_MAPPING_BM_H__IMPORTED_)
#define _YYCDLL_MAPPING_BM_H__IMPORTED_

#include "../stdafx.h"
#include <filesystem>
#include <unzip.h>

namespace func_namespace {
	namespace mapping {
		namespace BM {
			
			BOOL ImportBM();
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