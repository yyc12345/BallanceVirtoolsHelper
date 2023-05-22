#pragma once

#include "../../stdafx.h"
#include <string>
#include <filesystem>

namespace bvh::utils::win32_helper {

	BOOL OpenFileDialog(std::wstring* returned_file, const wchar_t* file_filter, const wchar_t* file_extension, BOOL isOpen);
	BOOL OpenFolderDialog(std::wstring* returned_folder, HWND owner);
	void GetTempFolder(CKContext* ctx, std::filesystem::path* temp_folder);
	void GetVirtoolsFolder(std::filesystem::path* vt_folder);
	BOOL CheckCPValidation(UINT cp);
	void StdWstring2CwndText(CWnd* ctrl, std::wstring* strl);
	void CwndText2Stdstring(CWnd* ctrl, std::string* strl);
	BOOL CheckANSIPathValidation(std::string* ansi_path);

	class UnhandledExceptionGuard {
	public:
		UnhandledExceptionGuard();
		~UnhandledExceptionGuard();

		void BacktraceRegister(void);
		void BacktraceUnregister(void);
	private:
		LPTOP_LEVEL_EXCEPTION_FILTER mProcBackup;
		bool mHasSet;
	};

}
