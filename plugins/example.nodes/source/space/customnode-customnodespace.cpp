#include "maxon/application.h"
#include "maxon/datadescriptiondatabase.h"
#include "maxon/nodes_corenodes_base.h"
#include "maxon/nodeslib.h"
#include "maxon/datadescriptiondefinitiondatabase.h"

#include "customnode-customnodespace_descriptions.h"
#include "nodespaceviewportmaterial_impl.h"
#include "nodesystemclass_impl.h"
#include "nodespace_impl.h"
#include "previewimageprovider_impl.h"
#include "thumbnailgenerator_impl.h"
#include "viewporttextureprovider_impl.h"

namespace maxonsdk
{

static maxon::GenericData g_exampleNodeSpace;
static maxon::Id g_exampleNodesDatabaseID = maxon::Id("net.maxonexample.nodes.module");

static maxon::nodes::NodeSystemClass g_nodeSystemClass;

// This description processor has to be used for all nodes of the example namespace unless they register themselves at the BuiltinNodes registry (such as DynamicNode).
MAXON_DECLARATION_REGISTER(maxon::DescriptionProcessors, "net.maxonexample.nodespace.descriptionprocessor")
{
	return maxon::nodes::NodesLib::CreateNodeDescriptionProcessor([] (const maxon::Id& descriptionId, const maxon::DataDescriptionDefinition& dataDef, const maxon::DataDescription& dataDescription) -> maxon::Result<maxon::nodes::NodeTemplate>
	{
		return maxon::nodes::NodesLib::CreateLazyTemplate(descriptionId,
			[descriptionId]() -> maxon::Result<maxon::nodes::NodeTemplate>
			{
				return maxon::nodes::NodesLib::BuildNodeFromDescription(descriptionId, g_nodeSystemClass);
			}, g_nodeSystemClass);
	});
}

static maxon::Result<void> NodeSpaceExampleInitialize()
{
	iferr_scope_handler
	{
		err.CritStop();
		return err;
	};

	g_nodeSystemClass = NodeSystemClassExample::GetClass().Create() iferr_return;

	// get plugin location
	const maxon::Url& binaryUrl = maxon::g_maxon.GetUrl();
	// get plugin folder
	maxon::Url pluginDir = binaryUrl.GetDirectory();
	// get resource folder
	const maxon::Url resourceUrl = pluginDir.Append("res"_s).Append("nodes"_s) iferr_return;
	// Load the space and user node descriptions.
	maxon::DataDescriptionDefinitionDatabaseInterface::RegisterDatabaseWithUrl(g_exampleNodesDatabaseID, resourceUrl) iferr_return;

	// Register the space.
	const maxon::Id spaceDescriptionId = maxon::Id("net.maxonexample.nodespace.example");
	maxon::DataDictionary exampleSpaceData = maxon::nodes::NodeSpaceHelpersInterface::LoadDescription(spaceDescriptionId) iferr_return;
	maxon::nodes::NodeSpaceRef exampleNodeSpace = NodeSpaceExample::CreateInit(exampleSpaceData) iferr_return;
	g_exampleNodeSpace = maxon::nodes::MaterialNodeSpaces::Register(NodeSpaceExample::GetDescriptor().GetId(), exampleNodeSpace) iferr_return;

	return maxon::OK;
}

static void NodeSpaceExampleFree()
{
	iferr_scope_handler
	{
		err.CritStop();
		return;
	};

	// Unregister the node space.
	g_exampleNodeSpace = maxon::GenericData();

	// Unregister all descriptions.
	maxon::DataDescriptionDefinitionDatabaseInterface::UnregisterDatabase(g_exampleNodesDatabaseID) iferr_return;

	g_nodeSystemClass = nullptr;
}

MAXON_INITIALIZATION(NodeSpaceExampleInitialize, NodeSpaceExampleFree);

} // namespace maxonsdk
