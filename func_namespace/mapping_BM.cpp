#include "../stdafx.h"
#define ZLIB_WINAPI
#include <zconf.h>

#include "mapping_BM.h"

#include <zlib.h>
#include <unzip.h>
#include <zip.h>
#include <iowin32.h>
#include <fstream>
#include <vector>
#include <cuchar>
#include <sstream>
#include <unordered_map>
#include "../config_manager.h"
#include "../func_helper.h"
#include "../func_window/mapping_BMExport.h"

#pragma warning(disable:4305)

extern PluginInterface* s_Plugininterface;
extern config_manager* cfg_manager;

namespace func_namespace {
	namespace mapping {
		namespace BM {

#pragma region const value

			const uint32_t CONST_ExternalTextureList_Length = 81;	//following list's length. not to change this when the list is changed
			const char* CONST_ExternalTextureList[] = {
				"atari.avi",
				"atari.bmp",
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

			const uint32_t CONST_ExternalTextureList_Length = 27;	//following list's length. not to change this when the list is changed
			const char* CONST_ExternalComponent[] = {
				"P_Extra_Life",
				"P_Extra_Point",
				"P_Trafo_Paper",
				"P_Trafo_Stone",
				"P_Trafo_Wood",
				"P_Ball_Paper",
				"P_Ball_Stone",
				"P_Ball_Wood",
				"P_Box",
				"P_Dome",
				"P_Modul_01",
				"P_Modul_03",
				"P_Modul_08",
				"P_Modul_17",
				"P_Modul_18",
				"P_Modul_19",
				"P_Modul_25",
				"P_Modul_26",
				"P_Modul_29",
				"P_Modul_30",
				"P_Modul_34",
				"P_Modul_37",
				"P_Modul_41",
				"PC_TwoFlames",
				"PE_Balloon",
				"PR_Resetpoint",
				"PS_FourFlames"
			};

#pragma endregion

#pragma region import

			BOOL ImportBM() {
				// ====================================== decompress
				// get file
				std::string filepath;
				std::filesystem::path file, temp, tempTexture;
				if (!func_namespace::OpenFileDialog(&filepath, "BM file(*.bm)\0*.bm\0", "bm", TRUE)) {
					strcpy(func_namespace::ExecutionResult, "No selected BM file.");
					return FALSE;
				}
				file = filepath;
				// get temp folder
				func_namespace::GetTempFolder(&temp);
				temp /= "9d2aa26133b94afaa2edcaf580c71e86";	// 9d2aa26133b94afaa2edcaf580c71e86 is guid
				tempTexture = temp / "Texture";
				//clean temp folder
				std::filesystem::remove_all(temp);
				std::filesystem::create_directory(temp);
				// decompress it
				zip_handle::Decompress(&file, &temp);


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
				findex.open(temp / "index.bm", std::ios_base::in | std::ios_base::binary);

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
				ftexture.open(temp / "texture.bm", std::ios_base::in | std::ios_base::binary);
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
					currenrTexture->FreeVideoMemory();
				}
				ftexture.close();

				// read material
				fmaterial.open(temp / "material.bm", std::ios_base::in | std::ios_base::binary);
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
				fmesh.open(temp / "mesh.bm", std::ios_base::in | std::ios_base::binary);
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
				fobject.open(temp / "object.bm", std::ios_base::in | std::ios_base::binary);
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
				// ============================================get file
				std::string filepath;
				std::filesystem::path file, temp, tempTexture;
				func_window::mapping_BMExport* bm_export_window = new func_window::mapping_BMExport();
				if (bm_export_window->DoModal() != IDOK) {
					strcpy(func_namespace::ExecutionResult, "You cancel this process.");
					delete bm_export_window;
					return FALSE;
				}
				file = bm_export_window->OUT_File;
				// get temp folder
				func_namespace::GetTempFolder(&temp);
				temp /= "a6694fa9ca1c46588cf4b6e6d376c3bd";	// a6694fa9ca1c46588cf4b6e6d376c3bd is guid
				tempTexture = temp / "Texture";
				//clean temp folder
				std::filesystem::remove_all(temp);
				std::filesystem::create_directory(temp);



				// ============================================write zip
				zip_handle::Compress(&file, &temp);
				delete bm_export_window;
				return TRUE;
			}

			// WARNING: all following `Write` func are based on current OS is little-endian.
			void WriteInt(std::ofstream* fs, uint32_t* num) {
				fs->write((char*)num, sizeof(uint32_t));
			}
			void WriteInt(std::ofstream* fs, uint64_t* num) {
				fs->write((char*)num, sizeof(uint64_t));
			}
			void WriteFloat(std::ofstream* fs, float* num) {
				fs->write((char*)num, sizeof(float));
			}
			void WriteString(std::ofstream* fs, std::string* str) {
				// for microsoft shit implementation. i need convert utf8 output into current locale output
				MultiByteToWideChar(CP_ACP, 0, str->c_str(), -1, func_namespace::WideCharCache, CACHE_SIZE);
				WideCharToMultiByte(CP_UTF8, 0, func_namespace::WideCharCache, -1, func_namespace::ExecutionCache, CACHE_SIZE, NULL, NULL);

				// start convert
				std::mbstate_t state{};
				size_t convCount = 0, convPos = 0, convAll = strlen(func_namespace::ExecutionCache);
				uint32_t length = 0;

				while ((convCount = mbrtoc32(&(func_namespace::BMNameCache[length]), &(func_namespace::ExecutionCache[convPos]), convAll - convPos, &state)) > 0) {
					convPos += convCount;
					length++;
				}

				// write length
				WriteInt(fs, &length);

				// write data
				fs->write((char*)func_namespace::BMNameCache, length * sizeof(char32_t));

			}

#pragma endregion

#pragma region misc feature

			BOOL FixBlenderTexture() {
				// filter texture
				std::unordered_map<CK_ID, uint32_t> textureList;
				CKTexture* texture = NULL; CKMaterial* material = NULL;
				std::filesystem::path filepath;
				std::string filename;
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
					filename = filepath.filename().string();
					for (uint32_t j = 0; j < CONST_ExternalTextureList_Length; j++) {
						if (filename == CONST_ExternalTextureList[j]) {
							textureList.try_emplace(texture->GetID(), j);
							goto nextTexture; //have get a valid texture. next texture
						}
						// no match. next one
					}

				nextTexture:
					;
				}

				//iterate material
				XObjectPointerArray objArray2 = ctx->GetObjectListByType(CKCID_MATERIAL, FALSE);
				count = objArray2.Size();
				for (int i = 0; i < count; i++) {
					material = (CKMaterial*)objArray2[i];
					texture = material->GetTexture(0);

					if (texture == NULL) continue;
					auto target = textureList.find(texture->GetID());
					if (target == textureList.end())
						continue;	// no found

					// judge its type
					switch (target->second) {
						//case 0:    //atari.avi
						//case 1:    //atari.bmp
						//case 2:    //Ball_LightningSphere1.bmp
						//case 3:    //Ball_LightningSphere2.bmp
						//case 4:    //Ball_LightningSphere3.bmp
						case 5:    //Ball_Paper.bmp
							color.r = 25 / 255.0; color.g = 25 / 255.0; color.b = 25 / 255.0;
							material->SetAmbient(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetDiffuse(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetSpecular(color);
							color.r = 100 / 255.0; color.g = 100 / 255.0; color.b = 100 / 255.0;
							material->SetEmissive(color);
							material->SetPower(0);
							break;
						case 6:    //Ball_Stone.bmp
						case 7:    //Ball_Wood.bmp
							color.r = 25 / 255.0; color.g = 25 / 255.0; color.b = 25 / 255.0;
							material->SetAmbient(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetDiffuse(color);
							color.r = 229 / 255.0; color.g = 229 / 255.0; color.b = 229 / 255.0;
							material->SetSpecular(color);
							color.r = 60 / 255.0; color.g = 60 / 255.0; color.b = 60 / 255.0;
							material->SetEmissive(color);
							material->SetPower(0);
							break;
						case 8:    //Brick.bmp
							color.r = 25 / 255.0; color.g = 25 / 255.0; color.b = 25 / 255.0;
							material->SetAmbient(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetDiffuse(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetSpecular(color);
							color.r = 100 / 255.0; color.g = 100 / 255.0; color.b = 100 / 255.0;
							material->SetEmissive(color);
							material->SetPower(0);
							break;
							//case 9:    //Button01_deselect.tga
							//case 10:    //Button01_select.tga
							//case 11:    //Button01_special.tga
						case 12:    //Column_beige.bmp
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 233 / 255.0; color.g = 233 / 255.0; color.b = 233 / 255.0;
							material->SetDiffuse(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetSpecular(color);
							color.r = 80 / 255.0; color.g = 80 / 255.0; color.b = 80 / 255.0;
							material->SetEmissive(color);
							material->SetPower(0);
							break;
						case 13:    //Column_beige_fade.tga
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 233 / 255.0; color.g = 233 / 255.0; color.b = 233 / 255.0;
							material->SetDiffuse(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetSpecular(color);
							color.r = 80 / 255.0; color.g = 80 / 255.0; color.b = 80 / 255.0;
							material->SetEmissive(color);
							material->SetPower(0);
							material->EnableAlphaTest(FALSE);
							material->SetAlphaFunc(VXCMP_GREATER);
							material->SetAlphaRef(1);
							material->EnableAlphaBlend();
							material->SetSourceBlend(VXBLEND_SRCALPHA);
							material->SetDestBlend(VXBLEND_INVSRCCOLOR);
							material->EnableZWrite();
							material->SetZFunc(VXCMP_LESSEQUAL);
							break;
						case 14:    //Column_blue.bmp
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 209 / 255.0; color.g = 209 / 255.0; color.b = 209 / 255.0;
							material->SetDiffuse(color);
							color.r = 150 / 255.0; color.g = 150 / 255.0; color.b = 150 / 255.0;
							material->SetSpecular(color);
							color.r = 80 / 255.0; color.g = 80 / 255.0; color.b = 80 / 255.0;
							material->SetEmissive(color);
							material->SetPower(31);
							break;
							//case 15:    //Cursor.tga
							//case 16:    //Dome.bmp
							//case 17:    //DomeEnvironment.bmp
							//case 18:    //DomeShadow.tga
							//case 19:    //ExtraBall.bmp
							//case 20:    //ExtraParticle.bmp
						case 21:    //E_Holzbeschlag.bmp
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 186 / 255.0; color.g = 186 / 255.0; color.b = 186 / 255.0;
							material->SetDiffuse(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetSpecular(color);
							color.r = 65 / 255.0; color.g = 65 / 255.0; color.b = 65 / 255.0;
							material->SetEmissive(color);
							material->SetPower(0);
							break;
							//case 22:    //FloorGlow.bmp
						case 23:    //Floor_Side.bmp
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 122 / 255.0; color.g = 122 / 255.0; color.b = 122 / 255.0;
							material->SetDiffuse(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetSpecular(color);
							color.r = 104 / 255.0; color.g = 104 / 255.0; color.b = 104 / 255.0;
							material->SetEmissive(color);
							material->SetPower(0);
							break;
						case 24:    //Floor_Top_Border.bmp
						case 25:    //Floor_Top_Borderless.bmp
						case 26:    //Floor_Top_Checkpoint.bmp
						case 27:    //Floor_Top_Flat.bmp
						case 28:    //Floor_Top_Profil.bmp
						case 29:    //Floor_Top_ProfilFlat.bmp
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetDiffuse(color);
							color.r = 80 / 255.0; color.g = 80 / 255.0; color.b = 80 / 255.0;
							material->SetSpecular(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetEmissive(color);
							material->SetPower(100);
							break;
							//case 30:    //Font_1.tga
							//case 31:    //Gravitylogo_intro.bmp
							//case 32:    //HardShadow.bmp
						case 33:    //Laterne_Glas.bmp
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetDiffuse(color);
							color.r = 229 / 255.0; color.g = 229 / 255.0; color.b = 229 / 255.0;
							material->SetSpecular(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetEmissive(color);
							material->SetPower(0);
							break;
						case 34:    //Laterne_Schatten.tga
						case 35:    //Laterne_Verlauf.tga
							color.r = 25 / 255.0; color.g = 25 / 255.0; color.b = 25 / 255.0;
							material->SetAmbient(color);
							color.r = 0; color.g = 0; color.b = 0;
							material->SetDiffuse(color);
							color.r = 229 / 255.0; color.g = 229 / 255.0; color.b = 229 / 255.0;
							material->SetSpecular(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetEmissive(color);
							material->SetPower(0);
							material->EnableAlphaTest();
							material->SetAlphaFunc(VXCMP_GREATER);
							material->SetAlphaRef(1);
							material->EnableAlphaBlend();
							material->SetSourceBlend(VXBLEND_SRCALPHA);
							material->SetDestBlend(VXBLEND_INVSRCCOLOR);
							material->EnableZWrite();
							material->SetZFunc(VXCMP_LESSEQUAL);
							material->SetTwoSided(TRUE);
							break;
							//case 36:    //Logo.bmp
							//case 37:    //Metal_stained.bmp
							//case 38:    //Misc_Ufo.bmp
							//case 39:    //Misc_UFO_Flash.bmp
							//case 40:    //Modul03_Floor.bmp
							//case 41:    //Modul03_Wall.bmp
							//case 42:    //Modul11_13_Wood.bmp
							//case 43:    //Modul11_Wood.bmp
							//case 44:    //Modul15.bmp
							//case 45:    //Modul16.bmp
							//case 46:    //Modul18.bmp
							//case 47:    //Modul18_Gitter.tga
							//case 48:    //Modul30_d_Seiten.bmp
							//case 49:    //Particle_Flames.bmp
							//case 50:    //Particle_Smoke.bmp
							//case 51:    //PE_Bal_balloons.bmp
							//case 52:    //PE_Bal_platform.bmp
							//case 53:    //PE_Ufo_env.bmp
							//case 54:    //Pfeil.tga
							//case 55:    //P_Extra_Life_Oil.bmp
							//case 56:    //P_Extra_Life_Particle.bmp
							//case 57:    //P_Extra_Life_Shadow.bmp
						case 58:    //Rail_Environment.bmp
							color.r = 0; color.g = 0; color.b = 0;
							material->SetAmbient(color);
							color.r = 100 / 255.0; color.g = 118 / 255.0; color.b = 133 / 255.0;
							material->SetDiffuse(color);
							color.r = 210 / 255.0; color.g = 210 / 255.0; color.b = 210 / 255.0;
							material->SetSpecular(color);
							color.r = 124 / 255.0; color.g = 134 / 255.0; color.b = 150 / 255.0;
							material->SetEmissive(color);
							material->SetPower(10);
							break;
							//case 59:    //sandsack.bmp
							//case 60:    //SkyLayer.bmp
							//case 61:    //Sky_Vortex.bmp
						case 62:    //Stick_Bottom.tga
							color.r = 25 / 255.0; color.g = 25 / 255.0; color.b = 25 / 255.0;
							material->SetAmbient(color);
							color.r = 100 / 255.0; color.g = 118 / 255.0; color.b = 133 / 255.0;
							material->SetDiffuse(color);
							color.r = 210 / 255.0; color.g = 210 / 255.0; color.b = 210 / 255.0;
							material->SetSpecular(color);
							color.r = 124 / 255.0; color.g = 134 / 255.0; color.b = 150 / 255.0;
							material->SetEmissive(color);
							material->SetPower(13);
							material->EnableAlphaTest(FALSE);
							material->SetAlphaFunc(VXCMP_GREATER);
							material->SetAlphaRef(1);
							material->EnableAlphaBlend();
							material->SetSourceBlend(VXBLEND_SRCALPHA);
							material->SetDestBlend(VXBLEND_INVSRCCOLOR);
							material->EnableZWrite();
							material->SetZFunc(VXCMP_LESSEQUAL);
							break;
						case 63:    //Stick_Stripes.bmp
							color.r = 25 / 255.0; color.g = 25 / 255.0; color.b = 25 / 255.0;
							material->SetAmbient(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetDiffuse(color);
							color.r = 229 / 255.0; color.g = 229 / 255.0; color.b = 229 / 255.0;
							material->SetSpecular(color);
							color.r = 106 / 255.0; color.g = 106 / 255.0; color.b = 106 / 255.0;
							material->SetEmissive(color);
							material->SetPower(13);
							break;
							//case 64:    //Target.bmp
						case 65:    //Tower_Roof.bmp
							color.r = 25 / 255.0; color.g = 25 / 255.0; color.b = 25 / 255.0;
							material->SetAmbient(color);
							color.r = 218 / 255.0; color.g = 218 / 255.0; color.b = 218 / 255.0;
							material->SetDiffuse(color);
							color.r = 64 / 255.0; color.g = 64 / 255.0; color.b = 64 / 255.0;
							material->SetSpecular(color);
							color.r = 103 / 255.0; color.g = 103 / 255.0; color.b = 103 / 255.0;
							material->SetEmissive(color);
							material->SetPower(100);
							break;
							//case 66:    //Trafo_Environment.bmp
							//case 67:    //Trafo_FlashField.bmp
							//case 68:    //Trafo_Shadow_Big.tga
							//case 69:    //Tut_Pfeil01.tga
							//case 70:    //Tut_Pfeil_Hoch.tga
							//case 71:    //Wolken_intro.tga
							//case 72:    //Wood_Metal.bmp
							//case 73:    //Wood_MetalStripes.bmp
							//case 74:    //Wood_Misc.bmp
							//case 75:    //Wood_Nailed.bmp
							//case 76:    //Wood_Old.bmp
						case 77:    //Wood_Panel.bmp
							color.r = 2 / 255.0; color.g = 2 / 255.0; color.b = 2 / 255.0;
							material->SetAmbient(color);
							color.r = 1; color.g = 1; color.b = 1;
							material->SetDiffuse(color);
							color.r = 59 / 255.0; color.g = 59 / 255.0; color.b = 59 / 255.0;
							material->SetSpecular(color);
							color.r = 30 / 255.0; color.g = 30 / 255.0; color.b = 30 / 255.0;
							material->SetEmissive(color);
							material->SetPower(25);
							break;
							//case 78:    //Wood_Plain.bmp
							//case 79:    //Wood_Plain2.bmp
							//case 80:    //Wood_Raft.bmp

					}
				}

				return TRUE;
			}

#pragma endregion


			namespace zip_handle {

				void Compress(std::filesystem::path* filepath, std::filesystem::path* folder) {
					zlib_filefunc_def ffunc;
					fill_win32_filefunc(&ffunc);
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
					memset(&filedata, 0, sizeof(WIN32_FIND_DATA)); ZeroMemory(&zi, sizeof(zip_fileinfo));
					sprintf(func_namespace::ExecutionCache, "%s\\*", nowFolder->string().c_str());
					HANDLE fhandle = FindFirstFile(func_namespace::ExecutionCache, &filedata);
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
								sprintf(func_namespace::ExecutionCache, "%s/", relativeFolder.string().c_str());
								if (zipOpenNewFileInZip3(*zip_file, func_namespace::ExecutionCache, &zi,
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

									file_get.read(func_namespace::ExecutionCache, CACHE_SIZE);
									file_get_count = file_get.gcount();
									zipWriteInFileInZip(*zip_file, func_namespace::ExecutionCache, file_get_count);
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

					std::string inner_filename;
					std::filesystem::path folder;
					folder = *_folder;

					fill_win32_filefunc(&ffunc);
					unzFile zip_file = NULL;
					zip_file = unzOpen2(filepath->string().c_str(), &ffunc);
					if (zip_file == NULL) throw std::bad_alloc();

					// read global information
					if (unzGetGlobalInfo(zip_file, &gi) != UNZ_OK)
						throw std::bad_alloc();

					// iterate file
					for (uLong i = 0; i < gi.number_entry; i++) {
						if (unzGetCurrentFileInfo(zip_file, &file_info, func_namespace::ExecutionCache, CACHE_SIZE, NULL, 0, NULL, 0) != UNZ_OK)
							throw std::bad_alloc();
						inner_filename = func_namespace::ExecutionCache;

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
							read_result = unzReadCurrentFile(*zip_file, func_namespace::ExecutionCache, CACHE_SIZE);
							if (read_result < 0)
								return FALSE;
							else if (read_result > 0) {
								fs.write(func_namespace::ExecutionCache, read_result);
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
}