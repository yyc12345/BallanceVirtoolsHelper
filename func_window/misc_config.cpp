#include "misc_config.h"
#include "../resource1.h"
#include "../config_manager.h"
#include "../func_helper.h"
#include <string>

extern config_manager* cfg_manager;

namespace func_window {

	IMPLEMENT_DYNAMIC(misc_config, CDialogEx)

	misc_config::misc_config(CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_DIALOG1, pParent) {

	}

	misc_config::~misc_config() {
	}

	void misc_config::DoDataExchange(CDataExchange* pDX) {
		DDX_Control(pDX, IDC_EDIT1, m_BM_ExternalTextureFolder);
		DDX_Control(pDX, IDC_EDIT2, m_BM_NoComponentGroup);
	}


	BEGIN_MESSAGE_MAP(misc_config, CDialogEx)
		ON_BN_CLICKED(IDC_BUTTON1, &misc_config::On_BM_ExternalTextureFolderBrowse)
		ON_BN_CLICKED(IDOK, &misc_config::On_Dialog_OK)
		ON_BN_CLICKED(IDCANCEL, &misc_config::On_Dialog_Cancel)
	END_MESSAGE_MAP()

	BOOL misc_config::OnInitDialog() {
		CDialogEx::OnInitDialog();
		
		// load config
		m_BM_ExternalTextureFolder.SetWindowTextA(cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder.c_str());
		m_BM_NoComponentGroup.SetWindowTextA(cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str());
		return TRUE;  // return TRUE unless you set the focus to a control
					  // 异常: OCX 属性页应返回 FALSE
	}

#pragma region misc_config message processor

	void misc_config::On_BM_ExternalTextureFolderBrowse() {
		std::string cache;
		func_namespace::OpenFolderDialog(&cache, m_hWnd);
		m_BM_ExternalTextureFolder.SetWindowTextA(cache.c_str());
	}

	void misc_config::On_Dialog_OK() {
		//save config
		m_BM_ExternalTextureFolder.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder = func_namespace::ExecutionCache;
		m_BM_NoComponentGroup.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName = func_namespace::ExecutionCache;

		cfg_manager->SaveConfig();

		CDialogEx::OnOK();
	}

	void misc_config::On_Dialog_Cancel() {
		CDialogEx::OnCancel();
	}


#pragma endregion

}