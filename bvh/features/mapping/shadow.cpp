#include "shadow.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"

namespace bvh {
	namespace features {
		namespace mapping {
			namespace shadow {

				void AddShadow(utils::ParamPackage* pkg) {
					CKContext* ctx = pkg->plgif->GetCKContext();
					CKGroup* group = (CKGroup*)ctx->GetObjectByNameAndClass("Shadow", CKCID_GROUP);
					if (group == NULL) {
						pkg->error_proc->SetExecutionResult(FALSE, "No group named Shadow.");
						return;
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

					pkg->error_proc->SetExecutionResult(TRUE);
				}

			}
		}
	}
}