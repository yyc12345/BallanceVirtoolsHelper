#pragma once

#include "../../stdafx.h"
#include <filesystem>
#include <string>
#include <fstream>
#include <zlib.h>
#include <unzip.h>
#include <zip.h>
#include <iowin32.h>

#define BVH_ZIP_BUFFER 2048

namespace bvh {
	namespace utils {
		namespace zip_helper {

			void Compress(std::filesystem::path* filepath, std::filesystem::path* folder);
			BOOL doCompressCurrentFolder(zipFile* zip_file, std::filesystem::path* basefolder, std::filesystem::path* nowFolder);

			void Decompress(std::filesystem::path* filepath, std::filesystem::path* folder);
			BOOL doExtractCurrentFile(unzFile* zip_file, std::string* inner_name, std::filesystem::path* folder);

		}
	}
}
