// classic API header files
#include "c4d_plugin.h"
#include "c4d_resource.h"

// project header files
#include "business_logic.h"
#include "user_interface.h"

// If the classic API is used PluginStart(), PluginMessage() and PluginEnd() must be implemented.

::Bool PluginStart()
{
	// register classic API plugins
	microsdk::RegisterCubeCommand();

	return true;
}

void PluginEnd()
{
	// free resources
}

::Bool PluginMessage(::Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
		{
			// load resources defined in the the optional "res" folder
			if (!g_resource.Init())
				return false;

			return true;
		}
		case C4DPL_PROGRAM_STARTED:
		{
			// perform some action after the program has started
			microsdk::ExecuteMicroExampleCode();
			break;
		}
	}

	return true;
}
