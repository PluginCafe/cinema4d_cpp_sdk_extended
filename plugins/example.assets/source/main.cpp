/*
  Asset API Basics Example Plugin
  (C) MAXON Computer GmbH, 2021

  Author: Ferdinand Hoppe
  Date: 12/08/2021

  Provides examples for accessing Asset API databases and assets contained in them, as well as
  creating new assets and modifying their metadata.

  The examples are structured as static functions which can be found at the top of this file. The
  parts below only contain a CommandData implementation which is required to run this example, but
  not for understanding the Asset API. When being run, all examples will print output to the
  Cinema 4D console. Most of the documentation can be found in the definitions, i.e.,
  asset_api_basics.cpp.
*/
#include "c4d_plugin.h"
#include "c4d_resource.h"

// Forward declaration
Bool RegisterAssetApiBasics();
Bool RegisterDotsDataAndGui();

Bool PluginStart()
{
  if (!RegisterAssetApiBasics())
    return false;
  if (!RegisterDotsDataAndGui())
    return false;
	return true;
}

void PluginEnd()
{

}

Bool PluginMessage(::Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
		{
			// don't start plugin without resource
			if (!g_resource.Init())
				return false;
			return true;
		}
	}
	return false;
}
