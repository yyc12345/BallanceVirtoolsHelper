#include "special_nmo.h"
#include "../../utils/win32_helper.h"
#include "../../utils/string_helper.h"

namespace bvh {
	namespace features {
		namespace misc {
			namespace special_nmo {
				void SaveSpecialNMO(utils::ParamPackage* pkg) {
					std::filesystem::path file, tempfile;
					std::wstring filepath, filename;
					if (!utils::win32_helper::OpenFileDialog(&filepath, L"NMO file(*.nmo)\0*.nmo\0", L"nmo", FALSE)) {
						pkg->error_proc->SetExecutionResult(FALSE, "No selected NMO file.");
						return;
					}
					file = filepath.c_str();

					// get temp file
					// temp file are created in vt temp folder and it must can be parsed
					// otherwise vt can not work. so we do not check it validation when WString -> String
					utils::win32_helper::GetTempFolder(pkg->plgif->GetCKContext(), &tempfile);
					utils::string_helper::StdwstringPrintf(&filename, L"7c20254bdc9a4409b490e46efaf5e128_%d.cmo", GetCurrentProcessId()); //7c20254bdc9a4409b490e46efaf5e128 is guid
					tempfile /= filename;

					// try re-construct a usable CKObjectArray
					auto ctx = pkg->plgif->GetCKContext();
					CKObjectArray* finalArray = CreateCKObjectArray();
					XObjectPointerArray tmpArray = ctx->GetObjectListByType(CKCID_OBJECT, TRUE);
					uint32_t count = (uint32_t)tmpArray.Size();
					for (uint32_t i = 0; i < count; i++)
						finalArray->AddIfNotHere(tmpArray.GetObjectA(i));

					if (ctx->GetCurrentLevel() == NULL) {
						//no level, add it and add all object into it.
						CKLevel* levels = (CKLevel*)ctx->CreateObject(CKCID_LEVEL);
						for (uint32_t i = 0; i < count; i++)
							levels->AddObject(tmpArray.GetObjectA(i));
					} else {
						//have level, just remove it
						finalArray->Remove(ctx->GetCurrentLevel());
						finalArray->Remove(ctx->GetCurrentScene());
					}

					// delete cache file and real file
					DeleteFileW(tempfile.wstring().c_str());
					DeleteFileW(file.wstring().c_str());
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
					auto code2 = MoveFileW(tempfile.wstring().c_str(), file.wstring().c_str());
					if (!code2) {
						pkg->error_proc->SetExecutionResult(FALSE, "Error when moving extension.");
						return;
					}

					return;
				}

			}
		}
	}
}