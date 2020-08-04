#if !defined(_YYCDLL_WINDOW_MAPPING_BMEXPORT_H__IMPORTED_)
#define _YYCDLL_WINDOW_MAPPING_BMEXPORT_H__IMPORTED_

#include "../stdafx.h"
#include <afxdialogex.h>
#include <vector>
#include <string>

namespace func_window {

	class window_BMExport : public CDialogEx {
		DECLARE_DYNAMIC(window_BMExport)

		public:
		window_BMExport(CWnd* pParent = nullptr);   // 标准构造函数
		virtual ~window_BMExport();

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
		std::string OUT_File;

		private:
		std::vector<CK_ID> comboboxMirror;
		int nowMode;
	};
}
#endif