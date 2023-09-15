/*
	example.2024_migration
	(C) MAXON Computer GmbH, 2023

	Author: Ferdinand Hoppe
	Date: 22/05/2023
*/
#include "c4d_plugin.h"
#include "c4d_resource.h"

#include "_migration_example_plugin.h"

Bool PluginStart()
{
	if (!RegisterBoundingBoxObject())
		return false;
	if (!RegisterChangeExamplesCommand())
		return false;
	return true;
}

void PluginEnd() { }

Bool PluginMessage(::Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
		{
			if (!g_resource.Init())
				return false;
			return true;
		}
	}
	return false;
}
