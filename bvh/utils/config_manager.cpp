#include "config_manager.h"
#include "win32_helper.h"

namespace bvh {
	namespace utils {

		ConfigManager::ConfigManager() :
			CurrentConfig(),
			config_version(14) {
			;
		}
		ConfigManager::~ConfigManager() {
			//destroy config vector's string
			// todo: add vector in there

		}

		void ConfigManager::GetConfigFilePath(std::filesystem::path* path) {
			win32_helper::GetVirtoolsFolder(path);
			*path /= L"BallanceVirtoolsHelper.cfg";
		}
		void ConfigManager::InitConfig() {
			CurrentConfig.func_mapping_bm_ExternalTextureFolder = L"";
			CurrentConfig.func_mapping_bm_NoComponentGroupName = "";
			CurrentConfig.func_mapping_bm_OmittedMaterialPrefix = "";

			CurrentConfig.window_mapping_bmExport_mode = 2;
			CurrentConfig.window_mapping_bmExport_filename = L"";

			CurrentConfig.window_ConvertEncoding_Method = 0;
			CurrentConfig.window_ConvertEncoding_OldCP = 0;
			CurrentConfig.window_ConvertEncoding_ManualOldCP = "";
			CurrentConfig.window_ConvertEncoding_NewCP = 0;
			CurrentConfig.window_ConvertEncoding_ManualNewCP = "";

			//todo: add setting default value in there
		}
		void ConfigManager::SaveConfig() {
			std::filesystem::path filepath;
			GetConfigFilePath(&filepath);

			FILE* f = _wfopen(filepath.wstring().c_str(), L"wb");

			//write version first
			WriteInt(f, &config_version);

			//write data
			WriteWstring(f, &CurrentConfig.func_mapping_bm_ExternalTextureFolder);
			WriteString(f, &CurrentConfig.func_mapping_bm_NoComponentGroupName);
			WriteString(f, &CurrentConfig.func_mapping_bm_OmittedMaterialPrefix);

			WriteInt(f, &CurrentConfig.window_mapping_bmExport_mode);
			WriteWstring(f, &CurrentConfig.window_mapping_bmExport_filename);

			WriteInt(f, &CurrentConfig.window_ConvertEncoding_Method);
			WriteInt(f, &CurrentConfig.window_ConvertEncoding_OldCP);
			WriteString(f, &CurrentConfig.window_ConvertEncoding_ManualOldCP);
			WriteInt(f, &CurrentConfig.window_ConvertEncoding_NewCP);
			WriteString(f, &CurrentConfig.window_ConvertEncoding_ManualNewCP);

			//todo: add setting write in there

			fclose(f);
		}
		void ConfigManager::LoadConfig() {
			std::filesystem::path filepath;
			GetConfigFilePath(&filepath);

			FILE* f = _wfopen(filepath.wstring().c_str(), L"rb");
			if (f == NULL) goto needInitCfg;

			//checker
			int version_checker = 0;
			ReadInt(f, &version_checker);
			if (version_checker != config_version) {
				// not suit for current version, generate new config
				fclose(f);
				goto needInitCfg;
			}

			//read data
			ReadWstring(f, &CurrentConfig.func_mapping_bm_ExternalTextureFolder);
			ReadString(f, &CurrentConfig.func_mapping_bm_NoComponentGroupName);
			ReadString(f, &CurrentConfig.func_mapping_bm_OmittedMaterialPrefix);

			ReadInt(f, &CurrentConfig.window_mapping_bmExport_mode);
			ReadWstring(f, &CurrentConfig.window_mapping_bmExport_filename);

			ReadInt(f, &CurrentConfig.window_ConvertEncoding_Method);
			ReadInt(f, &CurrentConfig.window_ConvertEncoding_OldCP);
			ReadString(f, &CurrentConfig.window_ConvertEncoding_ManualOldCP);
			ReadInt(f, &CurrentConfig.window_ConvertEncoding_NewCP);
			ReadString(f, &CurrentConfig.window_ConvertEncoding_ManualNewCP);

			//todo: add setting read in there

			fclose(f);
			return;

		needInitCfg:
			InitConfig();
			SaveConfig();
			return;
		}

#pragma region read/write func

		void ConfigManager::ReadInt(FILE* fs, int* num) {
			fread(num, sizeof(int), 1, fs);
		}
		void ConfigManager::ReadInt(FILE* fs, std::vector<int>* num) {
			int count = 0, cache = 0;
			ReadInt(fs, &count);
			for (int i = 0; i < count; ++i) {
				ReadInt(fs, &cache);
				num->push_back(cache);
			}
		}
		void ConfigManager::WriteInt(FILE* fs, int* num) {
			fwrite(num, sizeof(int), 1, fs);
		}
		void ConfigManager::WriteInt(FILE* fs, std::vector<int>* num) {
			int count = num->size(), cache = 0;
			WriteInt(fs, &count);
			for (auto iter = num->begin(); iter != num->end(); iter++) {
				cache = *iter;
				WriteInt(fs, &cache);
			}
		}
		void ConfigManager::ReadString(FILE* fs, std::string* str) {
			int count = 0;
			ReadInt(fs, &count);
			str->resize(count);
			fread(str->data(), sizeof(char), count, fs);
		}
		void ConfigManager::ReadWstring(FILE* fs, std::wstring* str) {
			int count = 0;
			ReadInt(fs, &count);
			str->resize(count);
			fread(str->data(), sizeof(wchar_t), count, fs);
		}
		//void ConfigManager::ReadString(FILE* fs, std::vector<std::string*>* str) {
		//	int count = 0;
		//	std::string* cache;
		//	ReadInt(fs, &count);
		//	for (int i = 0; i < count; ++i) {
		//		cache = new std::string();
		//		ReadString(fs, cache);
		//		str->push_back(cache);
		//	}
		//}
		void ConfigManager::WriteString(FILE* fs, std::string* str) {
			int count = str->size();
			WriteInt(fs, &count);
			fwrite(str->c_str(), sizeof(char), count, fs);
		}
		void ConfigManager::WriteWstring(FILE* fs, std::wstring* str) {
			int count = str->size();
			WriteInt(fs, &count);
			fwrite(str->c_str(), sizeof(wchar_t), count, fs);
		}
		//void ConfigManager::WriteString(FILE* fs, std::vector<std::string*>* str) {
		//	int count = str->size();
		//	WriteInt(fs, &count);
		//	for (auto iter = str->begin(); iter != str->end(); iter++) {
		//		WriteString(fs, *iter);
		//	}
		//}

#pragma endregion
	}
}