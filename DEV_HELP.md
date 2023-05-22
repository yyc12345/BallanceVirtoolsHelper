# Development Help

## Bump Version

### Update Plugin Version

The main version of this plugin. This should be updated when any changes happend.

* `vt_menu.cpp`: About dialog.
* `BallanceVirtoolsHelper.rc` - `VS_VERSION_INFO`: Update fields `PRODUCTVERSION` and `FILEVERSION`. Only use the first 2 number.

### Update BM Spec Version

This version should be changed when plugin adapting the new BM specification.

* `bvh/features/mapping/bmfile.h`: Macro `BM_FILE_VERSION`
* `bvh/utils/zip_helper.cpp`: Macro `BVH_ZIP_GLOBAL_COMMENT`
* `vt_menu.cpp`: About dialog.

### Update Plugin Config Version

This version needs to be changed when the structure of configuration file changed.

* `bvh/utils/config_manager.h`: Struct `ConfigBody`. Add or delete config entry in there.
* `bvh/utils/config_manager.cpp`: Method `ConfigManager::InitConfig()`. Add or delete config default value in there.
* `bvh/utils/config_manager.cpp`: Method `ConfigManager::SaveConfig()`. Add or remove save method for the config entry added or removed.
* `bvh/utils/config_manager.cpp`: Method `ConfigManager::LoadConfig()`. Add or remove read method for the config entry added or removed.
* `bvh/utils/config_manager.cpp`: Contructor `ConfigManager::ConfigManager()`. Change `config_version`'s value in initialize list.
* `vt_menu.cpp`: About dialog.

## Pack Release

The release package should include these files.

* InterfacePlugins
  - BallanceVirtoolsHelper.dll
  - BallanceVirtoolsHelper.pdb
* BMMeshes
  - P_Ball_Paper.bin
  - P_Ball_Stone.bin
  - other bin files...
* zlibwapi.dll
