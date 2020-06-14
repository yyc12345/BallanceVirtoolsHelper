#include "stdafx.h"
#include "vt_menu.h"
#include "func_helper.h"
#include <exception>

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
	// TODO: Add your specialized code here and/or call the base class

	if (!func_namespace::InitHelper())
		throw new std::bad_alloc();

	strcpy(g_PluginInfo0.m_Name, "BallanceVirtoolsHelper");
	g_PluginInfo0.m_PluginType = PluginInfo::PT_EDITOR;
	g_PluginInfo0.m_PluginType = (PluginInfo::PLUGIN_TYPE)(g_PluginInfo0.m_PluginType | PluginInfo::PTF_RECEIVENOTIFICATION);
	g_PluginInfo0.m_PluginCallback = PluginCallback;

	return CWinApp::InitInstance();
}

int SuperScriptMaterializer::ExitInstance() {
	// TODO: Add your specialized code here and/or call the base class

	func_namespace::DisposeHelper();

	return CWinApp::ExitInstance();
}