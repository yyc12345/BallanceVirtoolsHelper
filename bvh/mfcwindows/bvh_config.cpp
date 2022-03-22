#include "bvh_config.h"
#include "../../resource.h"
#include "../utils/param_package.h"
#include "../utils/config_manager.h"
#include "../utils/win32_helper.h"
#include "../utils/string_helper.h"
#include <string>

namespace bvh {
	namespace mfcwindows {

		IMPLEMENT_DYNAMIC(BVHConfig, CDialogEx)

			BVHConfig::BVHConfig(utils::ParamPackage* _pkg, CWnd* pParent /*=nullptr*/)
			: CDialogEx(IDD_DIALOG1, pParent),
			pkg(_pkg) {

		}

		BVHConfig::~BVHConfig() {
		}

		void BVHConfig::DoDataExchange(CDataExchange* pDX) {
			DDX_Control(pDX, IDC_EDIT1, m_BM_ExternalTextureFolder);
			DDX_Control(pDX, IDC_EDIT2, m_BM_NoComponentGroup);
			DDX_Control(pDX, IDC_EDIT3, m_BM_OmittedMaterialPrefix);
		}


		BEGIN_MESSAGE_MAP(BVHConfig, CDialogEx)
			ON_BN_CLICKED(IDC_BUTTON1, &BVHConfig::On_BM_ExternalTextureFolderBrowse)
			ON_BN_CLICKED(IDOK, &BVHConfig::On_Dialog_OK)
			ON_BN_CLICKED(IDCANCEL, &BVHConfig::On_Dialog_Cancel)
		END_MESSAGE_MAP()

		BOOL BVHConfig::OnInitDialog() {
			CDialogEx::OnInitDialog();

			// load config
			m_BM_ExternalTextureFolder.SetWindowTextA(pkg->cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder.c_str());
			m_BM_NoComponentGroup.SetWindowTextA(pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName.c_str());
			m_BM_OmittedMaterialPrefix.SetWindowTextA(pkg->cfg_manager->CurrentConfig.func_mapping_bm_OmittedMaterialPrefix.c_str());
			return TRUE;  // return TRUE unless you set the focus to a control
						  // 异常: OCX 属性页应返回 FALSE
		}

#pragma region window_Config message processor

		void BVHConfig::On_BM_ExternalTextureFolderBrowse() {
			std::string cache;
			utils::win32_helper::OpenFolderDialog(&cache, m_hWnd);
			m_BM_ExternalTextureFolder.SetWindowTextA(cache.c_str());
		}

		void BVHConfig::On_Dialog_OK() {
			//save config
			utils::win32_helper::CwndText2Stdstring(&m_BM_ExternalTextureFolder, &(pkg->cfg_manager->CurrentConfig.func_mapping_bm_ExternalTextureFolder));
			utils::win32_helper::CwndText2Stdstring(&m_BM_NoComponentGroup, &(pkg->cfg_manager->CurrentConfig.func_mapping_bm_NoComponentGroupName));
			utils::win32_helper::CwndText2Stdstring(&m_BM_OmittedMaterialPrefix, &(pkg->cfg_manager->CurrentConfig.func_mapping_bm_OmittedMaterialPrefix));

			pkg->cfg_manager->SaveConfig();

			CDialogEx::OnOK();
		}

		void BVHConfig::On_Dialog_Cancel() {
			CDialogEx::OnCancel();
		}


#pragma endregion

	}
}