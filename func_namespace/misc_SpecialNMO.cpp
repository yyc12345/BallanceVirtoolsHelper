#include "misc_SpecialNMO.h"
#include "../func_helper.h"

namespace func_namespace {
	namespace misc {
		namespace SpecialNMO {
			BOOL SaveSpecialNMO(PluginInterface* plgif) {
				std::filesystem::path file;
				getNmoFile(&file);
				if (file.empty()) {
					strcpy(func_namespace::ExecutionResult, "No selected NMO file.");
					return FALSE;
				}

				// try re-construct a usable CKObjectArray
				auto ctx = plgif->GetCKContext();
				CKObjectArray* finalArray = CreateCKObjectArray();
				XObjectPointerArray tmpArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
				int count = tmpArray.Size();
				for (unsigned int i = 0; i < count; i++)
					finalArray->AddIfNotHere(tmpArray.GetObjectA(i));

				if (ctx->GetCurrentLevel() == NULL) {
					//no level, add it and add all object into it.
					CKLevel* levels = (CKLevel*)ctx->CreateObject(CKCID_LEVEL);
					for (unsigned int i = 0; i < count; i++)
						levels->AddObject(tmpArray.GetObjectA(i));
				} else {
					//have level, just remove it
					finalArray->Remove(ctx->GetCurrentLevel());
					finalArray->Remove(ctx->GetCurrentScene());
				}

				//clean useless file
				strcpy(func_namespace::ExecutionCache, file.string().c_str());
				DeleteFile(func_namespace::ExecutionCache);
				file.replace_extension("cmo");
				strcpy(func_namespace::ExecutionCache2, file.string().c_str());
				DeleteFile(func_namespace::ExecutionCache2);
				//save it
				CKDependencies dep;
				dep.m_Flags = CK_DEPENDENCIES_FULL;
				CKERROR code = ctx->Save(func_namespace::ExecutionCache2, finalArray, 0xFFFFFFFF, &dep);
				DeleteCKObjectArray(finalArray);
				if (code != CK_OK) {
					sprintf(func_namespace::ExecutionResult, "Error when saving file. Error code: %d", code);
					return FALSE;
				}
				//rename
				auto code2 = MoveFile(func_namespace::ExecutionCache2, func_namespace::ExecutionCache);
				if (code != CK_OK) {
					strcpy(func_namespace::ExecutionResult, "Error when changing extension.");
					return FALSE;
				}

				return TRUE;
			}

			void getNmoFile(std::filesystem::path* str) {
				str->clear();

				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.lpstrFile = func_namespace::ExecutionCache;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = CACHE_SIZE;
				ofn.lpstrFilter = "NMO file(*.nmo)\0*.nmo\0";
				ofn.lpstrDefExt = "nmo";
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_EXPLORER;
				if (GetSaveFileName(&ofn))
					*str = func_namespace::ExecutionCache;

			}

		}
	}
}