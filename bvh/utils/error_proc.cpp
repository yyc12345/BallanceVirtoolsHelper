#include "error_proc.h"
#include "string_helper.h"

namespace bvh {
	namespace utils {

		ErrorProc::ErrorProc() :
			execution_result(), status(TRUE) {
			;
		}

		ErrorProc::~ErrorProc() {
			;
		}

		void ErrorProc::SetExecutionResult(BOOL status, const char* format, ...) {
			this->status = status;
			if (format != NULL) {
				va_list argptr;
				va_start(argptr, format);
				string_helper::StdstringVPrintf(&execution_result, format, argptr);
				va_end(argptr);
			} else execution_result.clear();
			
		}

		void ErrorProc::DisplayMessage(CKContext* ctx) {
			if (status) {
				ctx->OutputToConsole("Execution OK", TRUE);
				ctx->OutputToInfo("Execution OK");
			} else {
				ctx->OutputToConsoleExBeep("Execution failed, reason:");
				ctx->OutputToInfo("Execution failed");
			}

			if (!execution_result.empty()) {
				ctx->OutputToConsole((CKSTRING)execution_result.c_str());
			}
		}

	}
}
