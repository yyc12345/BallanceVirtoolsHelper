// Zlib function should be imported first
// otherwise compiler will throw error
#include "../../stdafx.h"
#define ZLIB_WINAPI
#include <zconf.h>

#include "zip_helper.h"
#include "string_helper.h"
#include <zlib.h>
#include <unzip.h>
#include <zip.h>
#include <iowin32.h>
#include <stdexcept>

// copy from zip.c
#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform depedent */
#endif

namespace bvh {
	namespace utils {
		namespace zip_helper {

			void Compress(std::filesystem::path* filepath, std::filesystem::path* folder) {
				zlib_filefunc64_def ffunc;
				zipFile zip_file = NULL;
				fill_win32_filefunc64W(&ffunc);
				zip_file = zipOpen2_64(filepath->wstring().c_str(), APPEND_STATUS_CREATE, NULL, &ffunc);
				if (zip_file == NULL) throw std::logic_error("Fail to opne zip file.");

				char* file_swap = (char*)malloc(BVH_ZIP_BUFFER * sizeof(char));
				if (file_swap == NULL)
					throw std::bad_alloc();
				if (!doCompressCurrentFolder(&zip_file, folder, folder, file_swap))
					throw std::logic_error("Fail to compress zip file");
				free(file_swap);

				zipClose(zip_file, BVH_ZIP_GLOBAL_COMMENT);
			}

			BOOL doCompressCurrentFolder(zipFile* zip_file, std::filesystem::path* base_folder, std::filesystem::path* current_folder, char* file_swap) {
				// init zip data struct
				zip_fileinfo zi;
				ZeroMemory(&zi, sizeof(zip_fileinfo));

				// alloc variables used in processing
				std::filesystem::path processing_folder_absolute, processing_folder_relative;
				std::ifstream cpoied_file;
				std::streamsize copied_file_gotten_char;
				std::string zip_internal_path_utf8;
				std::wstring zip_internal_path_acp;

				// build win32 fs query string
				std::wstring win32_query_filepath;
				utils::string_helper::StdwstringPrintf(&win32_query_filepath, L"%ls\\*", current_folder->wstring().c_str());
				// alloc win32 filesystem query values and query right now
				WIN32_FIND_DATAW filedata;
				memset(&filedata, 0, sizeof(WIN32_FIND_DATAW));
				HANDLE fhandle = FindFirstFileW(win32_query_filepath.c_str(), &filedata);
				if (fhandle == INVALID_HANDLE_VALUE)
					//error throw it
					return FALSE;


				while (TRUE) {
					if (filedata.cFileName[0] != L'.') {	//filter for .. and .
						// get relative folder
						processing_folder_absolute = *current_folder;
						processing_folder_absolute /= filedata.cFileName;
						processing_folder_relative = std::filesystem::relative(processing_folder_absolute, *base_folder);

						// distinguish folder or file
						if (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
							// process folder
							// generate folder path
							utils::string_helper::StdwstringPrintf(&zip_internal_path_acp, L"%ls/", processing_folder_relative.wstring().c_str());
							if (!utils::string_helper::Wstring2String(&zip_internal_path_acp, &zip_internal_path_utf8, CP_UTF8))
								throw std::runtime_error("Impossilble encoding convertion fail in zip compression.");

							// create blank folder
							if (zipOpenNewFileInZip4_64(
								*zip_file, // zip file
								zip_internal_path_utf8.c_str(),	// filepath in zip file
								&zi,	// zip file info
								NULL, 0, NULL, 0,	// extrafield_local and extrafield_global is useless
								NULL,	// no comment for file
								Z_DEFLATED, 9, 0,	// use deflated level 9 compress, not raw
								-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,	// copy from minizip.c
								NULL, 0,	// no password and no needed crc
								VERSIONMADEBY,	// copy from zip.c
								BVH_ZIP_FLAG_UNICODE, 	// we need set flag of unicode
								0) != ZIP_OK) {	// do not need zip64
								return FALSE;
							}
							zipCloseFileInZip(*zip_file);

							// iterate sub folder file
							if (!doCompressCurrentFolder(zip_file, base_folder, &processing_folder_absolute, file_swap))
								return FALSE;
						} else {
							// process file
							// file path do not need attach slash
							// conv encoding is enough
							zip_internal_path_acp = processing_folder_relative.wstring().c_str();
							if (!utils::string_helper::Wstring2String(&zip_internal_path_acp, &zip_internal_path_utf8, CP_UTF8))
								throw std::runtime_error("Impossilble encoding convertion fail in zip compression.");

							if (zipOpenNewFileInZip4_64(
								*zip_file, // zip file
								zip_internal_path_utf8.c_str(),	// filepath in zip file
								&zi,	// zip file info
								NULL, 0, NULL, 0,	// extrafield_local and extrafield_global is useless
								NULL,	// no comment for file
								Z_DEFLATED, 9, 0,	// use deflated level 9 compress, not raw
								-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,	// copy from minizip.c
								NULL, 0,	// no password and no needed crc
								VERSIONMADEBY,	// copy from zip.c
								BVH_ZIP_FLAG_UNICODE, 	// we need set flag of unicode
								0) != ZIP_OK) {	// do not need zip64
								return FALSE;
							}

							// copy file
							cpoied_file.open(processing_folder_absolute.wstring().c_str(), std::ios_base::in | std::ios_base::binary);

							while (TRUE) {
								if (cpoied_file.peek(), cpoied_file.eof()) break;

								cpoied_file.read(file_swap, BVH_ZIP_BUFFER);
								copied_file_gotten_char = cpoied_file.gcount();
								zipWriteInFileInZip(*zip_file, file_swap, (uint32_t)copied_file_gotten_char);
							}

							cpoied_file.close();
							zipCloseFileInZip(*zip_file);
						}
					}

					// try get next one
					if (!FindNextFileW(fhandle, &filedata))
						// no more files
						break;

				}

				FindClose(fhandle);
				return TRUE;
			}


			void Decompress(std::filesystem::path* filepath, std::filesystem::path* base_folder) {
				// init zip values
				zlib_filefunc64_def ffunc;
				unz_global_info gi;
				unz_file_info file_info;
				unzFile zip_file = NULL;
				fill_win32_filefunc64W(&ffunc);

				// alloc string for following use
				std::string zip_internal_path_utf8(BVH_MAX_PATH, '\0');
				std::wstring zip_internal_path_acp;

				// open zip file
				zip_file = unzOpen2_64(filepath->wstring().c_str(), &ffunc);
				if (zip_file == NULL) throw std::bad_alloc();
				// read global information
				if (unzGetGlobalInfo(zip_file, &gi) != UNZ_OK)
					throw std::bad_alloc();

				// allocate file swap
				char* file_swap = (char*)malloc(BVH_ZIP_BUFFER * sizeof(char));
				if (file_swap == NULL)
					throw std::bad_alloc();
				// iterate file entry
				for (uLong i = 0; i < gi.number_entry; i++) {
					// get entry
					// deliver NULL filename ptr with 0 length indicator
					// to try get filename size
					if (unzGetCurrentFileInfo(zip_file, &file_info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
						throw std::bad_alloc();
					// resize filename receiver and receive filename again
					zip_internal_path_utf8.resize(file_info.size_filename);
					if (unzGetCurrentFileInfo(zip_file, &file_info, zip_internal_path_utf8.data(), zip_internal_path_utf8.size(), NULL, 0, NULL, 0) != UNZ_OK)
						throw std::bad_alloc();

					// check unicode flag
					if (!(file_info.flag & BVH_ZIP_FLAG_UNICODE)) {
						throw std::invalid_argument("Zip file entry lost UNICODE flag.");
					}

					// convert filename and decompress file
					if (!utils::string_helper::String2Wstring(&zip_internal_path_utf8, &zip_internal_path_acp, CP_UTF8))
						throw std::runtime_error("Impossilble encoding convertion fail in zip compression.");
					if (!doExtractCurrentFile(&zip_file, &zip_internal_path_acp, base_folder, file_swap))
						throw std::logic_error("Fail to decompress file");

					// to next file and check it.
					if ((i + 1) < gi.number_entry) {
						if (unzGoToNextFile(zip_file) != UNZ_OK)
							throw std::bad_alloc();
					}
				}

				free(file_swap);
				unzClose(zip_file);
			}

			BOOL doExtractCurrentFile(unzFile* zip_file, std::wstring* relative_path, std::filesystem::path* base_folder, char* file_swap) {
				// allocate for generate absolute path
				std::filesystem::path absolute_path;

				// allocate for file cpoy
				std::ofstream cpoied_file;
				int copied_file_gotten;

				// distinguish file or folder
				int count = relative_path->size();
				if ((*relative_path)[count - 1] == L'\\' || (*relative_path)[count - 1] == L'/') {
					// process as folder
					// create new empty folder
					absolute_path = *base_folder / *relative_path;
					std::filesystem::create_directories(absolute_path);

				} else {
					// process as file
					// make sure its folder is existed
					absolute_path = *base_folder / *relative_path;
					std::filesystem::create_directories(absolute_path.parent_path());

					// open compressed file
					if (unzOpenCurrentFile(*zip_file) != UNZ_OK)
						return FALSE;

					// copy file
					cpoied_file.open(absolute_path.wstring().c_str(), std::ios_base::out | std::ios_base::binary);
					while (TRUE) {
						copied_file_gotten = unzReadCurrentFile(*zip_file, file_swap, BVH_ZIP_BUFFER);
						if (copied_file_gotten < 0)
							return FALSE;
						else if (copied_file_gotten > 0) {
							cpoied_file.write(file_swap, copied_file_gotten);
						} else break; // no any more data
					}
					cpoied_file.close();

					unzCloseCurrentFile(*zip_file);
				}

				return TRUE;
			}



		}
	}
}
