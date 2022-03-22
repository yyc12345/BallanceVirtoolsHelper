#pragma once

#include "../../stdafx.h"
#include <string>
#include <filesystem>

namespace bvh {
	namespace utils {
		namespace win32_helper {
			BOOL OpenFileDialog(std::string* returned_file, const char* file_filter, const char* file_extension, BOOL isOpen);
			BOOL OpenFolderDialog(std::string* returned_folder, HWND owner);
			void GetTempFolder(std::filesystem::path* temp_folder);
			void GetVirtoolsFolder(std::filesystem::path* vt_folder);
			void CwndText2Stdstring(CWnd* ctrl, std::string* strl);
			BOOL CheckCPValidation(UINT cp);
		}
	}
}
