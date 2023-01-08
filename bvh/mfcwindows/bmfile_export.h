#pragma once

#include "../../stdafx.h"
#include "../utils/param_package.h"
#include <afxdialogex.h>
#include <vector>
#include <string>

namespace bvh {
	namespace mfcwindows {

		class BMFileExport : public CDialogEx {
			DECLARE_DYNAMIC(BMFileExport)

		public:
			BMFileExport(utils::ParamPackage* _pkg, CWnd* pParent = nullptr);   // 标准构造函数
			virtual ~BMFileExport();

			// 对话框数据
#ifdef AFX_DESIGN_TIME
			enum { IDD = IDD_DIALOG2 };
#endif

		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

			DECLARE_MESSAGE_MAP()

		public:
			virtual BOOL OnInitDialog();

			CButton m_Mode_Object;
			CButton m_Mode_Group;
			CButton m_Mode_All;
			CComboBox m_Target;
			CEdit m_BM_File;

			afx_msg void On_Mode_Change();
			afx_msg void On_Browse_BM();
			afx_msg void On_Dialog_OK();
			afx_msg void On_Dialog_Cancel();

			int OUT_Mode;
			CK_ID OUT_Target;
			std::wstring OUT_File;

		private:
			utils::ParamPackage* pkg;
			std::vector<CK_ID> comboboxMirror;
			int nowMode;

			std::wstring realBMFileStorage;

		};
	}
}
