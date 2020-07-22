#include "misc_SpecialNMO.h"
#include "../func_helper.h"

namespace func_namespace {
	namespace misc {
		namespace SpecialNMO {
			BOOL SaveSpecialNMO(PluginInterface* plgif) {
				std::filesystem::path file, tempfile;
				std::string filepath;
				if (!func_namespace::OpenFileDialog(&filepath, "NMO file(*.nmo)\0*.nmo\0", "nmo", FALSE)) {
					strcpy(func_namespace::ExecutionResult, "No selected NMO file.");
					return FALSE;
				}
				file = filepath;
				
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

				//get temp file
				func_namespace::GetTempFolder(&tempfile);
				sprintf(func_namespace::ExecutionCache, "7c20254bdc9a4409b490e46efaf5e128_%d.cmo", GetCurrentProcessId()); //7c20254bdc9a4409b490e46efaf5e128 is guid
				tempfile /= func_namespace::ExecutionCache;
				strcpy(func_namespace::ExecutionCache, tempfile.string().c_str());
				DeleteFile(func_namespace::ExecutionCache);
				//get real file
				strcpy(func_namespace::ExecutionCache2, file.string().c_str());
				DeleteFile(func_namespace::ExecutionCache2);
				
				//save it
				CKDependencies dep;
				dep.m_Flags = CK_DEPENDENCIES_FULL;
				CKERROR code = ctx->Save(func_namespace::ExecutionCache, finalArray, 0xFFFFFFFF, &dep);
				DeleteCKObjectArray(finalArray);
				if (code != CK_OK) {
					sprintf(func_namespace::ExecutionResult, "Error when saving file. Error code: %d", code);
					return FALSE;
				}

				//move temp file
				auto code2 = MoveFile(func_namespace::ExecutionCache, func_namespace::ExecutionCache2);
				if (code != CK_OK) {
					strcpy(func_namespace::ExecutionResult, "Error when moving extension.");
					return FALSE;
				}

				return TRUE;
			}

		}
	}
}