#include "win32_helper.h"
#include "string_helper.h"
#include <DbgHelp.h>

namespace bvh::utils::win32_helper {

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

#pragma region Exception Proc

	static const char* UExceptionGetCodeName(DWORD code) {
		switch (code) {
			case EXCEPTION_ACCESS_VIOLATION:
				return "access violation";
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				return "array index out of bound";
			case EXCEPTION_BREAKPOINT:
				return "breakpoint reached";
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				return "misaligned data access";
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				return "operand had denormal value";
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				return "floating-point division by zero";
			case EXCEPTION_FLT_INEXACT_RESULT:
				return "no decimal fraction representation for value";
			case EXCEPTION_FLT_INVALID_OPERATION:
				return "invalid floating-point operation";
			case EXCEPTION_FLT_OVERFLOW:
				return "floating-point overflow";
			case EXCEPTION_FLT_STACK_CHECK:
				return "floating-point stack corruption";
			case EXCEPTION_FLT_UNDERFLOW:
				return "floating-point underflow";
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				return "illegal instruction";
			case EXCEPTION_IN_PAGE_ERROR:
				return "inaccessible page";
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				return "integer division by zero";
			case EXCEPTION_INT_OVERFLOW:
				return "integer overflow";
			case EXCEPTION_INVALID_DISPOSITION:
				return "documentation says this should never happen";
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				return "can't continue after a noncontinuable exception";
			case EXCEPTION_PRIV_INSTRUCTION:
				return "attempted to execute a privileged instruction";
			case EXCEPTION_SINGLE_STEP:
				return "one instruction has been executed";
			case EXCEPTION_STACK_OVERFLOW:
				return "stack overflow";
		}
		return "unknown exception";
	}

	static void UExceptionBacktrace(FILE* fs, LPCONTEXT context, int maxdepth) {
		// setup loading symbol options
		SymSetOptions(SymGetOptions() | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);	// lazy load symbol, and load line number.

		// setup handle
		HANDLE process = GetCurrentProcess();
		HANDLE thread = GetCurrentThread();

		// init symbol
		if (!SymInitialize(process, 0, TRUE)) {
			// fail to load. return
			fputs("Lost symbol file!\n", fs);
			return;
		}

		// ========== CORE DUMP ==========
		// prepare frame. setup correct fields
		DWORD machine_type = 0;
		STACKFRAME64 frame;
		memset(&frame, 0, sizeof(frame));
#if defined(_M_IX86) || defined(__i386__)
		machine_type = IMAGE_FILE_MACHINE_I386;
		frame.AddrPC.Offset = context->Eip;
		frame.AddrStack.Offset = context->Esp;
		frame.AddrFrame.Offset = context->Ebp;
#elif defined(_M_IX64) || defined(__amd64__)
		machine_type = IMAGE_FILE_MACHINE_AMD64;
		frame.AddrPC.Offset = context->Rip;
		frame.AddrStack.Offset = context->Rsp;
		frame.AddrFrame.Offset = context->Rbp;
#else
#error "Unsupported platform"
		//IA-64 anybody?
#endif
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrStack.Mode = AddrModeFlat;
		frame.AddrFrame.Mode = AddrModeFlat;

		// other variables
		char module_name_raw[MAX_PATH];

		// stack walker
		while (StackWalk64(machine_type, process, thread, &frame, context,
			0, SymFunctionTableAccess64, SymGetModuleBase64, 0)) {

			// depth breaker
			--maxdepth;
			if (maxdepth < 0) {
				fputs("...\n", fs);		// indicate there are some frames not listed
				break;
			}

			// get module name
			DWORD64 module_base = SymGetModuleBase64(process, frame.AddrPC.Offset);
			const char* module_name = "[unknown module]";
			if (module_base && GetModuleFileNameA((HINSTANCE)module_base, module_name_raw, MAX_PATH)) {
				module_name = module_name_raw;
			}

			// get source file and line
			const char* source_file = "[unknow_source_file]";
			DWORD64 source_file_line = 0;
			DWORD dwDisplacement;
			IMAGEHLP_LINE64 winline;
			winline.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &dwDisplacement, &winline)) {
				source_file = winline.FileName;
				source_file_line = winline.LineNumber;
			}

			// write to file
			fprintf(fs, "0x%08llx(rel: 0x%08llx)[%s]\t%s#%llu\n",
				frame.AddrPC.Offset, frame.AddrPC.Offset - module_base, module_name,
				source_file, source_file_line
			);

		}

		// ========== END CORE DUMP ==========

		// free symbol
		SymCleanup(process);
	}

	static LONG WINAPI UExceptionImpl(LPEXCEPTION_POINTERS info) {
		// create stream first
		std::filesystem::path logpath;
		GetVirtoolsFolder(&logpath);
		logpath /= "BallanceVirtoolsHelper.log";
		FILE* fs = _wfopen(logpath.wstring().c_str(), L"w");
		if (fs != nullptr) {

			// record exception type first
			PEXCEPTION_RECORD rec = info->ExceptionRecord;
			fprintf(fs, "Unhandled exception occured at 0x%08p: %s (%lu).\n",
				rec->ExceptionAddress,
				UExceptionGetCodeName(rec->ExceptionCode),
				rec->ExceptionCode
			);

			// special proc for 2 exceptions
			if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || rec->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) {
				if (rec->NumberParameters >= 2) {
					const char* op =
						rec->ExceptionInformation[0] == 0 ? "read" :
						rec->ExceptionInformation[0] == 1 ? "written" : "executed";
					fprintf(fs, "The data at memory address 0x%08x could not be %s.\n",
						rec->ExceptionInformation[1], op);
				}
			}

			// output stacktrace
			UExceptionBacktrace(fs, info->ContextRecord, 128);

			fclose(fs);
		}

		// continue proc exception
		return EXCEPTION_CONTINUE_SEARCH;
	}

	UnhandledExceptionGuard::UnhandledExceptionGuard() :
		mHasSet(false), mProcBackup(nullptr) {
		;
	}
	UnhandledExceptionGuard::~UnhandledExceptionGuard() {
		;
	}

	void UnhandledExceptionGuard::BacktraceRegister(void) {
		if (mHasSet) return;

		mProcBackup = SetUnhandledExceptionFilter(UExceptionImpl);
		mHasSet = true;
	}

	void UnhandledExceptionGuard::BacktraceUnregister(void) {
		if (!mHasSet) return;

		SetUnhandledExceptionFilter(mProcBackup);
		mHasSet = false;
	}

#pragma endregion


}