#pragma once

#include "../../stdafx.h"
#include "../utils/param_package.h"
#include <afxdialogex.h>

namespace bvh {
	namespace mfcwindows {

		class BVHConfig : public CDialogEx {
			DECLARE_DYNAMIC(BVHConfig)

		public:
			BVHConfig(utils::ParamPackage* _pkg, CWnd* pParent = nullptr);   // 标准构造函数
			virtual ~BVHConfig();

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

		private:
			utils::ParamPackage* pkg;
			std::wstring realExtTexFolderStorage;
		};

	}
}
