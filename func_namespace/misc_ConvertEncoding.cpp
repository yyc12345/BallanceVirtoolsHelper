#include "misc_ConvertEncoding.h"
#include "../func_window/window_ConvertEncoding.h"
#include "../func_helper.h"

extern PluginInterface* s_Plugininterface;

namespace func_namespace {
	namespace misc {
		namespace ConvertEncoding {
			BOOL DoConvertEncoding() {
				func_window::window_ConvertEncoding* setting_win = new func_window::window_ConvertEncoding();
				if (setting_win->DoModal() != IDOK) {
					strcpy(func_namespace::ExecutionResult, "You cancel this process.");
					delete setting_win;
					return FALSE;
				}

				// currently only support 1 method. so don't need to process OUT_RunMethod
				// todo: for more method, add if or switch in there

				CKContext* ctx = s_Plugininterface->GetCKContext();
				int counter_all = 0, counter_success = 0, counter_fail = 0, counter_blank = 0;
				CKObject* obj = NULL;
				XObjectPointerArray objArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
				for (auto item = objArray.Begin(); item != objArray.End(); ++item) {
					obj = *item;
					counter_all++;

					if (obj->GetName() == NULL) {
						counter_blank++;
						continue;
					}

					if (MultiByteToWideChar(setting_win->OUT_OldCP, 0, obj->GetName(), -1, func_namespace::WideCharCache, CACHE_SIZE) == 0) {
						counter_fail++;
						continue;
					}
					if (WideCharToMultiByte(setting_win->OUT_NewCP, 0, func_namespace::WideCharCache, -1, func_namespace::ExecutionCache, CACHE_SIZE, NULL, NULL) == 0) {
						counter_fail++;
						continue;
					}

					obj->SetName(func_namespace::ExecutionCache);
					counter_success++;
				}

				// output to console
				ctx->OutputToConsoleEx("Convert encoding result:\nAll: %d\nBlank: %d\nSuccess: %d\nFail: %d", counter_all, counter_blank, counter_success, counter_fail);

				delete setting_win;
				return TRUE;
			}
		}
	}
}