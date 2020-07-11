#include "script_CKDataArray.h"
#include "../func_helper.h"

namespace func_namespace {
	namespace script {
		namespace CKDataArray {

			BOOL ImportCsv(PluginInterface* plgif) {
				::CKDataArray* target = getEditingDataArray(plgif);
				if (target == NULL) {
					strcpy(func_namespace::ExecutionResult, "No selected CKDataArray.");
					return FALSE;
				}
				std::string file;
				func_namespace::OpenFileDialog(&file, "Csv file(*.csv)\0*.csv\0", "csv", TRUE);
				if (file.empty()) {
					strcpy(func_namespace::ExecutionResult, "No selected CSV file.");
					return FALSE;
				}

				strcpy(func_namespace::ExecutionCache, file.c_str());
				BOOL cache = target->LoadElements(func_namespace::ExecutionCache, TRUE, 0);
				if (!cache) {
					strcpy(func_namespace::ExecutionResult, "Error when import file.");
					return FALSE;
				}

				return TRUE;
			}
			BOOL ExportCsv(PluginInterface* plgif) {
				::CKDataArray* target = getEditingDataArray(plgif);
				if (target == NULL) {
					strcpy(func_namespace::ExecutionResult, "No selected CKDataArray.");
					return FALSE;
				}
				std::string file;
				func_namespace::OpenFileDialog(&file, "Csv file(*.csv)\0*.csv\0", "csv", FALSE);
				if (file.empty()) {
					strcpy(func_namespace::ExecutionResult, "No selected CSV file.");
					return FALSE;
				}

				strcpy(func_namespace::ExecutionCache, file.c_str());
				BOOL cache = target->WriteElements(func_namespace::ExecutionCache, 0, target->GetColumnCount(), FALSE);
				if (!cache) {
					strcpy(func_namespace::ExecutionResult, "Error when export file.");
					return FALSE;
				}

				return TRUE;
			}

			BOOL Clean(PluginInterface* plgif) {
				::CKDataArray* target = getEditingDataArray(plgif);
				if (target == NULL) {
					strcpy(func_namespace::ExecutionResult, "No selected CKDataArray.");
					return FALSE;
				}

				target->Clear();
				return TRUE;
			}


			::CKDataArray* getEditingDataArray(PluginInterface* plgif) {
				auto ctx = plgif->GetCKContext();
				CKControl::MultiParamEditDlg pickdialog(ctx);
				::CKDataArray* res = NULL;

				pickdialog.SetTitle("Pick CKDataArray");
				auto ifparam = plgif->GetInterfaceParameter();
				ifparam->SetGUID(CKPGUID_DATAARRAY);
				pickdialog.AddParameter(ifparam);
				if (pickdialog.DoModal() != IDOK) goto destroyCKDataArrayWindow;
				else
					res = (::CKDataArray*)ifparam->GetValueObject();


			destroyCKDataArrayWindow:
				plgif->FreeInterfaceParameter(ifparam->GetID());
				return res;
			}

		}
	}
}
