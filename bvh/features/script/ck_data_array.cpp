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
					std::wstring file;
					if (!utils::win32_helper::OpenFileDialog(&file, L"Csv file(*.csv)\0*.csv\0", L"csv", TRUE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected CSV file.");
						return;
					}

					std::string ansi_file;
					if (!utils::string_helper::Wstring2String(&file, &ansi_file, CP_ACP) || 
						!utils::win32_helper::CheckANSIPathValidation(&ansi_file)) {
						pkg->error_proc->SetExecutionResult(FALSE, "Selected CSV file path contain illegal character for current code page.");
						return;
					}
					BOOL cache = target->LoadElements((CKSTRING)ansi_file.c_str(), TRUE, 0);
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
					std::wstring file;
					if (!utils::win32_helper::OpenFileDialog(&file, L"Csv file(*.csv)\0*.csv\0", L"csv", FALSE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected CSV file.");
						return;
					}

					std::string ansi_file;
					if (!utils::string_helper::Wstring2String(&file, &ansi_file, CP_ACP) ||
						!utils::win32_helper::CheckANSIPathValidation(&ansi_file)) {
						pkg->error_proc->SetExecutionResult(FALSE, "Selected CSV file path contain illegal character for current code page.");
						return;
					}
					BOOL cache = target->WriteElements((CKSTRING)ansi_file.c_str(), 0, target->GetColumnCount(), FALSE);
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
