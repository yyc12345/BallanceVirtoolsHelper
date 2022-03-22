#include "vt_menu.h"
#include "bvh/utils/win32_helper.h"

//=============func import

#include "bvh/features/script//ck_data_array.h"
#include "bvh/features/misc/special_nmo.h"
#include "bvh/features/mapping/bmfile.h"
#include "bvh/features/mapping/grouping.h"
#include "bvh/features/mapping/shadow.h"
#include "bvh/features/misc/convert_encoding.h"

#include "bvh/mfcwindows/bvh_config.h"

//=============func import

extern PluginInterface* s_Plugininterface;
extern bvh::utils::config_manager* cfg_manager;

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
	s_Plugininterface->AddPluginMenuItem(sub_bm, 7, "Fix texture");

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

	// define error_proc
	// and param package
	bvh::utils::ErrorProc error_proc;
	bvh::utils::ParamPackage pkg(s_Plugininterface, &error_proc, cfg_manager);

	switch (commandID) {
		case 2:
			bvh::features::mapping::bmfile::ImportBM(&pkg);
			//we need a force refresh and mark change
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;
		case 3:
			bvh::features::mapping::bmfile::ExportBM(&pkg);
			break;
		case 7:
			bvh::features::mapping::bmfile::FixTexture(&pkg);
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;
		case 5:
			bvh::features::mapping::grouping::AutoGrouping(&pkg);
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;
		case 8:
			bvh::features::mapping::shadow::AddShadow(&pkg);
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;

		case 27:
			bvh::features::script::ck_data_array::ExportCsv(&pkg);
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;
		case 28:
			bvh::features::script::ck_data_array::ImportCsv(&pkg);
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;
		case 29:
			bvh::features::script::ck_data_array::Clean(&pkg);
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;

		case 4:
		{
			bvh::mfcwindows::BVHConfig* window_misc_config = new bvh::mfcwindows::BVHConfig(&pkg);
			window_misc_config->DoModal();
			delete window_misc_config;
		}break;
		case 32:
			bvh::features::misc::special_nmo::SaveSpecialNMO(&pkg);
			break;
		case 33:
			bvh::features::misc::convert_encoding::DoConvertEncoding(&pkg);
			s_Plugininterface->DoOneRenderNow();
			s_Plugininterface->SetProjectModified(TRUE);
			break;
		case 30:
			ShellExecute(NULL, "open", "https://github.com/yyc12345/BallanceVirtoolsHelper/issues", NULL, NULL, SW_SHOWNORMAL);
			break;
		case 31:
			AfxMessageBox("BallanceVirtoolsHelper - The plugin which can help Ballance mapping and script.\nBM file spec version: 1.3(13)\nConfig file version: 13\nPlugin version: 2.0\nUnder GPL v3 License.\nProject homepage: https://github.com/yyc12345/BallanceVirtoolsHelper", MB_ICONINFORMATION + MB_OK);
			break;
	}

	error_proc.DisplayMessage(s_Plugininterface->GetCKContext());
}
