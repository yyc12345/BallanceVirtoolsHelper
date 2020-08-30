#include "mapping_Group.h"
#include "../func_helper.h"
#include <string>

extern PluginInterface* s_Plugininterface;

namespace func_namespace {
	namespace mapping {
		namespace Group {

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


			BOOL AutoGrouping() {
				CKContext* ctx = s_Plugininterface->GetCKContext();
				CK3dObject* obj = NULL;
				std::string objName;

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
						if (objName.starts_with(CONST_ElementsList[j])) {
							strlength = strlen(CONST_ElementsList[j]);
							if (objName.size() < strlength + 3)
								continue;	// not allowed name. skip. check next element

							// match, group into
							// element group
							GroupIntoWithCreation(ctx, obj, CONST_ElementsList[j]);
							// sector group
							sprintf(func_namespace::ExecutionCache, "Sector_0%c", objName[strlength - 1 + 3]);
							GroupIntoWithCreation(ctx, obj, func_namespace::ExecutionCache);

							goto outFor;	// don't process following. process next obj
						}
						// no match. next one
					}

					// ====================== road
					if (objName.starts_with("A_Floor")) {
						GroupIntoWithCreation(ctx, obj, "Phys_Floors");
						GroupIntoWithCreation(ctx, obj, "Sound_HitID_01");
						GroupIntoWithCreation(ctx, obj, "Sound_RollID_01");
						GroupIntoWithCreation(ctx, obj, "Shadow");
						continue;
					}
					if (objName.starts_with("A_Rail")) {
						GroupIntoWithCreation(ctx, obj, "Phys_FloorRails");
						GroupIntoWithCreation(ctx, obj, "Sound_HitID_03");
						GroupIntoWithCreation(ctx, obj, "Sound_RollID_03");
						continue;
					}
					if (objName.starts_with("A_Wood")) {
						GroupIntoWithCreation(ctx, obj, "Phys_Floors");
						GroupIntoWithCreation(ctx, obj, "Sound_HitID_02");
						GroupIntoWithCreation(ctx, obj, "Sound_RollID_02");
						GroupIntoWithCreation(ctx, obj, "Shadow");
						continue;
					}

					// ====================== death cube
					if (objName.starts_with("DepthCubes")) {
						GroupIntoWithCreation(ctx, obj, "DepthTestCubes");
						continue;
					}

					// ====================== stopper
					if (objName.starts_with("A_Stopper")) {
						GroupIntoWithCreation(ctx, obj, "Phys_FloorStopper");
						continue;
					}

					// ====================== special element
					if (objName == "PS_FourFlames_01") {
						GroupIntoWithCreation(ctx, obj, "PS_Levelstart");
						continue;
					}
					if (objName == "PE_Balloon_01") {
						GroupIntoWithCreation(ctx, obj, "PE_Levelende");
						continue;
					}
					if (objName.starts_with("PC_TwoFlames")) {
						GroupIntoWithCreation(ctx, obj, "PC_Checkpoints");
						continue;
					}
					if (objName.starts_with("PR_Resetpoint")) {
						GroupIntoWithCreation(ctx, obj, "PR_Resetpoints");
						continue;
					}

					// no matched item. don't group it.

				outFor:
					;// jump from inner for postion
				}

				return TRUE;
			}
			void GroupIntoWithCreation(CKContext* ctx, CK3dObject* obj, const char* groupName) {
				CKGroup* group = NULL;
				if ((group = (CKGroup*)ctx->GetObjectByNameAndClass((char*)groupName, CKCID_GROUP, NULL)) == NULL) {
					// no group. create it
					group = (CKGroup*)ctx->CreateObject(CKCID_GROUP, (char*)groupName);
					ctx->GetCurrentScene()->AddObjectToScene(group);
				}
				group->AddObject(obj);
			}

			BOOL GroupingChecker() {
				return TRUE;
			}

		}
	}
}
