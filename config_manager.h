#if !defined(_YYCDLL_CONFIG_MANAGER_H__IMPORTED_)
#define _YYCDLL_CONFIG_MANAGER_H__IMPORTED_

#include <string>
#include <vector>
#include <filesystem>

struct config_body {
	//func_mapping_bm
	std::string func_mapping_bm_ExternalTextureFolder;
	std::string func_mapping_bm_NoComponentGroupName;
	//window_mapping_bm_export
	int window_mapping_bmExport_mode;	//0 is object, 1 is group, 2 is all
	std::string window_mapping_bmExport_filename;

	//todo: add setting in there
};

class config_manager {
	public:
	config_manager();
	~config_manager();

	void SaveConfig();
	void LoadConfig();
	config_body CurrentConfig;

	private:
	void GetConfigFilePath(std::filesystem::path* path);
	void InitConfig();
	int config_version;

	void ReadInt(FILE* fs, int* num);
	void ReadInt(FILE* fs, std::vector<int>* num);
	void WriteInt(FILE* fs, int* num);
	void WriteInt(FILE* fs, std::vector<int>* num);
	void ReadString(FILE* fs, std::string* str);
	void ReadString(FILE* fs, std::vector<std::string*>* str);
	void WriteString(FILE* fs, std::string* str);
	void WriteString(FILE* fs, std::vector<std::string*>* str);
};

#endif