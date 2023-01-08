#pragma once

#include "../../stdafx.h"
#include "../utils/param_package.h"
#include <afxdialogex.h>
#include <vector>
#include <string>

namespace bvh {
	namespace mfcwindows {

		class BMFileImport : public CDialogEx {
			DECLARE_DYNAMIC(BMFileImport)

		public:
			BMFileImport(utils::ParamPackage* _pkg, CWnd* pParent = nullptr);   // 标准构造函数
			virtual ~BMFileImport();

			// 对话框数据
#ifdef AFX_DESIGN_TIME
			enum { IDD = IDD_DIALOG4 };
#endif

		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

			DECLARE_MESSAGE_MAP()

		public:
			virtual BOOL OnInitDialog();

			CButton m_Obj_Rename, m_Obj_Current;
			CButton m_Mesh_Rename, m_Mesh_Current;
			CButton m_Mat_Rename, m_Mat_Current;
			CButton m_Tex_Rename, m_Tex_Current;

			afx_msg void On_Dialog_OK();
			afx_msg void On_Dialog_Cancel();

			BOOL OUT_rename_obj, OUT_rename_mesh, OUT_rename_mat, OUT_rename_tex;

		private:
			utils::ParamPackage* pkg;

		};

	}
}