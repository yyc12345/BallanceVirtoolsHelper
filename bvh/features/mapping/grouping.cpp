#include "grouping.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"
#include <string>

namespace bvh {
	namespace features {
		namespace mapping {
			namespace grouping {

#pragma region const value

				const int CONST_ElementsList_Length = 23;
				const char* CONST_ElementsList[] = {
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
					"P_Modul_41"
				};

#pragma endregion


				void AutoGrouping(utils::ParamPackage* pkg) {
					CKContext* ctx = pkg->plgif->GetCKContext();
					CK3dObject* obj = NULL;
					std::string objName, sectorName;

					XObjectPointerArray objArray = ctx->GetObjectListByType(CKCID_3DOBJECT, FALSE);
					int count = objArray.Size();
					int strlength = 0;
					for (int i = 0; i < count; i++) {
						obj = (CK3dObject*)objArray[i];

						// get name. no name no process
						if (obj->GetName() == NULL) continue;
						objName = obj->GetName();

						// ====================== elements
						for (int j = 0; j < CONST_ElementsList_Length; j++) {
							if (utils::string_helper::StdtringStartsWith(objName, CONST_ElementsList[j])) {
								strlength = strlen(CONST_ElementsList[j]);
								if (objName.size() < strlength + 3)
									continue;	// not allowed name. skip. check next element

								// match, group into
								// element group
								groupIntoWithCreation(ctx, obj, CONST_ElementsList[j]);
								// sector group
								utils::string_helper::StdstringPrintf(&sectorName, "Sector_0%c", objName[strlength - 1 + 3]);
								groupIntoWithCreation(ctx, obj, sectorName.c_str());

								goto outFor;	// don't process following. process next obj
							}
							// no match. next one
						}

						// ====================== road
						if (utils::string_helper::StdtringStartsWith(objName, "A_Floor")) {
							groupIntoWithCreation(ctx, obj, "Phys_Floors");
							groupIntoWithCreation(ctx, obj, "Sound_HitID_01");
							groupIntoWithCreation(ctx, obj, "Sound_RollID_01");
							groupIntoWithCreation(ctx, obj, "Shadow");
							continue;
						}
						if (utils::string_helper::StdtringStartsWith(objName, "A_Rail")) {
							groupIntoWithCreation(ctx, obj, "Phys_FloorRails");
							groupIntoWithCreation(ctx, obj, "Sound_HitID_03");
							groupIntoWithCreation(ctx, obj, "Sound_RollID_03");
							continue;
						}
						if (utils::string_helper::StdtringStartsWith(objName, "A_Wood")) {
							groupIntoWithCreation(ctx, obj, "Phys_Floors");
							groupIntoWithCreation(ctx, obj, "Sound_HitID_02");
							groupIntoWithCreation(ctx, obj, "Sound_RollID_02");
							groupIntoWithCreation(ctx, obj, "Shadow");
							continue;
						}

						// ====================== death cube
						if (utils::string_helper::StdtringStartsWith(objName, "DepthCubes")) {
							groupIntoWithCreation(ctx, obj, "DepthTestCubes");
							continue;
						}

						// ====================== stopper
						if (utils::string_helper::StdtringStartsWith(objName, "A_Stopper")) {
							groupIntoWithCreation(ctx, obj, "Phys_FloorStopper");
							continue;
						}

						// ====================== special element
						if (objName == "PS_FourFlames_01") {
							groupIntoWithCreation(ctx, obj, "PS_Levelstart");
							continue;
						}
						if (objName == "PE_Balloon_01") {
							groupIntoWithCreation(ctx, obj, "PE_Levelende");
							continue;
						}
						if (utils::string_helper::StdtringStartsWith(objName, "PC_TwoFlames")) {
							groupIntoWithCreation(ctx, obj, "PC_Checkpoints");
							continue;
						}
						if (utils::string_helper::StdtringStartsWith(objName, "PR_Resetpoint")) {
							groupIntoWithCreation(ctx, obj, "PR_Resetpoints");

							// solve the problem that the sector is not be created
							// when this sector don't contain any elements
							// sector group
							utils::string_helper::StdstringPrintf(&sectorName, "Sector_0%c", objName[13 - 1 + 3]); // 13 is "PR_Resetpoint"'s length.
							groupIntoWithCreation(ctx, obj, sectorName.c_str());
							continue;
						}

						// no matched item. don't group it.

					outFor:
						;// jump from inner for postion
					}

					pkg->error_proc->SetExecutionResult(TRUE);
				}
				void groupIntoWithCreation(CKContext* ctx, CK3dObject* obj, const char* groupName) {
					CKGroup* group = NULL;
					if ((group = (CKGroup*)ctx->GetObjectByNameAndClass((char*)groupName, CKCID_GROUP, NULL)) == NULL) {
						// no group. create it
						group = (CKGroup*)ctx->CreateObject(CKCID_GROUP, (char*)groupName);
						ctx->GetCurrentScene()->AddObjectToScene(group);
					}
					group->AddObject(obj);
				}

				void GroupingChecker(utils::ParamPackage* pkg) {
					pkg->error_proc->SetExecutionResult(TRUE);
				}

			}
		}
	}
}
