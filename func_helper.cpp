#include "func_helper.h"

namespace func_namespace {

	char* ExecutionResult;
	char* ExecutionCache;

	BOOL InitHelper() {
		ExecutionResult = (char*)malloc(sizeof(char) * 65526);
		if (ExecutionResult == NULL) return FALSE;

		ExecutionCache = (char*)malloc(sizeof(char) * CACHE_SIZE);
		if (ExecutionCache == NULL) return FALSE;

		return TRUE;
	}
	void DisposeHelper() {
		if (ExecutionResult != NULL)
			free(ExecutionResult);
		if (ExecutionCache != NULL)
			free(ExecutionCache);
	}
	void DisplayLastMessage(BOOL status, CKContext* ctx) {
		if (status) {
			ctx->OutputToConsole("Execution OK", FALSE);
			ctx->OutputToInfo("Execution OK");
		} else {
			ctx->OutputToConsoleExBeep("Execution failed, reason: %s", ExecutionResult);
			ctx->OutputToInfo("Execution failed, reason: %s", ExecutionResult);
		}
	}
}
