#include "window_BMExport.h"
#include "../resource.h"
#include "../config_manager.h"
#include "../func_helper.h"
#include <string>

extern PluginInterface* s_Plugininterface;
extern config_manager* cfg_manager;

namespace func_window {

	// window_BMExport 对话框

	IMPLEMENT_DYNAMIC(window_BMExport, CDialogEx)

	window_BMExport::window_BMExport(CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_DIALOG2, pParent),
		comboboxMirror(),
		nowMode(-1),
		OUT_Mode(2),
		OUT_Target(0),
		OUT_File() {
		;
	}

	window_BMExport::~window_BMExport() {
		;
	}

	void window_BMExport::DoDataExchange(CDataExchange* pDX) {
		CDialogEx::DoDataExchange(pDX);

		DDX_Control(pDX, IDC_RADIO1, m_Mode_Object);
		DDX_Control(pDX, IDC_RADIO2, m_Mode_Group);
		DDX_Control(pDX, IDC_RADIO3, m_Mode_All);

		DDX_Control(pDX, IDC_COMBO1, m_Target);
		DDX_Control(pDX, IDC_EDIT1, m_BM_File);
	}

	BOOL window_BMExport::OnInitDialog() {
		CDialogEx::OnInitDialog();
		
		switch (cfg_manager->CurrentConfig.window_mapping_bmExport_mode) {
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

		m_BM_File.SetWindowTextA(cfg_manager->CurrentConfig.window_mapping_bmExport_filename.c_str());

		return TRUE;  // return TRUE unless you set the focus to a control
					  // 异常: OCX 属性页应返回 FALSE
	}

	BEGIN_MESSAGE_MAP(window_BMExport, CDialogEx)
		ON_BN_CLICKED(IDC_RADIO1, &window_BMExport::On_Mode_Change)
		ON_BN_CLICKED(IDC_RADIO2, &window_BMExport::On_Mode_Change)
		ON_BN_CLICKED(IDC_RADIO3, &window_BMExport::On_Mode_Change)

		ON_BN_CLICKED(IDOK, &window_BMExport::On_Dialog_OK)
		ON_BN_CLICKED(IDCANCEL, &window_BMExport::On_Dialog_Cancel)

		ON_BN_CLICKED(IDC_BUTTON1, &window_BMExport::On_Browse_BM)
	END_MESSAGE_MAP()


	// window_BMExport 消息处理程序

	void window_BMExport::On_Mode_Change() {
		int selectedMode = 0;
		if (m_Mode_Object.GetCheck() == 1) selectedMode = 0;
		else if (m_Mode_Group.GetCheck() == 1) selectedMode = 1;
		else selectedMode = 2;

		if (this->nowMode == selectedMode) return;
		this->nowMode = selectedMode;

		// change mode
		m_Target.ResetContent();
		comboboxMirror.clear();
		CKContext* context = s_Plugininterface->GetCKContext();
		int count = 0;
		if (m_Mode_Object.GetCheck() == 1) {
			XObjectPointerArray objArray = context->GetObjectListByType(CKCID_3DENTITY, TRUE);
			count = objArray.Size();
			for (int i = 0; i < count; i++) {
				comboboxMirror.push_back(objArray[i]->GetID());
				if (objArray[i]->GetName() != NULL)
					m_Target.AddString(objArray[i]->GetName());
				else {
					sprintf(func_namespace::ExecutionCache, "[unnamed 3d object] (CKID: %d)", objArray[i]->GetID());
					m_Target.AddString(func_namespace::ExecutionCache);
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
					sprintf(func_namespace::ExecutionCache, "[unnamed group] (CKID: %d)", idList[i]);
					m_Target.AddString(func_namespace::ExecutionCache);
				}
			}
		} else;

		m_Target.EnableWindow(m_Mode_All.GetCheck() != 1);
	}
	void window_BMExport::On_Browse_BM() {
		std::string filepath;
		if (func_namespace::OpenFileDialog(&filepath, "BM file(*.bm)\0*.bm\0", "bm", FALSE))
			m_BM_File.SetWindowTextA(filepath.c_str());
	}
	void window_BMExport::On_Dialog_OK() {
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

		m_BM_File.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		OUT_File = func_namespace::ExecutionCache;
		if (OUT_File.empty()) {
			MessageBoxA("Export should not be empty.", "Setting error", MB_OK + MB_ICONERROR);
			return;
		}

		// setting save
		cfg_manager->CurrentConfig.window_mapping_bmExport_mode = OUT_Mode;
		cfg_manager->CurrentConfig.window_mapping_bmExport_filename = OUT_File;
		cfg_manager->SaveConfig();

		CDialogEx::OnOK();
	}
	void window_BMExport::On_Dialog_Cancel() {
		CDialogEx::OnCancel();
	}
}