#include "../stdafx.h"
#define ZLIB_WINAPI
#include <zconf.h>

#include "mapping_BM.h"

#include <zlib.h>
#include <unzip.h>
#include <iowin32.h>
#include <fstream>
#include <vector>
#include <cuchar>
#include <sstream>
#include "../func_helper.h"

extern PluginInterface* s_Plugininterface;

namespace func_namespace {
	namespace mapping {
		namespace BM {

#pragma region import

			BOOL ImportBM() {
				// ====================================== decompress
				// get file
				std::string filepath;
				std::filesystem::path file, temp, tempTexture;
				func_namespace::OpenFileDialog(&filepath, "BM file(*.bm)\0*.bm\0", "bm", TRUE);
				file = filepath;
				// get temp folder
				func_namespace::GetTempFolder(&temp);
				temp /= "9d2aa26133b94afaa2edcaf580c71e86";	// 9d2aa26133b94afaa2edcaf580c71e86 is guid
				//clean temp folder
				std::filesystem::remove_all(temp);
				std::filesystem::create_directory(temp);
				// decompress it
				zip_handle::Decompress(&file, &temp);

				tempTexture = temp / "Texture";

				// ====================================== read
				std::ifstream findex, fobject, fmesh, fmaterial, ftexture;
				std::vector<FILE_INDEX_HELPER*> objectList, meshList, materialList, textureList;

				// read info.bm and check version first
				FILE_INDEX_HELPER* helper_struct = NULL;
				FILE_INDEX_TYPE index_type;
				findex.open(temp / "index.bm", std::ios_base::out | std::ios_base::binary);

				uint32_t vercmp;
				ReadInt(&findex, &vercmp);
				if (vercmp != BM_FILE_VERSION) {
					findex.close();
					sprintf(func_namespace::ExecutionResult, "Not supported BM version. Need: %d, Got: %d", BM_FILE_VERSION, vercmp);
					return FALSE;
				}

				while (TRUE) {
					if (findex.peek(), findex.eof()) break;
					
					helper_struct = new FILE_INDEX_HELPER();
					ReadString(&findex, &(helper_struct->name));
					ReadInt(&findex, (uint32_t*)&index_type);
					switch (index_type) {
						case FILE_INDEX_TYPE__OBJECT:
							objectList.push_back(helper_struct);
							break;
						case FILE_INDEX_TYPE__MESH:
							meshList.push_back(helper_struct);
							break;
						case FILE_INDEX_TYPE__MATERIAL:
							materialList.push_back(helper_struct);
							break;
						case FILE_INDEX_TYPE__TEXTURE:
							textureList.push_back(helper_struct);
							break;
						default:
							;
					}
					ReadInt(&findex, &(helper_struct->offset));
				}

				findex.close();



				return TRUE;
			}

			// WARNING: all following read func are based on current OS is little-endian.
			void ReadInt(std::ifstream* fs, uint32_t* num) {
				fs->read((char*)num, sizeof(uint32_t));
			}
			void ReadInt(std::ifstream* fs, uint64_t* num) {
				fs->read((char*)num, sizeof(uint64_t));
			}
			void ReadFloat(std::ifstream* fs, float* num) {
				fs->read((char*)num, sizeof(float));
			}
			void ReadString(std::ifstream* fs, std::string* str) {
				// alloc a buffer
				str->clear();
				std::stringbuf strbuffer;

				// find length
				uint32_t length;
				ReadInt(fs, &length);

				// read and decode it
				fs->read((char*)func_namespace::BMNameCache, length * sizeof(char32_t));
				std::mbstate_t state{};
				size_t convCount;
				for (uint32_t i = 0; i < length; i++) {
					convCount = std::c32rtomb(func_namespace::ExecutionCache, func_namespace::BMNameCache[i], &state);
					if (convCount == -1) continue;
					else strbuffer.sputn(func_namespace::ExecutionCache, convCount);
				}
				strcpy(func_namespace::ExecutionCache, strbuffer.str().c_str());

				// for microsoft shit implementation. i need convert utf8 output into current locale output
				MultiByteToWideChar(CP_UTF8, 0, func_namespace::ExecutionCache, -1, func_namespace::WideCharCache, CACHE_SIZE);
				WideCharToMultiByte(CP_ACP, 0, func_namespace::WideCharCache, -1, func_namespace::ExecutionCache, CACHE_SIZE, NULL, NULL);

				//copy result
				*str = func_namespace::ExecutionCache;
			}

#pragma endregion


#pragma region export

			BOOL ExportBM() {
				return TRUE;
			}

#pragma endregion

			namespace zip_handle {

				void Compress(std::filesystem::path* filepath, std::filesystem::path* folder) {

				}




				void Decompress(std::filesystem::path* filepath, std::filesystem::path* _folder) {
					zlib_filefunc_def ffunc;
					unz_global_info gi;
					unz_file_info file_info;

					std::string inner_filename;
					std::filesystem::path folder;
					folder = *_folder;

					fill_win32_filefunc(&ffunc);
					unzFile zipfile = NULL;
					zipfile = unzOpen2(filepath->string().c_str(), &ffunc);
					if (zipfile == NULL) throw std::bad_alloc();

					// read global information
					if (unzGetGlobalInfo(zipfile, &gi) != UNZ_OK)
						throw std::bad_alloc();

					// iterate file
					for (uLong i = 0; i < gi.number_entry; i++) {
						if (unzGetCurrentFileInfo(zipfile, &file_info, func_namespace::ExecutionCache, CACHE_SIZE, NULL, 0, NULL, 0) != UNZ_OK)
							throw std::bad_alloc();
						inner_filename = func_namespace::ExecutionCache;

						if (!do_extract_currentfile(&zipfile, &inner_filename, &folder))
							throw std::bad_alloc();

						// to next file and check it.
						if ((i + 1) < gi.number_entry) {
							if (unzGoToNextFile(zipfile) != UNZ_OK)
								throw std::bad_alloc();
						}
					}

					unzClose(zipfile);
				}

				BOOL do_extract_currentfile(unzFile* zipfile, std::string* inner_name, std::filesystem::path* folder) {
					std::filesystem::path real_path;
					std::ofstream fs;
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
						if (unzOpenCurrentFile(*zipfile) != UNZ_OK)
							return FALSE;

						fs.open(real_path, std::ios_base::out | std::ios_base::binary);
						while (TRUE) {
							read_result = unzReadCurrentFile(*zipfile, func_namespace::ExecutionCache, CACHE_SIZE);
							if (read_result < 0)
								return FALSE;
							else if (read_result > 0) {
								fs.write(func_namespace::ExecutionCache, read_result);
							} else break; // no any more data
						}
						unzCloseCurrentFile(*zipfile);

						fs.close();
					}

					return TRUE;
				}



			}
		}
	}
}