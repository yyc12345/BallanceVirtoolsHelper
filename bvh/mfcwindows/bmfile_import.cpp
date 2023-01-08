#include "bmfile_import.h"
#include "../../resource.h"
#include "../utils/param_package.h"
#include "../utils/config_manager.h"
#include "../utils/win32_helper.h"
#include "../utils/string_helper.h"
#include <string>

namespace bvh {
	namespace mfcwindows {

		IMPLEMENT_DYNAMIC(BMFileImport, CDialogEx)

			BMFileImport::BMFileImport(utils::ParamPackage* _pkg, CWnd* pParent /*=nullptr*/)
			: CDialogEx(IDD_DIALOG4, pParent),
			OUT_rename_obj(TRUE), OUT_rename_mesh(TRUE), OUT_rename_mat(TRUE), OUT_rename_tex(TRUE),
			pkg(_pkg) {

		}

		BMFileImport::~BMFileImport() {
			;
		}

		void BMFileImport::DoDataExchange(CDataExchange* pDX) {
			CDialogEx::DoDataExchange(pDX);

			DDX_Control(pDX, IDC_RADIO1, m_Obj_Rename);
			DDX_Control(pDX, IDC_RADIO2, m_Obj_Current);
			DDX_Control(pDX, IDC_RADIO3, m_Mesh_Rename);
			DDX_Control(pDX, IDC_RADIO4, m_Mesh_Current);
			DDX_Control(pDX, IDC_RADIO5, m_Mat_Rename);
			DDX_Control(pDX, IDC_RADIO6, m_Mat_Current);
			DDX_Control(pDX, IDC_RADIO7, m_Tex_Rename);
			DDX_Control(pDX, IDC_RADIO8, m_Tex_Current);

		}

		BOOL BMFileImport::OnInitDialog() {
			CDialogEx::OnInitDialog();

			// apply data
			CButton* sel = NULL;
			sel = (BOOL)pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameObj ? &m_Obj_Rename : &m_Obj_Current;
			sel->SetCheck(1);
			sel = (BOOL)pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameMesh ? &m_Mesh_Rename : &m_Mesh_Current;
			sel->SetCheck(1);
			sel = (BOOL)pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameMat ? &m_Mat_Rename : &m_Mat_Current;
			sel->SetCheck(1);
			sel = (BOOL)pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameTex ? &m_Tex_Rename : &m_Tex_Current;
			sel->SetCheck(1);

			return TRUE;  // return TRUE unless you set the focus to a control
						  // 异常: OCX 属性页应返回 FALSE
		}

		BEGIN_MESSAGE_MAP(BMFileImport, CDialogEx)
			ON_BN_CLICKED(IDOK, &BMFileImport::On_Dialog_OK)
			ON_BN_CLICKED(IDCANCEL, &BMFileImport::On_Dialog_Cancel)
		END_MESSAGE_MAP()


		// BMFileImport 消息处理程序
		void BMFileImport::On_Dialog_OK() {
			// check data
			OUT_rename_obj = m_Obj_Rename.GetCheck() == 1;
			OUT_rename_mesh = m_Mesh_Rename.GetCheck() == 1;
			OUT_rename_mat = m_Mat_Rename.GetCheck() == 1;
			OUT_rename_tex = m_Tex_Rename.GetCheck() == 1;

			// setting save
			pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameObj = OUT_rename_obj;
			pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameMesh = OUT_rename_mesh;
			pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameMat = OUT_rename_mat;
			pkg->cfg_manager->CurrentConfig.window_mapping_bmImport_renameTex = OUT_rename_tex;
			pkg->cfg_manager->SaveConfig();

			CDialogEx::OnOK();
		}
		void BMFileImport::On_Dialog_Cancel() {
			CDialogEx::OnCancel();
		}
	}
}