/*
	example.2024_migration
	(C) MAXON Computer GmbH, 2023

	Author: Ferdinand Hoppe
	Date: 22/05/2023

	Contains boilerplate code to register and run the 2024 migration examples.

	Does not have to be read or understood when seeking information about 2024 API changes.
*/

#include "c4d_commandplugin.h"
#include "c4d_general.h"
#include "c4d_symbols.h"

#include "maxon/apibase.h"

#include "_migration_example_plugin.h"
#include "change_examples.h"

Bool ChangeExamplesCommand::Execute(BaseDocument* doc, GeDialog* parentManager)
{
	iferr_scope_handler
	{
		ApplicationOutput("Error: @", err);
		return false;
	};

	InstantiateDescID(doc) iferr_return;
	AccessNodeDataContainer(doc) iferr_return;
	AccessNodeBranches(doc) iferr_return;
	AvoidingDictionaries(doc) iferr_return;
	CustomDataTypeAccess(doc) iferr_return;
	GradientSampling(doc) iferr_return;
	FieldSampling(doc) iferr_return;
	CopyOnWriteSceneData(doc) iferr_return;
	CastingStyles(doc) iferr_return;

	return true;
}

Bool RegisterChangeExamplesCommand()
{
	if (!RegisterCommandPlugin(
		PID_CMD_CHANGEEXAMPLES, 
		"Run Change Examples"_s, 
		PLUGINFLAG_SMALLNODE, nullptr,
		"Runs all the minor 2024 API change examples"_s, 
		ChangeExamplesCommand::Alloc()))
	{
		ApplicationOutput("Failed to register @", "ChangeExamplesCommand"_s);
		return false;
	}
	return true;
}
