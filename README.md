# Ballance Virtools Helper

[中文版本](README_ZH.md)

## Brief introduction

This is a plugin for Virtools, which mainly serves Ballance mapping and other Ballance development.

Currently only contains the basic functions, the rest more useful functions will be developed in future versions.

## Technical information

The BM file standard used can be found in [here](https://github.com/yyc12345/gist/blob/master/BMFileSpec/BMSpec_ZH.md)(Chinese only).

The tools chain standard used and the format of the files in the `meshes` folder can be found in [here](https://github.com/yyc12345/gist/blob/master/BMFileSpec/YYCToolsChainSpec_ZH.md)(Chinese only).

Only supports Virtools 3.5

## Functions introduction

### Ballance Mapping

#### BM

Import BM file: Import BM file. It should be noted that in order to ensure compatibility, the import operation will forcibly separate each vertices. The result of this operation is that the number of vertices will always be three times the number of faces, and each connected vertices will be Repeat, so the mesh size will increase.

Export BM file: Export BM file, you can choose to export by object, group, and all. It should be noted that when exporting all, a Virtools default 3D object will also be exported, but it only needs to be deleted when importing again.

Fix Blender texture: Fix the texture problem of BM exported from Blender and imported into Virtools. This is a texture restoration in a broad sense, not just limited to Blender. This menu will repair material errors according to the texture file referenced by the material. This function is only to repair the error caused by the incomplete implementation of the early BM file import and export function. The BM import and export function is now complete. Using the latest version of the import and export tool, most materials do not need this option to repair.

#### Group

Auto grouping: automatic grouping, the grouping principle is based on the tools chain standard.

Grouping detector: Grouping detector, incomplete.

#### Add Shadow

Add shadow attributes to all objects in the Shadow group.

### Ballance Script

#### CKDataArray

Export to csv: Export the data in the specified table into csv format, by calling `WriteElements`.

Import to csv: Import csv data into the specified table, by calling `LoadElements`.

Clean all data: clear the data of the specified table.

### Ballance Misc

#### Export as special NMO

Save the current document in NMO format with IC.

#### Convert encoding

Convert the encoding which used in current document's multi-byte text. Used for garbled repair.

#### Plugin settings

* External texture folder: Please fill in the Texture directory of Ballance, the plug-in will call the external texture file from this directory (the texture file originally with Ballance).
* No component group name: Objects in this group will be forced to be set as non-Component. If left blank, this function is not needed.

## External library

### Header

* [zlib](http://www.zlib.net/): Download version 1.2.11, which already contains zlib itself and the minizip header file to be used later.

### Link library

* [minizip](http://www.winimage.com/zLibDll/minizip.html): Download `zlib123dll.zip`, and get the compiled link library that can be directly used and have integration of zlib and minizip.

## Compile and debug

It is not recommended to compile or debug this project by yourself, because some things will be very complicated.

It takes some time to compile this project. You first need the Virtools 3.5 SDK, and then you need to change many absolute paths to your SDK path in the project settings, such as external references, link libraries, etc.

In addition, the files that need to be deployed in the `BMMeshes` folder under the Virtools root directory are shared with another project of mine, which can be found in [here](https://github.com/yyc12345/BallanceBlenderHelper/tree/master/ballance_blender_plugin/meshes) found. You need to copy it manually.

If you manually modify the layout information of the configuration file in the code (for example, adding new fields), please delete the configuration file, or increase the version number of the configuration file, and reconfigure the settings in Virtools.

## Install

Manually compile the project and install it. But this is not recommended, because there will be some deployment problems, and the current latest commit may contain many bugs.

The recommended method is to find the latest stable version from the Release on the GitHub page and unzip it directly to the root directory of Virtools after downloading. If the same name or folder coverage appears, click Allow.
