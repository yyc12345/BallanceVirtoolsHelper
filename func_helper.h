#if !defined(_YYCDLL_FUNC_HELPER_H__IMPORTED_)
#define _YYCDLL_FUNC_HELPER_H__IMPORTED_

#include "stdafx.h"
#define CACHE_SIZE 65526

namespace func_namespace {
	extern char* ExecutionResult;
	extern char* ExecutionCache;
	extern char* ExecutionCache2;

	BOOL InitHelper();
	void DisposeHelper();
	void DisplayLastMessage(BOOL status, CKContext* ctx);
}

#endif