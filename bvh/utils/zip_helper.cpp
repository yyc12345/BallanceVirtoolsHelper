// Zlib function should be imported first
// otherwise compiler will throw error
#include "../../stdafx.h"
#define ZLIB_WINAPI
#include <zconf.h>

#include "zip_helper.h"
#include "string_helper.h"

namespace bvh {
	namespace utils {
		namespace zip_helper {

			void Compress(std::filesystem::path* filepath, std::filesystem::path* folder) {
				zlib_filefunc_def ffunc;
				fill_fopen_filefunc(&ffunc);
				zipFile zip_file = NULL;
				zip_file = zipOpen2(filepath->string().c_str(), APPEND_STATUS_CREATE, NULL, &ffunc);
				if (zip_file == NULL) throw std::bad_alloc();

				if (!do_compress_currentFolder(&zip_file, folder, folder))
					throw std::bad_alloc();

				zipClose(zip_file, NULL);
			}

			BOOL do_compress_currentFolder(zipFile* zip_file, std::filesystem::path* basefolder, std::filesystem::path* nowFolder) {
				WIN32_FIND_DATA filedata;
				zip_fileinfo zi;
				std::filesystem::path newFolder, relativeFolder;
				std::ifstream file_get;
				std::streamsize file_get_count;
				std::string win32_query_filepath, filepath_builder;
				char buffer[BVH_ZIP_BUFFER];
				memset(&filedata, 0, sizeof(WIN32_FIND_DATA)); ZeroMemory(&zi, sizeof(zip_fileinfo));
				utils::string_helper::StdstringPrintf(&win32_query_filepath, "%s\\*", nowFolder->string().c_str());
				HANDLE fhandle = FindFirstFile(win32_query_filepath.c_str(), &filedata);
				if (fhandle == INVALID_HANDLE_VALUE)
					//error throw it
					return FALSE;


				while (TRUE) {
					if (filedata.cFileName[0] != '.') {	//filter for .. and .
						newFolder = *nowFolder;
						newFolder /= filedata.cFileName;
						relativeFolder = std::filesystem::relative(newFolder, *basefolder);

						// process folder
						if (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
							// create blank folder first
							utils::string_helper::StdstringPrintf(&filepath_builder, "%s/", relativeFolder.string().c_str());
							if (zipOpenNewFileInZip3(*zip_file, filepath_builder.c_str(), &zi,
								NULL, 0, NULL, 0, NULL,
								Z_DEFLATED, 9, 0,
								-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
								NULL, 0) != ZIP_OK)
								return FALSE;
							zipCloseFileInZip(*zip_file);

							// iterate sub folder file
							if (!do_compress_currentFolder(zip_file, basefolder, &newFolder))
								return FALSE;
						} else {
							// process file
							if (zipOpenNewFileInZip3(*zip_file, relativeFolder.string().c_str(), &zi,
								NULL, 0, NULL, 0, NULL,
								Z_DEFLATED, 9, 0,
								-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
								NULL, 0) != ZIP_OK)
								return FALSE;
							file_get.open(newFolder.string().c_str(), std::ios_base::in | std::ios_base::binary);

							while (TRUE) {
								if (file_get.peek(), file_get.eof()) break;

								file_get.read(buffer, BVH_ZIP_BUFFER);
								file_get_count = file_get.gcount();
								zipWriteInFileInZip(*zip_file, buffer, file_get_count);
							}

							file_get.close();
							zipCloseFileInZip(*zip_file);
						}
					}

					// do next get
					if (!FindNextFile(fhandle, &filedata))
						// no more data
						break;

				}

				FindClose(fhandle);
				return TRUE;
			}


			void Decompress(std::filesystem::path* filepath, std::filesystem::path* _folder) {
				zlib_filefunc_def ffunc;
				unz_global_info gi;
				unz_file_info file_info;

				std::string inner_filename(MAX_PATH, '\0');
				std::filesystem::path folder;
				folder = *_folder;

				fill_fopen_filefunc(&ffunc);
				unzFile zip_file = NULL;
				zip_file = unzOpen2(filepath->string().c_str(), &ffunc);
				if (zip_file == NULL) throw std::bad_alloc();

				// read global information
				if (unzGetGlobalInfo(zip_file, &gi) != UNZ_OK)
					throw std::bad_alloc();

				// iterate file
				for (uLong i = 0; i < gi.number_entry; i++) {
					if (unzGetCurrentFileInfo(zip_file, &file_info, inner_filename.data(), MAX_PATH, NULL, 0, NULL, 0) != UNZ_OK)
						throw std::bad_alloc();

					if (!do_extract_currentfile(&zip_file, &inner_filename, &folder))
						throw std::bad_alloc();

					// to next file and check it.
					if ((i + 1) < gi.number_entry) {
						if (unzGoToNextFile(zip_file) != UNZ_OK)
							throw std::bad_alloc();
					}
				}

				unzClose(zip_file);
			}

			BOOL do_extract_currentfile(unzFile* zip_file, std::string* inner_name, std::filesystem::path* folder) {
				std::filesystem::path real_path;
				std::ofstream fs;
				char buffer[BVH_ZIP_BUFFER];
				int read_result;

				int count = inner_name->size();
				if ((*inner_name)[count - 1] == '\\' || (*inner_name)[count - 1] == '/') {
					//empty folder
					real_path = *folder / *inner_name;
					std::filesystem::create_directories(real_path);

				} else {
					//file
					//make sure its folder is existed
					real_path = *folder / *inner_name;
					std::filesystem::create_directories(real_path.parent_path());

					//then process file
					if (unzOpenCurrentFile(*zip_file) != UNZ_OK)
						return FALSE;

					fs.open(real_path, std::ios_base::out | std::ios_base::binary);
					while (TRUE) {
						read_result = unzReadCurrentFile(*zip_file, buffer, BVH_ZIP_BUFFER);
						if (read_result < 0)
							return FALSE;
						else if (read_result > 0) {
							fs.write(buffer, read_result);
						} else break; // no any more data
					}
					unzCloseCurrentFile(*zip_file);

					fs.close();
				}

				return TRUE;
			}



		}
	}
}
