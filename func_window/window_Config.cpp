#include "window_Config.h"
#include "../resource.h"
#include "../config_manager.h"
#include "../func_helper.h"
#include <string>

extern config_manager* cfg_manager;

namespace func_window {

	IMPLEMENT_DYNAMIC(window_Config, CDialogEx)

	window_Config::window_Config(CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_DIALOG1, pParent) {

	}

	window_Config::~window_Config() {
	}

	void window_Config::DoDataExchange(CDataExchange* pDX) {
		DDX_Control(pDX, IDC_EDIT1, m_BM_ExternalTextureFolder);
		DDX_Control(pDX, IDC_EDIT2, m_BM_NoComponentGroup);
		DDX_Control(pDX, IDC_EDIT3, m_BM_OmittedMaterialPrefix);
	}


	BEGIN_MESSAGE_MAP(window_Config, CDialogEx)
		ON_BN_CLICKED(IDC_BUTTON1, &window_Config::On_BM_ExternalTextureFolderBrowse)
		ON_BN_CLICKED(IDOK, &window_Config::On_Dialog_OK)
		ON_BN_CLICKED(IDCANCEL, &window_Config::On_Dialog_Cancel)
	END_MESSAGE_MAP()

	BOOL window_Config::OnInitDialog() {
		CDialogEx::OnInitDialog();
		
		// load config
		m_BM_ExternalTextureFolder.SetWindowTextA(cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder.c_str());
		m_BM_NoComponentGroup.SetWindowTextA(cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str());
		m_BM_OmittedMaterialPrefix.SetWindowTextA(cfg_manager->CurrentConfig.func_mapping_bm_OmittedMaterialPrefix.c_str());
		return TRUE;  // return TRUE unless you set the focus to a control
					  // 异常: OCX 属性页应返回 FALSE
	}

#pragma region window_Config message processor

	void window_Config::On_BM_ExternalTextureFolderBrowse() {
		std::string cache;
		func_namespace::OpenFolderDialog(&cache, m_hWnd);
		m_BM_ExternalTextureFolder.SetWindowTextA(cache.c_str());
	}

	void window_Config::On_Dialog_OK() {
		//save config
		m_BM_ExternalTextureFolder.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder = func_namespace::ExecutionCache;
		m_BM_NoComponentGroup.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName = func_namespace::ExecutionCache;
		m_BM_OmittedMaterialPrefix.GetWindowTextA(func_namespace::ExecutionCache, CACHE_SIZE);
		cfg_manager->CurrentConfig.func_mapping_bm_OmittedMaterialPrefix = func_namespace::ExecutionCache;

		cfg_manager->SaveConfig();

		CDialogEx::OnOK();
	}

	void window_Config::On_Dialog_Cancel() {
		CDialogEx::OnCancel();
	}


#pragma endregion

}