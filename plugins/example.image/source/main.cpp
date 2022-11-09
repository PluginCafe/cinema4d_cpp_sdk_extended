/*
  Image API Example Plugin
  (C) MAXON Computer GmbH, 2022

  Author: Ferdinand Hoppe
  Date: 12/08/2022

  Provides examples for the image.framework.
*/
#include "c4d_plugin.h"
#include "c4d_resource.h"

// Forward declaration for image_api_examples_plugin.h
Bool RegisterImageApiExamples();

Bool PluginStart()
{
  if (!RegisterImageApiExamples())
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
