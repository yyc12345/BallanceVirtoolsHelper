#include "func_helper.h"

namespace func_namespace {

	char* ExecutionResult;
	char* ExecutionCache;
	char* ExecutionCache2;
	char* ConfigCache;

#define safeAlloc(target,type,size) target=(type)malloc(size);if(target==NULL)return FALSE;
#define safeFree(target) if(target!=NULL)free(target);
	BOOL InitHelper() {
		safeAlloc(ExecutionResult, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(ExecutionCache, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(ExecutionCache2, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(ConfigCache, char*, sizeof(char) * CACHE_SIZE);

		return TRUE;
	}
	void DisposeHelper() {
		safeFree(ExecutionResult);
		safeFree(ExecutionCache);
		safeFree(ExecutionCache2);
		safeFree(ConfigCache);
	}
#undef safeAlloc
#undef safeFree

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
