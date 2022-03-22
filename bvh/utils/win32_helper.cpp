#include "win32_helper.h"

namespace bvh {
	namespace utils {
		namespace win32_helper {

			BOOL OpenFileDialog(std::string* returned_file, const char* file_filter, const char* file_extension, BOOL isOpen) {
				returned_file->resize(MAX_PATH);

				BOOL status;
				OPENFILENAMEA OpenFileStruct;
				ZeroMemory(&OpenFileStruct, sizeof(OPENFILENAMEA));
				OpenFileStruct.lStructSize = sizeof(OPENFILENAMEA);
				OpenFileStruct.lpstrFile = returned_file->data();
				OpenFileStruct.lpstrFile[0] = '\0';
				OpenFileStruct.nMaxFile = MAX_PATH;
				OpenFileStruct.lpstrFilter = file_filter;
				OpenFileStruct.lpstrDefExt = file_extension;
				OpenFileStruct.lpstrFileTitle = NULL;
				OpenFileStruct.nMaxFileTitle = 0;
				OpenFileStruct.lpstrInitialDir = NULL;
				OpenFileStruct.Flags = OFN_EXPLORER;
				if (isOpen) {
					status = GetOpenFileNameA(&OpenFileStruct);
				} else {
					status = GetSaveFileNameA(&OpenFileStruct);
				}

				if (!status) {
					returned_file->clear();
				}

				return status;
			}

			BOOL OpenFolderDialog(std::string* returned_folder, HWND owner) {
				std::string display_name(MAX_PATH, '\0');
				BOOL status;

				BROWSEINFOA folderViewer = { 0 };
				folderViewer.hwndOwner = owner;
				folderViewer.pidlRoot = NULL;
				folderViewer.pszDisplayName = display_name.data();
				folderViewer.pszDisplayName[0] = '\0';
				folderViewer.lpszTitle = "Pick a folder";
				folderViewer.lpfn = NULL;
				folderViewer.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
				PIDLIST_ABSOLUTE data = SHBrowseForFolder(&folderViewer);
				if (data == NULL) return FALSE;
				if (status = SHGetPathFromIDListA(data, display_name.data()))
					*returned_folder = display_name.c_str();
				else
					returned_folder->clear();

				CoTaskMemFree(data);
				return status;
			}

			void GetTempFolder(std::filesystem::path* temp_folder) {
				std::string pathcache(MAX_PATH, '\0');
				GetTempPathA(MAX_PATH, pathcache.data());
				*temp_folder = pathcache.c_str();
			}

			void GetVirtoolsFolder(std::filesystem::path* vt_folder) {
				std::filesystem::path vtname;
				std::string pathcache(MAX_PATH, '\0');

				GetModuleFileNameA(NULL, pathcache.data(), MAX_PATH);
				vtname = pathcache.c_str();
				*vt_folder = vtname.parent_path();
			}

			BOOL CheckCPValidation(UINT cp) {
				CPINFOEXA cpinfo;
				return GetCPInfoExA(cp, 0, &cpinfo);
			}

			void CwndText2Stdstring(CWnd* ctrl, std::string* strl) {
				int count = ctrl->GetWindowTextLengthA();
				if (count == 0) {
					strl->clear();
					return;
				}

				count++;
				strl->resize(count);
				ctrl->GetWindowTextA(strl->data(), count);
			}

		}
	}
}