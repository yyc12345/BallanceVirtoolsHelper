#include "convert_encoding.h"
#include "../../resource.h"
#include "../utils/param_package.h"
#include "../utils/config_manager.h"
#include "../utils/win32_helper.h"
#include "../utils/string_helper.h"

namespace bvh {
	namespace mfcwindows {

		// ConvertEncoding 对话框

		IMPLEMENT_DYNAMIC(ConvertEncoding, CDialogEx)

			ConvertEncoding::ConvertEncoding(utils::ParamPackage* _pkg, CWnd* pParent /*=nullptr*/)
			: CDialogEx(IDD_DIALOG3, pParent),
			OUT_OldCP(437),
			OUT_NewCP(437),
			OUT_RunMethod(0),
			pkg(_pkg)
		{
			;
		}

		ConvertEncoding::~ConvertEncoding() {
			;
		}

		void ConvertEncoding::DoDataExchange(CDataExchange* pDX) {
			CDialogEx::DoDataExchange(pDX);

			DDX_Control(pDX, IDC_COMBO1, m_Method);
			DDX_Control(pDX, IDC_COMBO2, m_OldCP);
			DDX_Control(pDX, IDC_EDIT1, m_ManualOldCP);
			DDX_Control(pDX, IDC_COMBO3, m_NewCP);
			DDX_Control(pDX, IDC_EDIT2, m_ManualNewCP);
		}

		BOOL ConvertEncoding::OnInitDialog() {
			CDialogEx::OnInitDialog();

			m_Method.SetCurSel(pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_Method);
			m_OldCP.SetCurSel(pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_OldCP);
			m_ManualOldCP.SetWindowTextA(pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_ManualOldCP.c_str());
			m_NewCP.SetCurSel(pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_NewCP);
			m_ManualNewCP.SetWindowTextA(pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_ManualNewCP.c_str());

			// trigger this for ui update
			On_OldCPChanged();
			On_NewCPChanged();

			return TRUE;  // return TRUE unless you set the focus to a control
						  // 异常: OCX 属性页应返回 FALSE
		}

		BEGIN_MESSAGE_MAP(ConvertEncoding, CDialogEx)
			ON_BN_CLICKED(IDC_BUTTON1, &ConvertEncoding::On_CPWebsite)

			ON_CBN_SELCHANGE(IDC_COMBO2, &ConvertEncoding::On_OldCPChanged)
			ON_CBN_SELCHANGE(IDC_COMBO3, &ConvertEncoding::On_NewCPChanged)

			ON_BN_CLICKED(IDOK, &ConvertEncoding::On_Dialog_OK)
			ON_BN_CLICKED(IDCANCEL, &ConvertEncoding::On_Dialog_Cancel)
		END_MESSAGE_MAP()

		// ConvertEncoding 消息处理程序

		afx_msg void ConvertEncoding::On_CPWebsite() {
			ShellExecute(NULL, "open", "https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers", NULL, NULL, SW_SHOWNORMAL);
		}
		afx_msg void ConvertEncoding::On_OldCPChanged() {
			m_ManualOldCP.EnableWindow(m_OldCP.GetCurSel() == 7);
		}
		afx_msg void ConvertEncoding::On_NewCPChanged() {
			m_ManualNewCP.EnableWindow(m_NewCP.GetCurSel() == 7);
		}
		afx_msg void ConvertEncoding::On_Dialog_OK() {
			int cursel;
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
				if (!TryGetManualCP(&m_ManualOldCP, &OUT_OldCP)) {
					MessageBoxA("Original manual code page is illegal.", "Setting error", MB_OK + MB_ICONERROR);
					return;
				}
			}

			if ((cursel = m_NewCP.GetCurSel()) == CB_ERR) {
				MessageBoxA("You should choose a target encoding", "Setting error", MB_OK + MB_ICONERROR);
				return;
			}
			if (cursel != 7) OUT_NewCP = GetCPFromList(cursel);
			else {
				if (!TryGetManualCP(&m_ManualNewCP, &OUT_NewCP)) {
					MessageBoxA("Target manual code page is illegal.", "Setting error", MB_OK + MB_ICONERROR);
					return;
				}
			}

			if (OUT_NewCP == OUT_OldCP) {
				MessageBoxA("Original and Target encoding shouldn't be the same encoding.", "Setting error", MB_OK + MB_ICONERROR);
				return;
			}

			// save settings
			pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_Method = m_Method.GetCurSel();
			pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_OldCP = m_OldCP.GetCurSel();
			utils::win32_helper::CwndText2Stdstring(&m_ManualOldCP, &(pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_ManualOldCP));
			pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_NewCP = m_NewCP.GetCurSel();
			utils::win32_helper::CwndText2Stdstring(&m_ManualNewCP, &(pkg->cfg_manager->CurrentConfig.window_ConvertEncoding_ManualNewCP));

			pkg->cfg_manager->SaveConfig();

			CDialogEx::OnOK();
		}
		afx_msg void ConvertEncoding::On_Dialog_Cancel() {
			CDialogEx::OnCancel();
		}


		UINT ConvertEncoding::GetCPFromList(int index) {
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
		BOOL ConvertEncoding::TryGetManualCP(CEdit* control, UINT* cp) {
			std::string pending_str;
			utils::win32_helper::CwndText2Stdstring(control, &pending_str);
			if (pending_str.empty()) {
				// fail to get string
				cp = CP_ACP;
				return FALSE;
			}

			if (!utils::string_helper::Stdstring2Uint(&pending_str, cp)) {
				// fail to parse to uint
				cp = CP_ACP;
				return FALSE;
			}

			if (!utils::win32_helper::CheckCPValidation(*cp)) {
				// cp is invalid
				cp = CP_ACP;
				return FALSE;
			}

			return TRUE;
		}

	}
}