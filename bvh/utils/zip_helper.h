#pragma once

#include "../../stdafx.h"
#include <filesystem>
#include <string>
#include <fstream>
#include <zlib.h>
#include <unzip.h>
#include <zip.h>
#include <iowin32.h>

#define BVH_ZIP_BUFFER 65526
#define BVH_ZIP_GLOBAL_COMMENT (u8"Use BM Spec 1.4")
#define BVH_ZIP_FLAG_UNICODE (1 << 11)

namespace bvh {
	namespace utils {
		namespace zip_helper {

			void Compress(std::filesystem::path* filepath, std::filesystem::path* folder);
			BOOL doCompressCurrentFolder(zipFile* zip_file, std::filesystem::path* basefolder, std::filesystem::path* nowFolder, char* file_swap);

			void Decompress(std::filesystem::path* filepath, std::filesystem::path* folder);
			BOOL doExtractCurrentFile(unzFile* zip_file, std::string* inner_name, std::filesystem::path* folder, char* file_swap);

		}
	}
}
