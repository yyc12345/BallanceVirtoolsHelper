#include "config_manager.h"
#include "func_helper.h"

config_manager::config_manager() :
	CurrentConfig(),
	config_version(10) {
	;
}
config_manager::~config_manager() {
	//destroy config vector's string
	for (auto iter = CurrentConfig.func_mapping_bm_PHReplacePair_Regex.begin(); iter != CurrentConfig.func_mapping_bm_PHReplacePair_Regex.end(); iter++)
		delete(*iter);

}

void config_manager::GetConfigFilePath(std::filesystem::path* path) {
	GetModuleFileName(NULL, func_namespace::ConfigCache, CACHE_SIZE);
	*path = func_namespace::ConfigCache;
	path->replace_filename("BallanceVirtoolsHelper.cfg");
}
void config_manager::InitConfig() {
	CurrentConfig.func_mapping_bm_NoCoponentGroupName = "";

	//todo: add standard regex for func_mapping_bm_PHReplacePair_Regex and Target
}
void config_manager::SaveConfig() {
	std::filesystem::path filepath;
	GetConfigFilePath(&filepath);

	FILE* f = fopen(filepath.string().c_str(), "wb");

	//write version first
	WriteInt(f, &config_version);

	//write data
	WriteString(f, &CurrentConfig.func_mapping_bm_NoCoponentGroupName);
	WriteString(f, &CurrentConfig.func_mapping_bm_PHReplacePair_Regex);
	WriteInt(f, &CurrentConfig.func_mapping_bm_PHReplacePair_Target);

	fclose(f);
}
void config_manager::LoadConfig() {
	std::filesystem::path filepath;
	GetConfigFilePath(&filepath);

	FILE* f = fopen(filepath.string().c_str(), "rb");
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
	ReadString(f, &CurrentConfig.func_mapping_bm_NoCoponentGroupName);
	ReadString(f, &CurrentConfig.func_mapping_bm_PHReplacePair_Regex);
	ReadInt(f, &CurrentConfig.func_mapping_bm_PHReplacePair_Target);

	fclose(f);
	return;

needInitCfg:
	InitConfig();
	SaveConfig();
	return;
}

#pragma region read/write func

void config_manager::ReadInt(FILE* fs, int* num) {
	fread(num, sizeof(int), 1, fs);
}
void config_manager::ReadInt(FILE* fs, std::vector<int>* num) {
	int count = 0, cache = 0;
	ReadInt(fs, &count);
	for (int i = 0; i < count; ++i) {
		ReadInt(fs, &cache);
		num->push_back(cache);
	}
}
void config_manager::WriteInt(FILE* fs, int* num) {
	fwrite(num, sizeof(int), 1, fs);
}
void config_manager::WriteInt(FILE* fs, std::vector<int>* num) {
	int count = num->size(), cache = 0;
	WriteInt(fs, &count);
	for (auto iter = num->begin(); iter != num->end(); iter++) {
		cache = *iter;
		WriteInt(fs, &cache);
	}
}
void config_manager::ReadString(FILE* fs, std::string* str) {
	int count = 0;
	ReadInt(fs, &count);
	fread(func_namespace::ConfigCache, sizeof(char), count, fs);
	*str = func_namespace::ConfigCache;
}
void config_manager::ReadString(FILE* fs, std::vector<std::string*>* str) {
	int count = 0;
	std::string* cache;
	ReadInt(fs, &count);
	for (int i = 0; i < count; ++i) {
		cache = new std::string();
		ReadString(fs, cache);
		str->push_back(cache);
	}
}
void config_manager::WriteString(FILE* fs, std::string* str) {
	int count = str->size();
	WriteInt(fs, &count);
	fwrite(str->c_str(), sizeof(char), count, fs);
}
void config_manager::WriteString(FILE* fs, std::vector<std::string*>* str) {
	int count = str->size();
	WriteInt(fs, &count);
	for (auto iter = str->begin(); iter != str->end(); iter++) {
		WriteString(fs, *iter);
	}
}

#pragma endregion
