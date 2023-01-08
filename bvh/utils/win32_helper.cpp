#include "win32_helper.h"
#include "string_helper.h"

namespace bvh {
	namespace utils {
		namespace win32_helper {

			BOOL OpenFileDialog(std::wstring* returned_file, const wchar_t* file_filter, const wchar_t* file_extension, BOOL isOpen) {
				returned_file->resize(BVH_MAX_PATH);

				BOOL status;
				OPENFILENAMEW OpenFileStruct;
				ZeroMemory(&OpenFileStruct, sizeof(OPENFILENAMEW));
				OpenFileStruct.lStructSize = sizeof(OPENFILENAMEW);
				OpenFileStruct.lpstrFile = returned_file->data();
				OpenFileStruct.lpstrFile[0] = L'\0';
				OpenFileStruct.nMaxFile = returned_file->size();
				OpenFileStruct.lpstrFilter = file_filter;
				OpenFileStruct.lpstrDefExt = file_extension;
				OpenFileStruct.lpstrFileTitle = NULL;
				OpenFileStruct.nMaxFileTitle = 0;
				OpenFileStruct.lpstrInitialDir = NULL;
				OpenFileStruct.Flags = OFN_EXPLORER;
				if (isOpen) {
					status = GetOpenFileNameW(&OpenFileStruct);
				} else {
					status = GetSaveFileNameW(&OpenFileStruct);
				}

				if (!status) {
					returned_file->clear();
				}

				return status;
			}

			BOOL OpenFolderDialog(std::wstring* returned_folder, HWND owner) {
				std::wstring display_name(BVH_MAX_PATH, '\0');
				BOOL status;

				BROWSEINFOW folderViewer = { 0 };
				folderViewer.hwndOwner = owner;
				folderViewer.pidlRoot = NULL;
				folderViewer.pszDisplayName = display_name.data();
				folderViewer.pszDisplayName[0] = L'\0';
				folderViewer.lpszTitle = L"Pick a folder";
				folderViewer.lpfn = NULL;
				folderViewer.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
				PIDLIST_ABSOLUTE data = SHBrowseForFolderW(&folderViewer);
				if (data == NULL) {
					returned_folder->clear();
					return FALSE;
				}

				if (status = SHGetPathFromIDListW(data, display_name.data()))
					*returned_folder = display_name.c_str();
				else
					returned_folder->clear();

				CoTaskMemFree(data);
				return status;
			}

			void GetTempFolder(CKContext* ctx, std::filesystem::path* temp_folder) {
				CKPathManager* path_mgr = ctx->GetPathManager();
				XString vt_temp = path_mgr->GetVirtoolsTemporaryFolder();
				std::string pathcache;
				pathcache.resize(vt_temp.Length());
				memcpy(pathcache.data(), vt_temp.CStr(), vt_temp.Length());
				*temp_folder = pathcache.c_str();
			}

			void GetVirtoolsFolder(std::filesystem::path* vt_folder) {
				std::filesystem::path vtname;
				std::wstring pathcache(BVH_MAX_PATH, '\0');

				if (GetModuleFileNameW(NULL, pathcache.data(), pathcache.size()) == 0)
					throw std::runtime_error("Fail to get Virtools folder.");

				vtname = pathcache.c_str();
				*vt_folder = vtname.parent_path();
			}

			BOOL CheckCPValidation(UINT cp) {
				CPINFOEXW cpinfo;
				return GetCPInfoExW(cp, 0, &cpinfo);
			}

			void StdWstring2CwndText(CWnd* ctrl, std::wstring* strl) {
				std::string conv;
				if (utils::string_helper::Wstring2String(strl, &conv, CP_ACP)) {
					// set with converted string
					ctrl->SetWindowTextA(conv.c_str());
				} else {
					// fallback use shitty geek mixed W function
					HWND h_ctl = ctrl->GetSafeHwnd();
					::SetWindowTextW(h_ctl, strl->c_str());
				}
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

			BOOL CheckANSIPathValidation(std::string* ansi_path) {
				for (auto it = ansi_path->begin(); it != ansi_path->end(); it++) {
					switch (*it) {
						case '*':
						case '?':
						case '"':
						case '<':
						case '>':
						case '|':
							return FALSE;
						default:
							break;
					}
				}
				return TRUE;
			}

		}
	}
}