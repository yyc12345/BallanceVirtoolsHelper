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
				getCsvFile(&file, TRUE);
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
				getCsvFile(&file, FALSE);
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
			void getCsvFile(std::string* str, BOOL isOpen) {
				str->clear();

				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.lpstrFile = func_namespace::ExecutionCache;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = CACHE_SIZE;
				ofn.lpstrFilter = "Csv file(*.csv)\0*.csv\0";
				ofn.lpstrDefExt = "csv";
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_EXPLORER;
				if (isOpen) {
					if (GetOpenFileName(&ofn))
						*str = func_namespace::ExecutionCache;
				} else {
					if (GetSaveFileName(&ofn))
						*str = func_namespace::ExecutionCache;
				}


			}

		}
	}
}
