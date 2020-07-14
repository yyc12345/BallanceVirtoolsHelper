#include "func_helper.h"

namespace func_namespace {

	char* ExecutionResult;
	char* ExecutionCache;
	char* ExecutionCache2;
	char* ConfigCache;
	char* SelfCache;
	char32_t* BMNameCache;
	wchar_t* WideCharCache;

#define safeAlloc(target,type,size) target=(type)malloc(size);if(target==NULL)return FALSE;
#define safeFree(target) if(target!=NULL)free(target);
	BOOL InitHelper() {
		safeAlloc(ExecutionResult, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(ExecutionCache, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(ExecutionCache2, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(ConfigCache, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(SelfCache, char*, sizeof(char) * CACHE_SIZE);
		safeAlloc(BMNameCache, char32_t*, sizeof(char32_t) * CACHE_SIZE);
		safeAlloc(WideCharCache, wchar_t*, sizeof(wchar_t) * CACHE_SIZE);

		return TRUE;
	}
	void DisposeHelper() {
		safeFree(ExecutionResult);
		safeFree(ExecutionCache);
		safeFree(ExecutionCache2);
		safeFree(ConfigCache);
		safeFree(SelfCache);
		safeFree(BMNameCache);
		safeFree(WideCharCache);
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

	void OpenFileDialog(std::string* returned_file, const char* file_filter, const char* file_extension, BOOL isOpen) {
		returned_file->clear();

		OPENFILENAME OpenFileStruct;
		ZeroMemory(&OpenFileStruct, sizeof(OPENFILENAME));
		OpenFileStruct.lStructSize = sizeof(OPENFILENAME);
		OpenFileStruct.lpstrFile = func_namespace::SelfCache;
		OpenFileStruct.lpstrFile[0] = '\0';
		OpenFileStruct.nMaxFile = CACHE_SIZE;
		OpenFileStruct.lpstrFilter = file_filter;
		OpenFileStruct.lpstrDefExt = file_extension;
		OpenFileStruct.lpstrFileTitle = NULL;
		OpenFileStruct.nMaxFileTitle = 0;
		OpenFileStruct.lpstrInitialDir = NULL;
		OpenFileStruct.Flags = OFN_EXPLORER;
		if (isOpen) {
			if (GetOpenFileName(&OpenFileStruct))
				*returned_file = SelfCache;
		} else {
			if (GetSaveFileName(&OpenFileStruct))
				*returned_file = SelfCache;
		}
	}

	void OpenFolderDialog(std::string* returned_folder, HWND owner) {
		returned_folder->clear();

		BROWSEINFOA folderViewer = { 0 };
		folderViewer.hwndOwner = owner;
		folderViewer.pidlRoot = NULL;
		folderViewer.pszDisplayName = func_namespace::SelfCache;
		folderViewer.lpszTitle = "Pick a folder";
		folderViewer.lpfn = NULL;
		folderViewer.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		PIDLIST_ABSOLUTE data = SHBrowseForFolder(&folderViewer);
		if (data == NULL) return;
		if (SHGetPathFromIDList(data, func_namespace::SelfCache))
			*returned_folder = func_namespace::SelfCache;
		CoTaskMemFree(data);
	}

	void GetTempFolder(std::filesystem::path* temp_folder) {
		GetTempPath(CACHE_SIZE, SelfCache);
		*temp_folder = SelfCache;
	}

}
