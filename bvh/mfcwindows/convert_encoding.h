#pragma once

#include "../../stdafx.h"
#include "../utils/param_package.h"
#include <afxdialogex.h>

namespace bvh {
	namespace mfcwindows {

		// ConvertEncoding 对话框

		class ConvertEncoding : public CDialogEx {
			DECLARE_DYNAMIC(ConvertEncoding)

		public:
			ConvertEncoding(utils::ParamPackage* _pkg, CWnd* pParent = nullptr);   // 标准构造函数
			virtual ~ConvertEncoding();

			// 对话框数据
#ifdef AFX_DESIGN_TIME
			enum { IDD = IDD_DIALOG3 };
#endif

		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

			DECLARE_MESSAGE_MAP()

		public:
			virtual BOOL OnInitDialog();

			CComboBox m_Method;
			CComboBox m_OldCP;
			CEdit m_ManualOldCP;
			CComboBox m_NewCP;
			CEdit m_ManualNewCP;

			afx_msg void On_CPWebsite();
			afx_msg void On_OldCPChanged();
			afx_msg void On_NewCPChanged();
			afx_msg void On_Dialog_OK();
			afx_msg void On_Dialog_Cancel();

			UINT OUT_OldCP;
			UINT OUT_NewCP;
			int OUT_RunMethod;

		private:
			utils::ParamPackage* pkg;
			UINT GetCPFromList(int index);
			BOOL ConvertEncoding::TryGetManualCP(CEdit* control, UINT* num);
		};
	}
}
