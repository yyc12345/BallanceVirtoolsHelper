#include "window_ConvertEncoding.h"
#include "../resource.h"
#include "../config_manager.h"
#include "../func_helper.h"

extern config_manager* cfg_manager;

namespace func_window {

	// window_ConvertEncoding 对话框

	IMPLEMENT_DYNAMIC(window_ConvertEncoding, CDialogEx)

	window_ConvertEncoding::window_ConvertEncoding(CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_DIALOG3, pParent),
		OUT_OldCP(437),
		OUT_NewCP(437),
		OUT_RunMethod(0) {
		;
	}

	window_ConvertEncoding::~window_ConvertEncoding() {
		;
	}

	void window_ConvertEncoding::DoDataExchange(CDataExchange* pDX) {
		CDialogEx::DoDataExchange(pDX);

		DDX_Control(pDX, IDC_COMBO1, m_Method);
		DDX_Control(pDX, IDC_COMBO2, m_OldCP);
		DDX_Control(pDX, IDC_EDIT1, m_ManualOldCP);
		DDX_Control(pDX, IDC_COMBO3, m_NewCP);
		DDX_Control(pDX, IDC_EDIT2, m_ManualNewCP);
	}

	BOOL window_ConvertEncoding::OnInitDialog() {
		CDialogEx::OnInitDialog();

		m_Method.SetCurSel(cfg_manager->CurrentConfig.window_ConvertEncoding_Method);
		m_OldCP.SetCurSel(cfg_manager->CurrentConfig.window_ConvertEncoding_OldCP);
		m_ManualOldCP.SetWindowTextA(cfg_manager->CurrentConfig.window_ConvertEncoding_ManualOldCP.c_str());
		m_NewCP.SetCurSel(cfg_manager->CurrentConfig.window_ConvertEncoding_NewCP);
		m_ManualNewCP.SetWindowTextA(cfg_manager->CurrentConfig.window_ConvertEncoding_ManualNewCP.c_str());

		// trigger this for ui update
		On_OldCPChanged();
		On_NewCPChanged();

		return TRUE;  // return TRUE unless you set the focus to a control
					  // 异常: OCX 属性页应返回 FALSE
	}

	BEGIN_MESSAGE_MAP(window_ConvertEncoding, CDialogEx)
		ON_BN_CLICKED(IDC_BUTTON1, &window_ConvertEncoding::On_CPWebsite)

		ON_CBN_SELCHANGE(IDC_COMBO2, &window_ConvertEncoding::On_OldCPChanged)
		ON_CBN_SELCHANGE(IDC_COMBO3, &window_ConvertEncoding::On_NewCPChanged)

		ON_BN_CLICKED(IDOK, &window_ConvertEncoding::On_Dialog_OK)
		ON_BN_CLICKED(IDCANCEL, &window_ConvertEncoding::On_Dialog_Cancel)
	END_MESSAGE_MAP()

	// window_ConvertEncoding 消息处理程序

	afx_msg void window_ConvertEncoding::On_CPWebsite() {
		ShellExecute(NULL, "open", "https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers", NULL, NULL, SW_SHOWNORMAL);
	}
	afx_msg void window_ConvertEncoding::On_OldCPChanged() {
		m_ManualOldCP.EnableWindow(m_OldCP.GetCurSel() == 7);
	}
	afx_msg void window_ConvertEncoding::On_NewCPChanged() {
		m_ManualNewCP.EnableWindow(m_NewCP.GetCurSel() == 7);
	}
	afx_msg void window_ConvertEncoding::On_Dialog_OK() {
		int cursel;
		LONG cache;
		if ((cursel = m_Method.GetCurSel()) == CB_ERR) {
			MessageBoxA("You should choose a method", "Setting error", MB_OK + MB_ICONERROR);
			return;
		}
		OUT_RunMethod = cursel;

		if ((cursel = m_OldCP.GetCurSel()) == CB_ERR) {
			MessageBoxA("You should choose a original encoding", "Setting error", MB_OK + MB_ICONERROR);
			return;
		}
		if (cursel != 7) OUT_OldCP = GetCPFromList(cursel);
		else {
			if ((cache = TryGetManualCP(&m_ManualOldCP)) == -1) {
				MessageBoxA("Original manual code page is illegal.", "Setting error", MB_OK + MB_ICONERROR);
				return;
			} else OUT_OldCP = (UINT)cache;
		}

		if ((cursel = m_NewCP.GetCurSel()) == CB_ERR) {
			MessageBoxA("You should choose a target encoding", "Setting error", MB_OK + MB_ICONERROR);
			return;
		}
		if (cursel != 7) OUT_NewCP = GetCPFromList(cursel);
		else {
			if ((cache = TryGetManualCP(&m_ManualNewCP)) == -1) {
				MessageBoxA("Target manual code page is illegal.", "Setting error", MB_OK + MB_ICONERROR);
				return;
			} else OUT_NewCP = (UINT)cache;
		}
		
		if (OUT_NewCP == OUT_OldCP) {
			MessageBoxA("Original and Target encoding shouldn't be the same encoding.", "Setting error", MB_OK + MB_ICONERROR);
			return;
		}

		// save settings
		cfg_manager->CurrentConfig.window_ConvertEncoding_Method = m_Method.GetCurSel();
		cfg_manager->CurrentConfig.window_ConvertEncoding_OldCP = m_OldCP.GetCurSel();
		m_ManualOldCP.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		cfg_manager->CurrentConfig.window_ConvertEncoding_ManualOldCP = func_namespace::ExecutionCache;
		cfg_manager->CurrentConfig.window_ConvertEncoding_NewCP = m_NewCP.GetCurSel();
		m_ManualNewCP.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		cfg_manager->CurrentConfig.window_ConvertEncoding_ManualNewCP = func_namespace::ExecutionCache;

		CDialogEx::OnOK();
	}
	afx_msg void window_ConvertEncoding::On_Dialog_Cancel() {
		CDialogEx::OnCancel();
	}


	UINT window_ConvertEncoding::GetCPFromList(int index) {
		switch (index) {
			case 0:
				return CP_ACP;
			case 1:
				return CP_MACCP;
			case 2:
				return CP_OEMCP;
			case 3:
				return CP_SYMBOL;
			case 4:
				return CP_THREAD_ACP;
			case 5:
				return CP_UTF7;
			case 6:
				return CP_UTF8;
			default:
				throw std::runtime_error("Unexpected index in CP choose list");
		}
	}
	LONG window_ConvertEncoding::TryGetManualCP(CEdit* control) {
		control->GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		if (strlen(func_namespace::ExecutionCache) == 0) return -1;
		int gotten;
		gotten = atoi(func_namespace::ExecutionCache);
		itoa(gotten, func_namespace::ExecutionCache2, 10);
		if (strcmp(func_namespace::ExecutionCache, func_namespace::ExecutionCache2) == 0)
			return gotten;
		else return -1;
	}

}