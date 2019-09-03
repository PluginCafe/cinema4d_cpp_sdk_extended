#include "maxon/application.h"
#include "maxon/datadescriptiondefinitiondatabase.h"
#include "maxon/datadescriptiondatabase.h"
#include "maxon/datadescription_data.h"
#include "maxon/descriptionprocessor.h"
#include "maxon/micronodes.h"

#include "corenode_descriptions.h"

namespace maxonsdk
{
static maxon::BaseArray<maxon::GenericData> g_coreNodeDescriptions;
static maxon::Id g_corenodesDatabaseId = maxon::Id("net.maxonexample.nodes_corenodes.module");
	
using namespace maxon::corenodes;

class ExampleCoreNode
{
public:
	MAXON_PORT_INPUT(maxon::ColorA, colora);
	MAXON_PORT_OUTPUT(maxon::ColorA, result);

	class Impl : public BasicMicroNode
	{
	public:
		MAXON_ATTRIBUTE_FORCE_INLINE maxon::Result<void> Process(const Ports<colora, result>& ports) const
		{
			iferr_scope;
			
			const maxon::ColorA& inputColor = ports.colora();
			const maxon::ColorA shiftedColor = maxon::ColorA(maxon::FMod(inputColor.r + 0.2, 1.0), inputColor.g, inputColor.b, 1);
			ports.result.Update(shiftedColor);

			return maxon::OK;
		}
	};

	static maxon::Result<void> Init(const MicroNodeGroupRef& group)
	{
		return group.AddChild<Impl>();
	}
};

// We choose pure registration, because the node has no lazy evaluation (and is evaluated from left to right).
	MAXON_CORENODE_REGISTER_PURE(ExampleCoreNode, maxonexample::NODE_CORENODE::SIMPLECORENODE::GetId());

static maxon::Result<void> HandleCoreNodeDescriptions(const maxon::Id& databaseId)
{
	iferr_scope;

	maxon::BaseArray<maxon::IdAndVersion> ids = maxon::DataDescriptionDefinitionDatabaseInterface::GetRegisteredDescriptions(g_corenodesDatabaseId, maxon::DATADESCRIPTION_CATEGORY_DATA, maxon::LanguageRef()) iferr_return;
	for (const maxon::IdAndVersion& id : ids)
	{
		maxon::DataDescription description = maxon::DataDescriptionDatabaseInterface::LoadDescription(maxon::DATADESCRIPTION_CATEGORY_DATA, maxon::LanguageRef(), id.first) iferr_return;

		maxon::DataDictionary info = description.GetInfo();

		maxon::Id builderId = info.Get(maxon::DESCRIPTION::DATA::INFO::PROCESSOR, maxon::Id());
		if (builderId.IsPopulated())
		{
			const maxon::DescriptionProcessor& processor = maxon::DescriptionProcessors::Get(builderId);
			if (processor)
			{
				maxon::GenericData d = processor.Process(id.first, description) iferr_return;
				g_coreNodeDescriptions.Append(std::move(d)) iferr_return;
			}
		}
	}

	return maxon::OK;
}

static maxon::Result<void> HandleInitializeModule()
{
	iferr_scope_handler
	{
		err.CritStop();
		return err;
	};

	// get plugin location
	const maxon::Url& binaryUrl = maxon::g_maxon.GetUrl();
	// get plugin folder
	maxon::Url pluginDir = binaryUrl.GetDirectory();
	// get resource folder
	const maxon::Url coreNodesResourceUrl = pluginDir.Append("res"_s).Append("nodes"_s) iferr_return;

	// Load core node descriptions (they register automatically).
	maxon::DataDescriptionDefinitionDatabaseInterface::RegisterDatabaseWithUrl(g_corenodesDatabaseId, coreNodesResourceUrl) iferr_return;
	// Register the core nodes.
	HandleCoreNodeDescriptions(g_corenodesDatabaseId) iferr_return;

	return maxon::OK;
}

static void HandleFreeModule()
{
	iferr_scope_handler
	{
		err.CritStop();
		return;
	};

	g_coreNodeDescriptions.Reset();

}

MAXON_INITIALIZATION(HandleInitializeModule, HandleFreeModule);
	
} // namespace maxonsdk
