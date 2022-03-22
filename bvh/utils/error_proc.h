#pragma once

#include "../../stdafx.h"
#include <string>

namespace bvh {
	namespace utils {
		class ErrorProc {
		private:
			std::string execution_result;
			BOOL status;

		public:
			ErrorProc();
			~ErrorProc();
			void SetExecutionResult(BOOL status, const char* format = NULL, ...);
			void DisplayMessage(CKContext* ctx);
		};
	}
}
