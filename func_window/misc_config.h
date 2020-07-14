#if !defined(_YYCDLL_WINDOW_MISC_CONFIG_H__IMPORTED_)
#define _YYCDLL_WINDOW_MISC_CONFIG_H__IMPORTED_

#include "../stdafx.h"
#include <afxdialogex.h>

namespace func_window {

	class misc_config : public CDialogEx {
		DECLARE_DYNAMIC(misc_config)

		public:
		misc_config(CWnd* pParent = nullptr);   // 标准构造函数
		virtual ~misc_config();

		// 对话框数据
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_DIALOG1 };
#endif

		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

		DECLARE_MESSAGE_MAP()

		public:
		virtual BOOL OnInitDialog();

		CEdit m_BM_ExternalTextureFolder;
		CEdit m_BM_NoComponentGroup;

		afx_msg void On_BM_ExternalTextureFolderBrowse();
		afx_msg void On_Dialog_OK();
		afx_msg void On_Dialog_Cancel();
	};

}

#endif