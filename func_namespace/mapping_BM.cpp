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
#include "../config_manager.h"
#include "../func_helper.h"

extern PluginInterface* s_Plugininterface;
extern config_manager* cfg_manager;

namespace func_namespace {
	namespace mapping {
		namespace BM {

#pragma region const value

			const uint32_t CONST_External_Texture_Length = 79;	//following list's length. not to change this when the list is changed
			const char* CONST_External_Texture_Value[] = {
				"Ball_LightningSphere1.bmp",
				"Ball_LightningSphere2.bmp",
				"Ball_LightningSphere3.bmp",
				"Ball_Paper.bmp",
				"Ball_Stone.bmp",
				"Ball_Wood.bmp",
				"Brick.bmp",
				"Button01_deselect.tga",
				"Button01_select.tga",
				"Button01_special.tga",
				"Column_beige.bmp",
				"Column_beige_fade.tga",
				"Column_blue.bmp",
				"Cursor.tga",
				"Dome.bmp",
				"DomeEnvironment.bmp",
				"DomeShadow.tga",
				"ExtraBall.bmp",
				"ExtraParticle.bmp",
				"E_Holzbeschlag.bmp",
				"FloorGlow.bmp",
				"Floor_Side.bmp",
				"Floor_Top_Border.bmp",
				"Floor_Top_Borderless.bmp",
				"Floor_Top_Checkpoint.bmp",
				"Floor_Top_Flat.bmp",
				"Floor_Top_Profil.bmp",
				"Floor_Top_ProfilFlat.bmp",
				"Font_1.tga",
				"Gravitylogo_intro.bmp",
				"HardShadow.bmp",
				"Laterne_Glas.bmp",
				"Laterne_Schatten.tga",
				"Laterne_Verlauf.tga",
				"Logo.bmp",
				"Metal_stained.bmp",
				"Misc_Ufo.bmp",
				"Misc_UFO_Flash.bmp",
				"Modul03_Floor.bmp",
				"Modul03_Wall.bmp",
				"Modul11_13_Wood.bmp",
				"Modul11_Wood.bmp",
				"Modul15.bmp",
				"Modul16.bmp",
				"Modul18.bmp",
				"Modul18_Gitter.tga",
				"Modul30_d_Seiten.bmp",
				"Particle_Flames.bmp",
				"Particle_Smoke.bmp",
				"PE_Bal_balloons.bmp",
				"PE_Bal_platform.bmp",
				"PE_Ufo_env.bmp",
				"Pfeil.tga",
				"P_Extra_Life_Oil.bmp",
				"P_Extra_Life_Particle.bmp",
				"P_Extra_Life_Shadow.bmp",
				"Rail_Environment.bmp",
				"sandsack.bmp",
				"SkyLayer.bmp",
				"Sky_Vortex.bmp",
				"Stick_Bottom.tga",
				"Stick_Stripes.bmp",
				"Target.bmp",
				"Tower_Roof.bmp",
				"Trafo_Environment.bmp",
				"Trafo_FlashField.bmp",
				"Trafo_Shadow_Big.tga",
				"Tut_Pfeil01.tga",
				"Tut_Pfeil_Hoch.tga",
				"Wolken_intro.tga",
				"Wood_Metal.bmp",
				"Wood_MetalStripes.bmp",
				"Wood_Misc.bmp",
				"Wood_Nailed.bmp",
				"Wood_Old.bmp",
				"Wood_Panel.bmp",
				"Wood_Plain.bmp",
				"Wood_Plain2.bmp",
				"Wood_Raft.bmp"
			};


#pragma endregion

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
				// ckcontext ensurance
				CKContext* ctx = s_Plugininterface->GetCKContext();
				CKScene* curScene = NULL;
				XObjectPointerArray tmpArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
				int count = tmpArray.Size();
				if (ctx->GetCurrentLevel() == NULL) {
					//no level, add it and add all object into it.
					CKLevel* levels = (CKLevel*)ctx->CreateObject(CKCID_LEVEL);
					for (unsigned int i = 0; i < count; i++)
						levels->AddObject(tmpArray.GetObjectA(i));
					ctx->SetCurrentLevel(levels);
				}
				curScene = ctx->GetCurrentLevel()->GetLevelScene();

				//used in index
				FILE_INDEX_HELPER* helper_struct = NULL;
				FILE_INDEX_TYPE index_type;
				//used in texture
				std::string texture_filename;
				CKTexture* currenrTexture = NULL;
				uint32_t is_external;
				std::filesystem::path texture_file;
				//used in material
				CKMaterial* currentMaterial = NULL;
				VxColor color;
				uint32_t is_texture, texture_index;
				//used in mesh
				CKMesh* currentMesh = NULL;
				VxVector vector;
				uint32_t vecCount;
				std::vector<VxVector> vList, vnList, vtList; // vt only use xy to store uv
				uint32_t face_data[9];
				uint32_t is_material, material_index;
				//used in object
				CK3dObject* currentObject = NULL;
				VxMatrix world_matrix;
				uint32_t is_component, mesh_index;


				// read info.bm and check version first
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

				// read texture
				ftexture.open(temp / "texture.bm", std::ios_base::out | std::ios_base::binary);
				for (auto iter = textureList.begin(); iter != textureList.end(); iter++) {
					ftexture.seekg((*iter)->offset);

					ReadString(&ftexture, &texture_filename);
					currenrTexture = (CKTexture*)ctx->CreateObject(CKCID_TEXTURE, (char*)(*iter)->name.c_str());
					(*iter)->id = currenrTexture->GetID();
					ReadInt(&ftexture, &is_external);
					if (is_external) {
						LoadExternalTexture(&texture_filename, currenrTexture);
						currenrTexture->SetSaveOptions(CKTEXTURE_EXTERNAL);
					} else {
						texture_file = tempTexture / texture_filename;
						currenrTexture->LoadImageA((char*)texture_file.string().c_str());
						currenrTexture->SetSaveOptions(CKTEXTURE_RAWDATA);
					}
				}
				ftexture.close();

				// read material
				fmaterial.open(temp / "material.bm", std::ios_base::out | std::ios_base::binary);
				for (auto iter = materialList.begin(); iter != materialList.end(); iter++) {
					fmaterial.seekg((*iter)->offset);

					color.a = 1;
					currentMaterial = (CKMaterial*)ctx->CreateObject(CKCID_MATERIAL, (char*)(*iter)->name.c_str());
					(*iter)->id = currentMaterial->GetID();
					ReadFloat(&fmaterial, &(color.r)); ReadFloat(&fmaterial, &(color.g)); ReadFloat(&fmaterial, &(color.b));
					currentMaterial->SetAmbient(color);
					ReadFloat(&fmaterial, &(color.r)); ReadFloat(&fmaterial, &(color.g)); ReadFloat(&fmaterial, &(color.b));
					currentMaterial->SetDiffuse(color);
					ReadFloat(&fmaterial, &(color.r)); ReadFloat(&fmaterial, &(color.g)); ReadFloat(&fmaterial, &(color.b));
					currentMaterial->SetEmissive(color);

					ReadInt(&fmaterial, &is_texture);
					if (is_texture) {
						ReadInt(&fmaterial, &texture_index);
						currentMaterial->SetTexture((CKTexture*)ctx->GetObjectA(textureList[texture_index]->id));
					}
				}
				fmaterial.close();

				// read mesh
				fmesh.open(temp / "mesh.bm", std::ios_base::out | std::ios_base::binary);
				for (auto iter = meshList.begin(); iter != meshList.end(); iter++) {
					fmesh.seekg((*iter)->offset);

					currentMesh = (CKMesh*)ctx->CreateObject(CKCID_MESH, (char*)(*iter)->name.c_str());
					(*iter)->id = currentMesh->GetID();
					vList.clear(); vnList.clear(); vtList.clear();
					ReadInt(&fmesh, &vecCount);
					// lazy load v
					for (int i = 0; i < vecCount; i++) {
						ReadFloat(&fmesh, &(vector.x));
						ReadFloat(&fmesh, &(vector.y));
						ReadFloat(&fmesh, &(vector.z));
						vList.push_back(vector);
					}
					// vn and vt need stored in vector
					ReadInt(&fmesh, &vecCount);
					vector.z = 0;
					for (int i = 0; i < vecCount; i++) {
						ReadFloat(&fmesh, &(vector.x));
						ReadFloat(&fmesh, &(vector.y));
						vtList.push_back(vector);
					}
					ReadInt(&fmesh, &vecCount);
					for (int i = 0; i < vecCount; i++) {
						ReadFloat(&fmesh, &(vector.x));
						ReadFloat(&fmesh, &(vector.y));
						ReadFloat(&fmesh, &(vector.z));
						vnList.push_back(vector);
					}
					// read face
					ReadInt(&fmesh, &vecCount);
					// init vector and face count
					currentMesh->SetVertexCount(vecCount * 3);
					currentMesh->SetFaceCount(vecCount);
					for (int i = 0; i < vecCount; i++) {
						for (int j = 0; j < 9; j++)
							ReadInt(&fmesh, &(face_data[j]));

						for (int j = 0; j < 9; j += 3) {
							vector = vList[face_data[j]];
							currentMesh->SetVertexPosition(i * 3 + j / 3, &vector);
							vector = vtList[face_data[j + 1]];
							currentMesh->SetVertexTextureCoordinates(i * 3 + j / 3, vector.x, vector.y);
							vector = vnList[face_data[j + 2]];
							currentMesh->SetVertexNormal(i * 3 + j / 3, &vector);
						}

						currentMesh->SetFaceVertexIndex(i, i * 3, i * 3 + 1, i * 3 + 2);
						ReadInt(&fmesh, &is_material);
						if (is_material) {
							ReadInt(&fmesh, &material_index);
							currentMesh->SetFaceMaterial(i, (CKMaterial*)ctx->GetObjectA(materialList[material_index]->id));
						}
					}
				}
				fmesh.close();

				// read object
				fobject.open(temp / "object.bm", std::ios_base::out | std::ios_base::binary);
				for (auto iter = objectList.begin(); iter != objectList.end(); iter++) {
					fobject.seekg((*iter)->offset);

					currentObject = (CK3dObject*)ctx->CreateObject(CKCID_3DOBJECT, (char*)(*iter)->name.c_str());
					(*iter)->id = currentObject->GetID();
					ReadInt(&fobject, &is_component);
					for (int i = 0; i < 4; i++)
						for (int j = 0; j < 4; j++)
							ReadFloat(&fobject, &(world_matrix[i][j]));
					currentObject->SetWorldMatrix(world_matrix);
					ReadInt(&fobject, &mesh_index);
					if (is_component) LoadComponenetMesh(currentObject, mesh_index);
					else currentObject->SetCurrentMesh((CKMesh*)ctx->GetObjectA(meshList[mesh_index]->id));

					// all object should add into current scene with dependency
					curScene->AddObjectToScene(currentObject);
				}
				fobject.close();

				// release all list data
				for (auto iter = textureList.begin(); iter != textureList.end(); iter++)
					delete (*iter);
				for (auto iter = materialList.begin(); iter != materialList.end(); iter++)
					delete (*iter);
				for (auto iter = meshList.begin(); iter != meshList.end(); iter++)
					delete (*iter);
				for (auto iter = objectList.begin(); iter != objectList.end(); iter++)
					delete (*iter);

				return TRUE;
			}

			// WARNING: all following `Read` func are based on current OS is little-endian.
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
			void LoadExternalTexture(std::string* name, CKTexture* texture) {
				std::filesystem::path external_folder;
				if (cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder.empty())
					return;	//empty folder, don't load it.
				external_folder = cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder;
				external_folder /= *name;
				
				texture->LoadImageA((char*)external_folder.string().c_str());
				texture->SetSaveOptions(CKTEXTURE_EXTERNAL);
			}
			void LoadComponenetMesh(CK3dEntity* obj, uint32_t index) {
				;
			}
#pragma endregion


#pragma region export

			BOOL ExportBM() {
				return TRUE;
			}

#pragma endregion

#pragma region misc feature

			BOOL FixBlenderTexture() {
				// filter texture
				std::vector<CK_ID> textureList;
				CKTexture* texture = NULL; CKMaterial* material = NULL;
				std::filesystem::path filepath;
				VxColor color;
				color.a = 1;

				// iterate texture
				CKContext* ctx = s_Plugininterface->GetCKContext();
				XObjectPointerArray objArray = ctx->GetObjectListByType(CKCID_TEXTURE, FALSE);
				int count = objArray.Size();
				for (int i = 0; i < count; i++) {
					texture = (CKTexture*)objArray[i];
					if (texture->GetSlotFileName(0) == NULL) continue;

					filepath = texture->GetSlotFileName(0);
					if (filepath.filename() == "Rail_Environment.bmp") {
						// this texture is need to be changed
						textureList.push_back(texture->GetID());
					}
				}

				//iterate material
				XObjectPointerArray objArray2 = ctx->GetObjectListByType(CKCID_MATERIAL, FALSE);
				count = objArray2.Size();
				for (int i = 0; i < count; i++) {
					material = (CKMaterial*)objArray2[i];
					texture = material->GetTexture(0);

					if (texture == NULL) continue;
					if (std::find(textureList.begin(), textureList.end(), texture->GetID()) == textureList.end()) {
						// no match
					} else {
						// got, change this material's data
						color.r = 0; color.g = 0; color.b = 0;
						material->SetAmbient(color);
						color.r = 100 / 255.0; color.g = 118 / 255.0; color.b = 133 / 255.0;
						material->SetDiffuse(color);
						color.r = 210 / 255.0; color.g = 210 / 255.0; color.b = 210 / 255.0;
						material->SetSpecular(color);
						color.r = 124 / 255.0; color.g = 134 / 255.0; color.b = 150 / 255.0;
						material->SetEmissive(color);
						material->SetPower(10);
					}
				}

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