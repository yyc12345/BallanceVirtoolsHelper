#include "bmfile.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"
#include <unordered_map>
#include <filesystem>

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {

				void FixTexture(utils::ParamPackage* pkg) {
					// filter texture
					std::unordered_map<CK_ID, uint32_t> textureList;
					CKTexture* texture = NULL; CKMaterial* material = NULL;
					std::filesystem::path filepath;
					std::string filename;
					std::string materialname;
					VxColor color;
					color.a = 1;

					// iterate texture
					CKContext* ctx = pkg->plgif->GetCKContext();
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

						// detect omitted material
						if (material->GetName() != NULL) {
							materialname = material->GetName();
							if ((!pkg->cfg_manager->CurrentConfig.func_mapping_bm_OmittedMaterialPrefix.empty()) &&
								utils::string_helper::StdstringStartsWith(materialname, pkg->cfg_manager->CurrentConfig.func_mapping_bm_OmittedMaterialPrefix)) {
								continue;
							}
						}

						// get texture index
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
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetSpecular(color);
								color.r = 100 / 255.0f; color.g = 100 / 255.0f; color.b = 100 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);
								break;
							case 6:    //Ball_Stone.bmp
							case 7:    //Ball_Wood.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 60 / 255.0f; color.g = 60 / 255.0f; color.b = 60 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);
								break;
							case 8:    //Brick.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetSpecular(color);
								color.r = 100 / 255.0f; color.g = 100 / 255.0f; color.b = 100 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);
								break;
								//case 9:    //Button01_deselect.tga
								//case 10:    //Button01_select.tga
								//case 11:    //Button01_special.tga
							case 12:    //Column_beige.bmp
								color.r = 0; color.g = 0; color.b = 0;
								material->SetAmbient(color);
								color.r = 233 / 255.0f; color.g = 233 / 255.0f; color.b = 233 / 255.0f;
								material->SetDiffuse(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetSpecular(color);
								color.r = 80 / 255.0f; color.g = 80 / 255.0f; color.b = 80 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);
								break;
							case 13:    //Column_beige_fade.tga
								color.r = 0; color.g = 0; color.b = 0;
								material->SetAmbient(color);
								color.r = 233 / 255.0f; color.g = 233 / 255.0f; color.b = 233 / 255.0f;
								material->SetDiffuse(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetSpecular(color);
								color.r = 80 / 255.0f; color.g = 80 / 255.0f; color.b = 80 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);

								material->EnableAlphaTest(FALSE);
								material->SetAlphaFunc(VXCMP_GREATER);
								material->SetAlphaRef(1);
								material->EnableAlphaBlend(TRUE);
								material->SetSourceBlend(VXBLEND_SRCALPHA);
								material->SetDestBlend(VXBLEND_INVSRCALPHA);
								material->EnableZWrite(TRUE);
								material->SetZFunc(VXCMP_LESSEQUAL);
								break;
							case 14:    //Column_blue.bmp
								color.r = 0; color.g = 0; color.b = 0;
								material->SetAmbient(color);
								color.r = 209 / 255.0f; color.g = 209 / 255.0f; color.b = 209 / 255.0f;
								material->SetDiffuse(color);
								color.r = 150 / 255.0f; color.g = 150 / 255.0f; color.b = 150 / 255.0f;
								material->SetSpecular(color);
								color.r = 80 / 255.0f; color.g = 80 / 255.0f; color.b = 80 / 255.0f;
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
								color.r = 186 / 255.0f; color.g = 186 / 255.0f; color.b = 186 / 255.0f;
								material->SetDiffuse(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetSpecular(color);
								color.r = 65 / 255.0f; color.g = 65 / 255.0f; color.b = 65 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);
								break;
								//case 22:    //FloorGlow.bmp
							case 23:    //Floor_Side.bmp
								color.r = 0; color.g = 0; color.b = 0;
								material->SetAmbient(color);
								color.r = 122 / 255.0f; color.g = 122 / 255.0f; color.b = 122 / 255.0f;
								material->SetDiffuse(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetSpecular(color);
								color.r = 104 / 255.0f; color.g = 104 / 255.0f; color.b = 104 / 255.0f;
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
								color.r = 80 / 255.0f; color.g = 80 / 255.0f; color.b = 80 / 255.0f;
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
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetEmissive(color);
								material->SetPower(0);
								break;
							case 34:    //Laterne_Schatten.tga
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetDiffuse(color);
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetEmissive(color);
								material->SetPower(0);

								material->EnableAlphaTest(TRUE);
								material->SetAlphaFunc(VXCMP_GREATER);
								material->SetAlphaRef(1);
								material->EnableAlphaBlend(TRUE);
								material->SetSourceBlend(VXBLEND_SRCALPHA);
								material->SetDestBlend(VXBLEND_INVSRCALPHA);
								material->EnableZWrite(TRUE);
								material->SetZFunc(VXCMP_LESSEQUAL);
								break;
							case 35:    //Laterne_Verlauf.tga
								color.r = 0; color.g = 0; color.b = 0;
								material->SetAmbient(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetDiffuse(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetSpecular(color);
								color.r = 59 / 255.0f; color.g = 59 / 255.0f; color.b = 59 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);

								material->EnableAlphaTest(TRUE);
								material->SetAlphaFunc(VXCMP_GREATER);
								material->SetAlphaRef(1);
								material->EnableAlphaBlend(TRUE);
								material->SetSourceBlend(VXBLEND_SRCALPHA);
								material->SetDestBlend(VXBLEND_INVSRCALPHA);
								material->EnableZWrite(TRUE);
								material->SetZFunc(VXCMP_LESSEQUAL);
								material->SetTwoSided(TRUE);
								break;
								//case 36:    //Logo.bmp
							case 37:    //Metal_stained.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 65 / 255.0f; color.g = 65 / 255.0f; color.b = 65 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(25);
								break;
								//case 38:    //Misc_Ufo.bmp
								//case 39:    //Misc_UFO_Flash.bmp
								//case 40:    //Modul03_Floor.bmp
								//case 41:    //Modul03_Wall.bmp
							case 42:    //Modul11_13_Wood.bmp
								color.r = 9 / 255.0f; color.g = 9 / 255.0f; color.b = 9 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 100 / 255.0f; color.g = 100 / 255.0f; color.b = 100 / 255.0f;
								material->SetSpecular(color);
								color.r = 70 / 255.0f; color.g = 70 / 255.0f; color.b = 70 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(50);
								break;
							case 43:    //Modul11_Wood.bmp
								color.r = 9 / 255.0f; color.g = 9 / 255.0f; color.b = 9 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 100 / 255.0f; color.g = 100 / 255.0f; color.b = 100 / 255.0f;
								material->SetSpecular(color);
								color.r = 50 / 255.0f; color.g = 50 / 255.0f; color.b = 50 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(50);
								break;
							case 44:    //Modul15.bmp
								color.r = 16 / 255.0f; color.g = 16 / 255.0f; color.b = 16 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 100 / 255.0f; color.g = 100 / 255.0f; color.b = 100 / 255.0f;
								material->SetSpecular(color);
								color.r = 70 / 255.0f; color.g = 70 / 255.0f; color.b = 70 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(100);
								break;
							case 45:    //Modul16.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 100 / 255.0f; color.g = 100 / 255.0f; color.b = 100 / 255.0f;
								material->SetSpecular(color);
								color.r = 85 / 255.0f; color.g = 85 / 255.0f; color.b = 85 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(100);
								break;
							case 46:    //Modul18.bmp
								color.r = 0; color.g = 0; color.b = 0;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetEmissive(color);
								material->SetPower(25);
								break;
							case 47:    //Modul18_Gitter.tga
								color.r = 0; color.g = 0; color.b = 0;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 0; color.g = 0; color.b = 0;
								material->SetEmissive(color);
								material->SetPower(25);

								material->EnableAlphaTest(TRUE);
								material->SetAlphaFunc(VXCMP_GREATER);
								material->SetAlphaRef(1);
								material->EnableAlphaBlend(TRUE);
								material->SetSourceBlend(VXBLEND_SRCALPHA);
								material->SetDestBlend(VXBLEND_INVSRCALPHA);
								material->EnableZWrite(TRUE);
								material->SetZFunc(VXCMP_LESSEQUAL);
								break;
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
								color.r = 100 / 255.0f; color.g = 118 / 255.0f; color.b = 133 / 255.0f;
								material->SetDiffuse(color);
								color.r = 210 / 255.0f; color.g = 210 / 255.0f; color.b = 210 / 255.0f;
								material->SetSpecular(color);
								color.r = 124 / 255.0f; color.g = 134 / 255.0f; color.b = 150 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(10);
								break;
								//case 59:    //sandsack.bmp
								//case 60:    //SkyLayer.bmp
								//case 61:    //Sky_Vortex.bmp
							case 62:    //Stick_Bottom.tga
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 100 / 255.0f; color.g = 118 / 255.0f; color.b = 133 / 255.0f;
								material->SetDiffuse(color);
								color.r = 210 / 255.0f; color.g = 210 / 255.0f; color.b = 210 / 255.0f;
								material->SetSpecular(color);
								color.r = 124 / 255.0f; color.g = 134 / 255.0f; color.b = 150 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(13);

								material->EnableAlphaTest(FALSE);
								material->SetAlphaFunc(VXCMP_GREATER);
								material->SetAlphaRef(1);
								material->EnableAlphaBlend(TRUE);
								material->SetSourceBlend(VXBLEND_SRCALPHA);
								material->SetDestBlend(VXBLEND_INVSRCALPHA);
								material->EnableZWrite(TRUE);
								material->SetZFunc(VXCMP_LESSEQUAL);
								break;
							case 63:    //Stick_Stripes.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 106 / 255.0f; color.g = 106 / 255.0f; color.b = 106 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(13);
								break;
								//case 64:    //Target.bmp
							case 65:    //Tower_Roof.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 218 / 255.0f; color.g = 218 / 255.0f; color.b = 218 / 255.0f;
								material->SetDiffuse(color);
								color.r = 64 / 255.0f; color.g = 64 / 255.0f; color.b = 64 / 255.0f;
								material->SetSpecular(color);
								color.r = 103 / 255.0f; color.g = 103 / 255.0f; color.b = 103 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(100);
								break;
								//case 66:    //Trafo_Environment.bmp
								//case 67:    //Trafo_FlashField.bmp
								//case 68:    //Trafo_Shadow_Big.tga
								//case 69:    //Tut_Pfeil01.tga
								//case 70:    //Tut_Pfeil_Hoch.tga
								//case 71:    //Wolken_intro.tga
							case 72:    //Wood_Metal.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 229 / 255.0f; color.g = 229 / 255.0f; color.b = 229 / 255.0f;
								material->SetSpecular(color);
								color.r = 40 / 255.0f; color.g = 40 / 255.0f; color.b = 40 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(0);
								break;
								//case 73:    //Wood_MetalStripes.bmp
								//case 74:    //Wood_Misc.bmp
								//case 75:    //Wood_Nailed.bmp
								//case 76:    //Wood_Old.bmp
							case 77:    //Wood_Panel.bmp
								color.r = 2 / 255.0f; color.g = 2 / 255.0f; color.b = 2 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 59 / 255.0f; color.g = 59 / 255.0f; color.b = 59 / 255.0f;
								material->SetSpecular(color);
								color.r = 30 / 255.0f; color.g = 30 / 255.0f; color.b = 30 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(25);
								break;
								//case 78:    //Wood_Plain.bmp
							case 79:    //Wood_Plain2.bmp
								color.r = 25 / 255.0f; color.g = 25 / 255.0f; color.b = 25 / 255.0f;
								material->SetAmbient(color);
								color.r = 1; color.g = 1; color.b = 1;
								material->SetDiffuse(color);
								color.r = 100 / 255.0f; color.g = 100 / 255.0f; color.b = 100 / 255.0f;
								material->SetSpecular(color);
								color.r = 50 / 255.0f; color.g = 50 / 255.0f; color.b = 50 / 255.0f;
								material->SetEmissive(color);
								material->SetPower(50);
								break;
								//case 80:    //Wood_Raft.bmp

						}
					}

					pkg->error_proc->SetExecutionResult(TRUE);
				}


			}
		}
	}
}
