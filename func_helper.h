#if !defined(_YYCDLL_FUNC_HELPER_H__IMPORTED_)
#define _YYCDLL_FUNC_HELPER_H__IMPORTED_

#include "stdafx.h"
#include <string>
#include <filesystem>
#define CACHE_SIZE 65526

namespace func_namespace {
	extern char* ExecutionResult;
	extern char* ExecutionCache;
	extern char* ExecutionCache2;
	extern char* ConfigCache;
	extern char* SelfCache;
	extern char32_t* BMNameCache;
	extern wchar_t* WideCharCache;
	

	BOOL InitHelper();
	void DisposeHelper();
	void DisplayLastMessage(BOOL status, CKContext* ctx);

	BOOL OpenFileDialog(std::string* returned_file, const char* file_filter, const char* file_extension, BOOL isOpen);
	BOOL OpenFolderDialog(std::string* returned_folder, HWND owner);
	void GetTempFolder(std::filesystem::path* temp_folder);
	void GetVirtoolsFolder(std::filesystem::path* vt_folder);
	
}

#endif