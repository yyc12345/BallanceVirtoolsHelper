#pragma once

#include "../../stdafx.h"
#include <filesystem>
#include <string>
#include <fstream>
#include <unzip.h>
#include <zip.h>

#define BVH_ZIP_BUFFER 2048

namespace bvh {
	namespace utils {
		namespace zip_helper {

			void Compress(std::filesystem::path* filepath, std::filesystem::path* folder);
			BOOL do_compress_currentFolder(zipFile* zip_file, std::filesystem::path* basefolder, std::filesystem::path* nowFolder);

			void Decompress(std::filesystem::path* filepath, std::filesystem::path* folder);
			BOOL do_extract_currentfile(unzFile* zip_file, std::string* inner_name, std::filesystem::path* folder);

		}
	}
}
