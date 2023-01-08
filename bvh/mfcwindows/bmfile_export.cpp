#include "bmfile_export.h"
#include "../../resource.h"
#include "../utils/param_package.h"
#include "../utils/config_manager.h"
#include "../utils/win32_helper.h"
#include "../utils/string_helper.h"
#include <string>

namespace bvh {
	namespace mfcwindows {

		// BMFileExport 对话框

		IMPLEMENT_DYNAMIC(BMFileExport, CDialogEx)

			BMFileExport::BMFileExport(utils::ParamPackage* _pkg, CWnd* pParent /*=nullptr*/)
			: CDialogEx(IDD_DIALOG2, pParent),
			comboboxMirror(), nowMode(-1), realBMFileStorage(),
			OUT_Mode(2), OUT_Target(0), OUT_File(),
			pkg(_pkg) {
			;
		}

		BMFileExport::~BMFileExport() {
			;
		}

		void BMFileExport::DoDataExchange(CDataExchange* pDX) {
			CDialogEx::DoDataExchange(pDX);

			DDX_Control(pDX, IDC_RADIO1, m_Mode_Object);
			DDX_Control(pDX, IDC_RADIO2, m_Mode_Group);
			DDX_Control(pDX, IDC_RADIO3, m_Mode_All);

			DDX_Control(pDX, IDC_COMBO1, m_Target);
			DDX_Control(pDX, IDC_EDIT1, m_BM_File);
		}

		BOOL BMFileExport::OnInitDialog() {
			CDialogEx::OnInitDialog();

			// set mode
			switch (pkg->cfg_manager->CurrentConfig.window_mapping_bmExport_mode) {
				case 0:
					m_Mode_Object.SetCheck(1);
					break;
				case 1:
					m_Mode_Group.SetCheck(1);
					break;
				default:
					m_Mode_All.SetCheck(1);
					break;
			}
			//following function will change nowMode
			// nowMode = cfg_manager->CurrentConfig.window_mapping_bmExport_mode;
			On_Mode_Change();

			// set filename
			realBMFileStorage = pkg->cfg_manager->CurrentConfig.window_mapping_bmExport_filename.c_str();
			utils::win32_helper::StdWstring2CwndText(&m_BM_File, &realBMFileStorage);

			return TRUE;  // return TRUE unless you set the focus to a control
						  // 异常: OCX 属性页应返回 FALSE
		}

		BEGIN_MESSAGE_MAP(BMFileExport, CDialogEx)
			ON_BN_CLICKED(IDC_RADIO1, &BMFileExport::On_Mode_Change)
			ON_BN_CLICKED(IDC_RADIO2, &BMFileExport::On_Mode_Change)
			ON_BN_CLICKED(IDC_RADIO3, &BMFileExport::On_Mode_Change)

			ON_BN_CLICKED(IDOK, &BMFileExport::On_Dialog_OK)
			ON_BN_CLICKED(IDCANCEL, &BMFileExport::On_Dialog_Cancel)

			ON_BN_CLICKED(IDC_BUTTON1, &BMFileExport::On_Browse_BM)
		END_MESSAGE_MAP()


		// BMFileExport 消息处理程序

		void BMFileExport::On_Mode_Change() {
			int selectedMode = 0;
			if (m_Mode_Object.GetCheck() == 1) selectedMode = 0;
			else if (m_Mode_Group.GetCheck() == 1) selectedMode = 1;
			else selectedMode = 2;

			if (this->nowMode == selectedMode) return;
			this->nowMode = selectedMode;

			// change mode
			m_Target.ResetContent();
			comboboxMirror.clear();
			CKContext* context = pkg->plgif->GetCKContext();
			int count = 0;
			std::string format_str;
			if (m_Mode_Object.GetCheck() == 1) {
				XObjectPointerArray objArray = context->GetObjectListByType(CKCID_3DENTITY, TRUE);
				count = objArray.Size();
				for (int i = 0; i < count; i++) {
					comboboxMirror.push_back(objArray[i]->GetID());
					if (objArray[i]->GetName() != NULL)
						m_Target.AddString(objArray[i]->GetName());
					else {
						utils::string_helper::StdstringPrintf(&format_str, "[unnamed 3d object] (CKID: %d)", objArray[i]->GetID());
						m_Target.AddString(format_str.c_str());
					}
				}
			} else if (m_Mode_Group.GetCheck() == 1) {
				count = context->GetObjectsCountByClassID(CKCID_GROUP);
				CK_ID* idList = context->GetObjectsListByClassID(CKCID_GROUP);
				//iterate
				for (int i = 0; i < count; i++) {
					comboboxMirror.push_back(idList[i]);
					if (context->GetObjectA(idList[i])->GetName() != NULL)
						m_Target.AddString(context->GetObjectA(idList[i])->GetName());
					else {
						utils::string_helper::StdstringPrintf(&format_str, "[unnamed group] (CKID: %d)", idList[i]);
						m_Target.AddString(format_str.c_str());
					}
				}
			} else;

			m_Target.EnableWindow(m_Mode_All.GetCheck() != 1);
		}
		void BMFileExport::On_Browse_BM() {
			std::wstring filepath;
			if (utils::win32_helper::OpenFileDialog(&filepath, L"BM file(*.bmx)\0*.bmx\0", L"bmx", FALSE)) {
				realBMFileStorage = filepath.c_str();
				utils::win32_helper::StdWstring2CwndText(&m_BM_File, &realBMFileStorage);
			}

		}
		void BMFileExport::On_Dialog_OK() {
			// check data
			if (m_Mode_All.GetCheck() == 1)
				OUT_Mode = 2;
			else {
				int gottenIndex = m_Target.GetCurSel();
				if (gottenIndex == CB_ERR) {
					MessageBoxA("You should specific a export target.", "Setting error", MB_OK + MB_ICONERROR);
					return;
				}
				OUT_Target = comboboxMirror[gottenIndex];

				if (m_Mode_Object.GetCheck() == 1) OUT_Mode = 0;
				else OUT_Mode = 1;
			}

			if (realBMFileStorage.empty()) {
				MessageBoxA("Export file should not be empty.", "Setting error", MB_OK + MB_ICONERROR);
				return;
			}
			OUT_File = realBMFileStorage.c_str();

			// setting save
			pkg->cfg_manager->CurrentConfig.window_mapping_bmExport_mode = OUT_Mode;
			pkg->cfg_manager->CurrentConfig.window_mapping_bmExport_filename = OUT_File.c_str();
			pkg->cfg_manager->SaveConfig();

			CDialogEx::OnOK();
		}
		void BMFileExport::On_Dialog_Cancel() {
			CDialogEx::OnCancel();
		}
	}
}