#include "vt_menu.h"
#include "func_helper.h"

//=============func import

#include "func_namespace/script_CKDataArray.h"
#include "func_namespace/misc_SpecialNMO.h"
#include "func_namespace/mapping_BM.h"
#include "func_namespace/mapping_Group.h"
#include "func_namespace/mapping_Shadow.h"
#include "func_namespace/misc_ConvertEncoding.h"

#include "func_window/window_Config.h"

//=============func import

extern PluginInterface* s_Plugininterface;

CMenu* s_MappingMenu = NULL;
CMenu* s_ScriptMenu = NULL;
CMenu* s_MiscMenu = NULL;


void PluginCallback(PluginInfo::CALLBACK_REASON reason, PluginInterface* plugininterface) {
	switch (reason) {
		case PluginInfo::CR_LOAD:
		{
			s_Plugininterface = plugininterface;
			InitMenu();
			UpdateMenu();
		}break;
		case PluginInfo::CR_UNLOAD:
		{
			RemoveMenu();
			s_Plugininterface = NULL;
		}break;
		case PluginInfo::CR_NEWCOMPOSITIONNAME:
		{
		}break;
		case PluginInfo::CR_NOTIFICATION:
		{
		}break;
	}
}

void InitMenu() {
	if (!s_Plugininterface)
		return;

	s_MappingMenu = s_Plugininterface->AddPluginMenu("Ballance Mapping", 100, NULL, (VoidFunc1Param)MenuCallback);
	s_ScriptMenu = s_Plugininterface->AddPluginMenu("Ballance Script", 100, NULL, (VoidFunc1Param)MenuCallback);
	s_MiscMenu = s_Plugininterface->AddPluginMenu("Ballance Misc", 100, NULL, (VoidFunc1Param)MenuCallback);
}

void RemoveMenu() {
	if (!s_Plugininterface || !s_MappingMenu || !s_ScriptMenu || !s_MiscMenu)
		return;

	s_Plugininterface->RemovePluginMenu(s_MappingMenu);
	s_Plugininterface->RemovePluginMenu(s_ScriptMenu);
	s_Plugininterface->RemovePluginMenu(s_MiscMenu);
}

void UpdateMenu() { //current max id: 32
	// mapping
	s_Plugininterface->ClearPluginMenu(s_MappingMenu);

	CMenu* sub_bm = s_Plugininterface->AddPluginMenuItem(s_MappingMenu, 0, "BM", FALSE, TRUE);
	//CMenu* sub_3dentity = s_Plugininterface->AddPluginMenuItem(s_MappingMenu, 0, "3DEntity", FALSE, TRUE);
	CMenu* sub_group = s_Plugininterface->AddPluginMenuItem(s_MappingMenu, 1, "Group", FALSE, TRUE);
	s_Plugininterface->AddPluginMenuItem(s_MappingMenu, 8, "Add Shadow");
	//CMenu* sub_light = s_Plugininterface->AddPluginMenuItem(s_MappingMenu, 2, "Light", FALSE, TRUE);
	//CMenu* sub_mesh = s_Plugininterface->AddPluginMenuItem(s_MappingMenu, 3, "Mesh", FALSE, TRUE);
	//CMenu* sub_name = s_Plugininterface->AddPluginMenuItem(s_MappingMenu, 4, "Name", FALSE, TRUE);

	s_Plugininterface->AddPluginMenuItem(sub_bm, 2, "Import BM file");
	s_Plugininterface->AddPluginMenuItem(sub_bm, 3, "Export BM file");
	s_Plugininterface->AddPluginMenuItem(sub_bm, -1, NULL, TRUE);
	s_Plugininterface->AddPluginMenuItem(sub_bm, 7, "Fix Blender texture");

	//s_Plugininterface->AddPluginMenuItem(sub_3dentity, 5, "Align");
	//s_Plugininterface->AddPluginMenuItem(sub_3dentity, 6, "Center distribute");
	//s_Plugininterface->AddPluginMenuItem(sub_3dentity, 7, "Exchange position");
	//s_Plugininterface->AddPluginMenuItem(sub_3dentity, 8, "Fast splice");
	//s_Plugininterface->AddPluginMenuItem(sub_3dentity, 9, "Gather");
	//s_Plugininterface->AddPluginMenuItem(sub_3dentity, 10, "Splice");
	//s_Plugininterface->AddPluginMenuItem(sub_3dentity, 11, "Accurately move");

	s_Plugininterface->AddPluginMenuItem(sub_group, 5, "Auto grouping");
	s_Plugininterface->AddPluginMenuItem(sub_group, 6, "Grouping checker");

	//s_Plugininterface->AddPluginMenuItem(sub_light, 16, "Add test light");
	//s_Plugininterface->AddPluginMenuItem(sub_light, 17, "Remove test light");

	//s_Plugininterface->AddPluginMenuItem(sub_mesh, 18, "Mesh clone");
	//s_Plugininterface->AddPluginMenuItem(sub_mesh, 19, "Mesh exchange");
	//s_Plugininterface->AddPluginMenuItem(sub_mesh, 20, "Shadow reset");
	//s_Plugininterface->AddPluginMenuItem(sub_mesh, 21, "Clean shared mesh");

	//s_Plugininterface->AddPluginMenuItem(sub_name, 22, "Auto rename");

	s_Plugininterface->UpdatePluginMenu(s_MappingMenu);

	// script
	s_Plugininterface->ClearPluginMenu(s_ScriptMenu);

	CMenu* sub_dataarray = s_Plugininterface->AddPluginMenuItem(s_ScriptMenu, 26, "CKDataArray", FALSE, TRUE);

	s_Plugininterface->AddPluginMenuItem(sub_dataarray, 27, "Export to csv");
	s_Plugininterface->AddPluginMenuItem(sub_dataarray, 28, "Import from csv");
	s_Plugininterface->AddPluginMenuItem(sub_dataarray, -1, NULL, TRUE);
	s_Plugininterface->AddPluginMenuItem(sub_dataarray, 29, "Clean all data");

	s_Plugininterface->UpdatePluginMenu(s_ScriptMenu);

	// misc
	s_Plugininterface->ClearPluginMenu(s_MiscMenu);

	s_Plugininterface->AddPluginMenuItem(s_MiscMenu, 32, "Export as special NMO");
	s_Plugininterface->AddPluginMenuItem(s_MiscMenu, 33, "Convert encoding");
	s_Plugininterface->AddPluginMenuItem(s_MiscMenu, -1, NULL, TRUE);
	s_Plugininterface->AddPluginMenuItem(s_MiscMenu, 4, "Plugin settings");
	s_Plugininterface->AddPluginMenuItem(s_MiscMenu, -1, NULL, TRUE);
	//s_Plugininterface->AddPluginMenuItem(s_MiscMenu, 99, "Test function");
	s_Plugininterface->AddPluginMenuItem(s_MiscMenu, 30, "Report bug");
	s_Plugininterface->AddPluginMenuItem(s_MiscMenu, 31, "About BallanceVirtoolsHelper");

	s_Plugininterface->UpdatePluginMenu(s_MiscMenu);
}

void MenuCallback(int commandID) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	BOOL runResult = TRUE;

	// window define
	func_window::window_Config* window_misc_config = NULL;

	switch (commandID) {
		case 2:
			runResult = func_namespace::mapping::BM::ImportBM();
			break;
		case 3:
			runResult = func_namespace::mapping::BM::ExportBM();
			break;
		case 7:
			runResult = func_namespace::mapping::BM::FixBlenderTexture();
			break;
		case 5:
			runResult = func_namespace::mapping::Group::AutoGrouping();
			break;
		case 8:
			runResult = func_namespace::mapping::Shadow::AddShadow();
			break;

		case 27:
			runResult = func_namespace::script::CKDataArray::ExportCsv(s_Plugininterface);
			break;
		case 28:
			runResult = func_namespace::script::CKDataArray::ImportCsv(s_Plugininterface);
			break;
		case 29:
			runResult = func_namespace::script::CKDataArray::Clean(s_Plugininterface);
			break;

		case 4:
			runResult = TRUE;
			window_misc_config = new func_window::window_Config();
			window_misc_config->DoModal();
			delete window_misc_config;
			break;
		case 32:
			runResult = func_namespace::misc::SpecialNMO::SaveSpecialNMO(s_Plugininterface);
			break;
		case 33:
			runResult = func_namespace::misc::ConvertEncoding::DoConvertEncoding();
			break;
		case 30:
			ShellExecute(NULL, "open", "https://github.com/yyc12345/BallanceVirtoolsHelper/issues", NULL, NULL, SW_SHOWNORMAL);
			break;
		case 31:
			AfxMessageBox("BallanceVirtoolsHelper - The plugin which can help Ballance mapping and script.\nBM file spec version: 1.3(13)\nConfig file version: 12\nPlugin version: 1.1\nUnder GPL v3 License.\nProject homepage: https://github.com/yyc12345/BallanceVirtoolsHelper", MB_ICONINFORMATION + MB_OK);
			break;
	}

	//we need a force refresh and mark change
	s_Plugininterface->DoOneRenderNow();
	s_Plugininterface->SetProjectModified(TRUE);

	func_namespace::DisplayLastMessage(runResult, s_Plugininterface->GetCKContext());
}
