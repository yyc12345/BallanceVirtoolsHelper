#pragma once

#include "stdafx.h"

void PluginCallback(PluginInfo::CALLBACK_REASON reason, PluginInterface* plugininterface);

void InitMenu();
void RemoveMenu();
void UpdateMenu();

void MenuCallback(int commandID);
