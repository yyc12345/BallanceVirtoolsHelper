#include "ck_data_array.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"

namespace bvh {
	namespace features {
		namespace script {
			namespace ck_data_array {

				void ImportCsv(utils::ParamPackage* pkg) {
					::CKDataArray* target = getEditingDataArray(pkg->plgif);
					if (target == NULL) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected CKDataArray.");
						return;
					}
					std::string file;
					if (!utils::win32_helper::OpenFileDialog(&file, "Csv file(*.csv)\0*.csv\0", "csv", TRUE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected CSV file.");
						return;
					}

					BOOL cache = target->LoadElements((CKSTRING)file.c_str(), TRUE, 0);
					if (!cache) {
						pkg->error_proc->SetExecutionResult(FALSE, "Error when import file.");
						return;
					}

					pkg->error_proc->SetExecutionResult(TRUE);
				}
				void ExportCsv(utils::ParamPackage* pkg) {
					::CKDataArray* target = getEditingDataArray(pkg->plgif);
					if (target == NULL) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected CKDataArray.");
						return;
					}
					std::string file;
					if (!utils::win32_helper::OpenFileDialog(&file, "Csv file(*.csv)\0*.csv\0", "csv", FALSE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected CSV file.");
						return;
					}

					BOOL cache = target->WriteElements((CKSTRING)file.c_str(), 0, target->GetColumnCount(), FALSE);
					if (!cache) {
						pkg->error_proc->SetExecutionResult(FALSE, "Error when export file.");
						return;
					}

					pkg->error_proc->SetExecutionResult(TRUE);
				}

				void Clean(utils::ParamPackage* pkg) {
					::CKDataArray* target = getEditingDataArray(pkg->plgif);
					if (target == NULL) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected CKDataArray.");
						return;
					}

					target->Clear();
					pkg->error_proc->SetExecutionResult(TRUE);
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
}
