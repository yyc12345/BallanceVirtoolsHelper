# Ballance Virtools Helper

[English version](README.md)

## 简介

这是一个用于Virtools的插件，其主要是服务于Ballance制图以及其他方面的开发。

目前仅仅包含比较基本的功能，其余的更多有用的功能将在未来版本中进行开发

## 技术信息

使用的BM文件标准可以在[这里](https://github.com/yyc12345/gist/blob/master/BMFileSpec/BMSpec_ZH.md)查找

使用的制图链标准以及`meshes`文件夹下的文件的格式可以在[这里](https://github.com/yyc12345/gist/blob/master/BMFileSpec/YYCToolsChainSpec_ZH.md)查找

仅支持Virtools 3.5

## 功能介绍

### Ballance Mapping

#### BM

Import BM file：导入BM文件，需要注意的是，为了保证兼容性，导入操作会强行分离各个顶点，这种操作的结果就是顶点个数将永远是面数的三倍，且各个相接顶点会重复，增大网格体积。

Export BM file：导出BM文件，可以选择按物体，按组，以及全部导出。需要注意的是，全部导出时，一个Virtools默认的3D物体也会被导出，不过只需要删除即可。

Fix Blender texture：修复从Blender导出的BM，导入进Virtools时出现的材质问题。这是一个广义上的材质修复，不仅仅局限于Blender，根据材质引用的贴图文件来修复材质的错误。此功能仅是修复早期BM文件实现不完全时的错误，BM现在已经完善，使用最新版的导入导出工具，大多数材质无需此选项来进行修复了。

#### Group

Auto grouping：自动归组，归组原则基于制图链标准。

Grouping detector：归组检测器，未完成。

#### Add Shadow

为位于Shadow组中的所有物体加上影子属性。

### Ballance Script

#### CKDataArray

Export to csv：将指定表中的数据导出成csv格式，通过调用`WriteElements`实现

Import to csv：导入csv数据到指定表中，通过调用`LoadElements`实现

Clean all data：清空指定表的数据

### Ballance Misc

#### Export as special NMO

将当前文档保存成为具有IC的NMO格式。

#### Convert encoding

转换当前文档中的多字节文本所用的编码。用于乱码修复。

#### Plugin settings

* External texture folder：请填写为Ballance的Texture目录，插件将从此目录下调用外置贴图文件（即Ballance原本带有的贴图文件）
* No component group name：处于此组中的物体将被强制指定为非Component。如果留空则表示不需要这个功能。

## 外部库

### 头文件

* [zlib](http://www.zlib.net/)：下载1.2.11版本，其中已经包含zlib本身以及后续要使用的minizip的头文件

### 链接库

* [minizip](http://www.winimage.com/zLibDll/minizip.html)：下载`zlib123dll.zip`，获取到已经编译完成的，可直接用于连接的zlib和minizip的整合的链接库。

## 编译与调试

我不建议自行编译或调试此工程，因为一些事情会很复杂。

编译此工程需要消耗一些时间，您首先需要有Virtools 3.5 SDK，然后您需要在项目设置中将诸多绝对路径更换为您SDK的路径，例如外部引用，链接库等。

此外，需要被部署在Virtools根目录下的`BMMeshes`文件夹内的的文件与我的另一个工程共享，可以在[这里](https://github.com/yyc12345/BallanceBlenderHelper/tree/master/ballance_blender_plugin/meshes)找到。您需要手动复制它。

如果您在代码中手动修改了配置文件的布局信息（例如新增了字段），请将配置文件删除，或者提高配置文件版本号，并在进入Virtools中重新配置设置。

## 安装

手动编译工程并安装。但不建议这么做，因为会出现一些部署上的问题，以及当前最新的Commit可能包含很多bug。

推荐的方法是从GitHub页面的Release中查找最新的稳定版本，下载后直接解压到Virtools根目录即可，如果出现同名或者文件夹覆盖，点击允许即可。
