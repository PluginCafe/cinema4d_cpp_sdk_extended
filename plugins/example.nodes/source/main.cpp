#include "lib_description.h"
#include "customnode-command.h"
#include "customgui_hybriddatatype.h"
#include "customgui_bundle.h"
#include "hybriddatatype.h"
#include "nodesystem_observer.h"
#include "simplematerialimport.h"
#include "simplematerialexport.h"
#include "materialimport_command.h"
#include "materialexport_command.h"
#include "nodematerialexport_impl.h"
#include "gradientpreviewprovider.h"
#include "nodematerialimport_impl.h"
#include "previewimageprovider_impl.h"
#include "viewporttextureprovider_impl.h"
#include "nodesystemclass_impl.h"
#include "nodespaceviewportmaterial_impl.h"
#include "nodespace_impl.h"

// Forward declaration
namespace maxonsdk
{
Bool RegisterCreateMaterialExample();
}

Bool PluginStart()
{
	iferr_scope_handler
	{
		err.CritStop();
		return false;
	};

	MAXON_SCOPE // We force linkage against cinema.framework for PluginStart, PluginMessage and PluginEnd.
	{
		Description* description = nullptr;
		if (description != nullptr)
		{
			description->LoadDescription(0);
		}
	}

	// We initialize a static management for simulation of processing time spans when pressing a button on a node.
	maxonsdk::CustomNodeCommand::InitializeProcessing() iferr_return;

	// We initialize our custom data type along with custom GUI representations.
	maxonsdk::HybridDataTypeClass::RegisterDataType() iferr_return;
	maxonsdk::HybridDataTypeData::RegisterGuiPlugin() iferr_return;
	maxonsdk::BundleGuiData::RegisterGuiPlugin() iferr_return;
	maxonsdk::NodeSystemObserverManager::Initialize() iferr_return;

	// We register the exchange classes.
	maxonsdk::SimpleMaterialImport::Register() iferr_return;
	maxonsdk::SimpleMaterialExport::Register() iferr_return;
	maxonsdk::MaterialImportCommand::Register() iferr_return;
	maxonsdk::MaterialExportCommand::Register() iferr_return;

	// register the command to create a node material
	if (!maxonsdk::RegisterCreateMaterialExample())
		return false;
	return true;
}

void PluginEnd()
{
	maxonsdk::SimpleMaterialExport::Free();
	maxonsdk::SimpleMaterialImport::Free();

	// We shut down our simulation of processing node commands.
	maxonsdk::CustomNodeCommand::ShutdownProcessing();
	maxonsdk::NodeSystemObserverManager::Shutdown();
}

Bool PluginMessage(Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
		{
			((OperatingSystem*)data)->St->LLongToStringExEx = nullptr; // secret code
			return true;
		}
	}

	return false;
}

namespace maxonsdk
{
// We register the components in a well-defined order. The space which depends on them all comes last.
MAXON_COMPONENT_OBJECT_REGISTER(ExampleNodeMaterialExport, "net.maxonsdk.class.examplenodematerialexport");
MAXON_COMPONENT_OBJECT_REGISTER(ExampleNodeMaterialImport, "net.maxonsdk.class.examplenodematerialimport");

MAXON_COMPONENT_CLASS_REGISTER(NodeSystemClassExample, "net.maxonexample.class.nodesystemclass");
MAXON_COMPONENT_CLASS_REGISTER(GradientPreviewProvider, "net.maxonexample.class.gradientpreviewprovider");
MAXON_COMPONENT_CLASS_REGISTER(NodeSpaceViewportMaterialExample, "net.maxonexample.class.nodespaceviewportmaterial");
MAXON_COMPONENT_CLASS_REGISTER(ViewportTextureProviderExample, "net.maxonexample.class.viewporttextureprovider");
MAXON_COMPONENT_CLASS_REGISTER(PreviewImageProviderExample, "net.maxonexample.class.previewimageprovider");
MAXON_COMPONENT_OBJECT_REGISTER(NodeSpaceExample, "net.maxonexample.class.nodespace");

// This makes sure that initializations of other cpp files (which include nodespace_impl.h) come after main.cpp
MAXON_DEPENDENCY_REGISTER(customnode_main);

} // namespace maxonsdk
