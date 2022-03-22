#include "convert_encoding.h"
#include "../../mfcwindows/convert_encoding.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"

namespace bvh {
	namespace features {
		namespace misc {
			namespace convert_encoding {
				void DoConvertEncoding(utils::ParamPackage* pkg) {
					mfcwindows::ConvertEncoding* setting_win = new mfcwindows::ConvertEncoding(pkg);
					if (setting_win->DoModal() != IDOK) {
						pkg->error_proc->SetExecutionResult(FALSE, "You cancel this process.");
						delete setting_win;
						return;
					}

					// currently only support 1 method. so don't need to process OUT_RunMethod
					// todo: for more method, add if or switch in there

					CKContext* ctx = pkg->plgif->GetCKContext();
					int counter_all = 0, counter_success = 0, counter_fail = 0, counter_blank = 0;
					std::wstring wscache;
					std::string mbcache;
					CKObject* obj = NULL;
					XObjectPointerArray objArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
					for (auto item = objArray.Begin(); item != objArray.End(); ++item) {
						obj = *item;
						counter_all++;

						if (obj->GetName() == NULL) {
							counter_blank++;
							continue;
						}

						mbcache = obj->GetName();
						if (!utils::string_helper::String2Wstring(&mbcache, &wscache, setting_win->OUT_OldCP)) {
							counter_fail++;
							continue;
						}
						if (!utils::string_helper::Wstring2String(&wscache, &mbcache, setting_win->OUT_NewCP)) {
							counter_fail++;
							continue;
						}

						obj->SetName((CKSTRING)mbcache.c_str());
						counter_success++;
					}

					// output to console
					pkg->error_proc->SetExecutionResult(TRUE, "Convert encoding result:\nAll: %d\nBlank: %d\nSuccess: %d\nFail: %d", counter_all, counter_blank, counter_success, counter_fail);
					delete setting_win;
					return;
				}
			}
		}
	}
}