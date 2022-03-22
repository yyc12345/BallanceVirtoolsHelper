#include "special_nmo.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"

namespace bvh {
	namespace features {
		namespace misc {
			namespace special_nmo {
				void SaveSpecialNMO(utils::ParamPackage* pkg) {
					std::filesystem::path file, tempfile;
					std::string filepath, filename;
					if (!utils::win32_helper::OpenFileDialog(&filepath, "NMO file(*.nmo)\0*.nmo\0", "nmo", FALSE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected NMO file.");
						return;
					}
					file = filepath;

					// try re-construct a usable CKObjectArray
					auto ctx = pkg->plgif->GetCKContext();
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
					utils::win32_helper::GetTempFolder(&tempfile);
					utils::string_helper::StdstringPrintf(&filename, "7c20254bdc9a4409b490e46efaf5e128_%d.cmo", GetCurrentProcessId()); //7c20254bdc9a4409b490e46efaf5e128 is guid
					tempfile /= filename;
					DeleteFileA(tempfile.string().c_str());
					//get real file
					DeleteFileA(file.string().c_str());

					//save it
					CKDependencies dep;
					dep.m_Flags = CK_DEPENDENCIES_FULL;
					CKERROR code = ctx->Save((CKSTRING)tempfile.string().c_str(), finalArray, 0xFFFFFFFF, &dep);
					DeleteCKObjectArray(finalArray);
					if (code != CK_OK) {
						pkg->error_proc->SetExecutionResult(FALSE, "Error when saving file. Error code: %d", code);
						return;
					}

					//move temp file
					auto code2 = MoveFileA(tempfile.string().c_str(), file.string().c_str());
					if (code != CK_OK) {
						pkg->error_proc->SetExecutionResult(FALSE, "Error when moving extension.");
						return;
					}

					return;
				}

			}
		}
	}
}