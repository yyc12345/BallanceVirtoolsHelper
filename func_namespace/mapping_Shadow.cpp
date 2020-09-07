#include "mapping_Shadow.h"
#include "../func_helper.h"

extern PluginInterface* s_Plugininterface;

namespace func_namespace {
	namespace mapping {
		namespace Shadow {

			BOOL AddShadow() {
				CKContext* ctx = s_Plugininterface->GetCKContext();
				CKGroup* group = (CKGroup*)ctx->GetObjectByNameAndClass("Shadow", CKCID_GROUP);
				if (group == NULL) {
					strcpy(func_namespace::ExecutionResult, "No group named Shadow.");
					return FALSE;
				}

				CKBeObject* cache = NULL;
				CK3dEntity* obj = NULL;
				int count = group->GetObjectCount();
				CK_CLASSID objClass;
				for (int i = 0; i < count; i++) {
					cache = group->GetObjectA(i);
					objClass = cache->GetClassID();
					if (objClass == CKCID_3DENTITY || objClass == CKCID_3DOBJECT) {
						obj = (CK3dEntity*)cache;
						obj->ModifyMoveableFlags(8, 0);
					}
				}

				return TRUE;
			}

		}
	}
}