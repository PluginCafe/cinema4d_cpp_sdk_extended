//
//  main.cpp
//  blw_pluginlicensedialog
//
//  Created by Riccardo Gigante on 23/07/2019.
//  Copyright © 2019 MAXON Computer GmbH. All rights reserved.
//

#include "c4d_resource.h"

#include "main.h"

Bool PluginStart()
{
	// register the CommandData to generate the plugin license dialog
	if (!RegisterBLWPluginLicenseDialog())
		return false;
	
	return true;
}

void PluginEnd()
{
}

Bool PluginMessage(Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!g_resource.Init())
				return false;
			return true;
	}

	return false;
}
