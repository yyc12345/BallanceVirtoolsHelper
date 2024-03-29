#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace bvh {
	namespace utils {

		struct ConfigBody {
			//func_mapping_bm
			std::wstring func_mapping_bm_ExternalTextureFolder;
			std::string func_mapping_bm_NoComponentGroupName;
			std::string func_mapping_bm_OmittedMaterialPrefix;
			//window_mapping_bm_export
			int window_mapping_bmExport_mode;	//0 is object, 1 is group, 2 is all
			std::wstring window_mapping_bmExport_filename;
			//window_convertEncoding
			int window_ConvertEncoding_Method;
			int window_ConvertEncoding_OldCP;
			std::string window_ConvertEncoding_ManualOldCP;
			int window_ConvertEncoding_NewCP;
			std::string window_ConvertEncoding_ManualNewCP;
			//window_mapping_bm_import
			int window_mapping_bmImport_renameObj;
			int window_mapping_bmImport_renameMesh;
			int window_mapping_bmImport_renameMat;
			int window_mapping_bmImport_renameTex;

			//todo: add setting in there
		};

		class ConfigManager {
		public:
			ConfigManager();
			~ConfigManager();

			void SaveConfig();
			void LoadConfig();
			ConfigBody CurrentConfig;

		private:
			void GetConfigFilePath(std::filesystem::path* path);
			void InitConfig();
			int config_version;

			void ReadInt(FILE* fs, int* num);
			void ReadInt(FILE* fs, std::vector<int>* num);
			void WriteInt(FILE* fs, int* num);
			void WriteInt(FILE* fs, std::vector<int>* num);
			void ReadString(FILE* fs, std::string* str);
			void ReadWstring(FILE* fs, std::wstring* str);
			//void ReadString(FILE* fs, std::vector<std::string*>* str);
			void WriteString(FILE* fs, std::string* str);
			void WriteWstring(FILE* fs, std::wstring* str);
			//void WriteString(FILE* fs, std::vector<std::string*>* str);
		};
	}
}

