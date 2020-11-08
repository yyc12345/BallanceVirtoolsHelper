#if !defined(_YYCDLL_WINDOW_MISC_CONFIG_H__IMPORTED_)
#define _YYCDLL_WINDOW_MISC_CONFIG_H__IMPORTED_

#include "../stdafx.h"
#include <afxdialogex.h>

namespace func_window {

	class window_Config : public CDialogEx {
		DECLARE_DYNAMIC(window_Config)

		public:
		window_Config(CWnd* pParent = nullptr);   // 标准构造函数
		virtual ~window_Config();

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
		CEdit m_BM_OmittedMaterialPrefix;

		afx_msg void On_BM_ExternalTextureFolderBrowse();
		afx_msg void On_Dialog_OK();
		afx_msg void On_Dialog_Cancel();
	};

}

#endif