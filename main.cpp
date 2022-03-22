#include "stdafx.h"
#include "vt_menu.h"
#include <exception>
#include "bvh/utils/config_manager.h"
#include "bvh/utils/win32_helper.h"

bvh::utils::config_manager* cfg_manager = NULL;
PluginInterface* s_Plugininterface = NULL;
PluginInfo g_PluginInfo0;

int GetVirtoolsPluginInfoCount() {
	return 1;
}

PluginInfo* GetVirtoolsPluginInfo(int index) {
	switch (index) {
		case 0:
			return &g_PluginInfo0;
	}
	return NULL;
}

class SuperScriptMaterializer : CWinApp {
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

SuperScriptMaterializer theApp;

BOOL SuperScriptMaterializer::InitInstance() {
	// set locale
	//setlocale(LC_ALL, "");

	cfg_manager = new bvh::utils::config_manager();
	cfg_manager->LoadConfig();

	strcpy(g_PluginInfo0.m_Name, "BallanceVirtoolsHelper");
	g_PluginInfo0.m_PluginType = PluginInfo::PT_EDITOR;
	g_PluginInfo0.m_PluginType = (PluginInfo::PLUGIN_TYPE)(g_PluginInfo0.m_PluginType | PluginInfo::PTF_RECEIVENOTIFICATION);
	g_PluginInfo0.m_PluginCallback = PluginCallback;

	return CWinApp::InitInstance();
}

int SuperScriptMaterializer::ExitInstance() {

	delete cfg_manager;

	return CWinApp::ExitInstance();
}