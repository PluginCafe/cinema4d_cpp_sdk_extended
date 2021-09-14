#include "nodespace_impl.h"

#include "gradientpreviewprovider.h"
#include "maxon/datadescription_node_spaces.h"
#include "nodematerialimport_impl.h"
#include "previewimageprovider_impl.h"
#include "nodesystemclass_impl.h"
#include "nodespaceviewportmaterial_impl.h"
#include "nodematerialexport_impl.h"

#include "maxon/definitions/nodes_utility.h"
#include "maxon/nodes_corenodes_base.h"
#include "maxon/nodeslib.h"

namespace maxonsdk
{
		
maxon::Result<void> NodeSpaceExample::CreateMaterialGraph(const maxon::nodes::NodesGraphModelRef& graph)
{
	iferr_scope;
	
	MAXON_SCOPE // Create the end node setup
	{
		// Instantiate a node that only has a description, but no process function.
		maxon::nodes::NodeTemplate userNodeSystem = maxon::nodes::NodesLib::LoadTemplate(maxon::AssetInterface::GetBuiltinRepository(), maxonexample::NODE::USERNODE::GetId()) iferr_return;
		maxon::GraphNode userGraphNode = graph.AddChild(maxon::Id("user node instance"), userNodeSystem) iferr_return;
	
		// Instantiate a node with bundles and custom gui. 
		maxon::nodes::NodeTemplate bundleNodeSystem = maxon::nodes::NodesLib::LoadTemplate(maxon::AssetInterface::GetBuiltinRepository(), maxonexample::NODE::BUNDLENODE::GetId()) iferr_return;
		maxon::GraphNode bundleGraphNode = graph.AddChild(maxon::Id("bundle node instance"), bundleNodeSystem) iferr_return;

		// Instantiate a node that only has a description, but no process function.
		maxon::nodes::NodeTemplate endNodeSystem = maxon::nodes::NodesLib::LoadTemplate(maxon::AssetInterface::GetBuiltinRepository(), maxonexample::NODE::ENDNODE::GetId()) iferr_return;
		maxon::GraphNode endGraphNode = graph.AddChild(maxon::Id("end node instance"), endNodeSystem) iferr_return;
	
		// Replace the default value for the first instance.
		maxon::GraphNode valuePort = userGraphNode.GetInputs().FindChild(maxonexample::NODE::USERNODE::COLORA) iferr_return;
		valuePort.SetDefaultValue(maxon::Data(maxon::ColorA(1.0, 1.0, 0.0, 1.0))) iferr_return;
	}

	MAXON_SCOPE // Instantiate a gradient node and connect one of the knots.
	{
		// Create a red color.
		maxon::nodes::NodeTemplate colorNodeSystem = maxon::nodes::NodesLib::LoadTemplate(maxon::AssetInterface::GetBuiltinRepository(), maxon::NODE::TYPE::GetId()) iferr_return;
		maxon::GraphNode colorNode = graph.AddChild(maxon::Id("Color node instance"), colorNodeSystem) iferr_return;

		maxon::GraphNode datatypePort = colorNode.GetInputs().FindChild(maxon::NODE::TYPE::DATATYPE) iferr_return;
		datatypePort.SetDefaultValue(maxon::Id("net.maxon.parametrictype.col<3,float>")) iferr_return;
		maxon::GraphNode valuePort = colorNode.GetInputs().FindChild(maxon::NODE::TYPE::IN) iferr_return;
		valuePort.SetDefaultValue(maxon::Color(1, 0, 0)) iferr_return;
		maxon::GraphNode colorOutPort = colorNode.GetOutputs().FindChild(maxon::NODE::TYPE::OUT) iferr_return;

		// Create the gradient.
		maxon::nodes::NodeTemplate gradientNodeSystem = maxon::nodes::NodesLib::LoadTemplate(maxon::AssetInterface::GetBuiltinRepository(), maxonexample::NODE::GRADIENTNODE::GetId()) iferr_return;
		maxon::GraphNode gradientNode = graph.AddChild(maxon::Id("gradient node instance"), gradientNodeSystem) iferr_return;

		// We connect the red color to the first knot.
		maxon::GraphNode gradientBundle = gradientNode.GetInputs().FindChild(maxonexample::NODE::GRADIENTNODE::FIRSTBUNDLE) iferr_return;
		gradientBundle.GetChildren(
		[&colorOutPort](const maxon::GraphNode& bundleKnot) -> maxon::Result<maxon::Bool>
		{
			iferr_scope;
			maxon::GraphNode colorPort = bundleKnot.FindChild(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::COLOR) iferr_return;
			colorOutPort.Connect(colorPort) iferr_return;

			// We are done and stop the iteration over children.
			return false;
		}) iferr_return;
	}
	
	return maxon::OK;
}

// Defaults for nodes that contain variadics cannot be fully defined in the Resource Editor.
// Inside the Resource Editor you can define the defaults of the datatype that is bundled, but you may not define
// defaults for individual instances when bundling this datatype.
// As a solution you can defined defaults whenever the bundling node is instantiated.
// This way, we can define defaults that are specific to a bundle entry.
MAXON_DECLARATION_REGISTER(maxon::nodes::DescriptionFinalizers, )
{
	objectId = maxonexample::NODE::GRADIENTNODE::GetId();
	return maxon::nodes::DescriptionFinalizers::EntryType([](const maxon::nodes::MutableRoot& root)-> maxon::Result<void>
	{
		iferr_scope;
		maxon::nodes::MutablePort firstKnot0 = root.GetInputs().FindPort(maxonexample::NODE::GRADIENTNODE::FIRSTBUNDLE).FindPort(maxon::Id("_0")) iferr_return;
		if (firstKnot0)
		{
			firstKnot0.FindPort(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::COLOR).SetDefaultValue(maxon::ColorA()) iferr_return;
			firstKnot0.FindPort(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::POSITION).SetDefaultValue(maxon::Float(0.0)) iferr_return;
		}
		maxon::nodes::MutablePort secondKnot0 = root.GetInputs().FindPort(maxonexample::NODE::GRADIENTNODE::SECONDBUNDLE).FindPort(maxon::Id("_0")) iferr_return;
		if (secondKnot0)
		{
			secondKnot0.FindPort(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::COLOR).SetDefaultValue(maxon::ColorA()) iferr_return;
			secondKnot0.FindPort(maxonexample::DATATYPE::PORTBUNDLE::GRADIENT::POSITION).SetDefaultValue(maxon::Float(0.0)) iferr_return;
		}
		return maxon::OK;
	});
}

maxon::Result<void> NodeSpaceExample::ConfigurePreviewImageRequest(maxon::DataDictionaryObjectRef request)
{
	iferr_scope;

	const	maxon::InternedId intent = request.Get(maxon::nodes::PREVIEWIMAGEREQUEST::INTENT, maxon::nodes::PREVIEWIMAGEREQUEST::INTENT_NODE2D);
	
	switch (ID_SWITCH(intent))
	{
		case ID_CASE(maxon::nodes::PREVIEWIMAGEREQUEST::INTENT_GRADIENTAVERAGE):
		case ID_CASE(maxon::nodes::PREVIEWIMAGEREQUEST::INTENT_GRADIENTBAR):
		{
			// We need to treat gradient previews in a special way as we are not asked to provide a preview of the node's output, but rather represent
			// the internal gradient bundle's result. Note that our example node contains multiple of these bundles to be really extreme.
			request.Set(maxon::nodes::PREVIEWIMAGEREQUEST::PROVIDER, GradientPreviewProvider::GetClass()) iferr_return;
			break;
		}
		default:
		{
			request.Set(maxon::nodes::PREVIEWIMAGEREQUEST::PROVIDER, PreviewImageProviderExample::GetClass()) iferr_return;
			break;
		}
	}
	return maxon::OK;
}

maxon::Result<Bool> NodeSpaceExample::NodeMaterialMessageHandler(const maxon::nodes::NodesGraphModelRef& graph, Int32 messageId, void* messageData, void* nodeMaterial)
{
	return false;
}

MAXON_METHOD maxon::Result<void> NodeSpaceExample::Init(maxon::DataDictionary spaceData)
{
	iferr_scope;
	
	_class = NodeSystemClassExample::GetClass().Create() iferr_return;
	// Register the function that will be called when the graph is created.
	spaceData.Set(maxon::nodes::NODESPACE::CREATEMATERIALGRAPHFUNC, maxon::nodes::NODESPACE::CreateMaterialGraphFunc(CreateMaterialGraph)) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::CONFIGUREPREVIEWIMAGEREQUESTFUNC, maxon::nodes::NODESPACE::ConfigurePreviewImageRequestFunc(ConfigurePreviewImageRequest)) iferr_return;
	
	maxon::BaseArray<maxon::Id> materialEndNodeIds;
	materialEndNodeIds.Append(maxonexample::NODE::ENDNODE::GetId()) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::MATERIALENDNODEIDS, std::move(materialEndNodeIds)) iferr_return;
	
	// Provide all types that require a right-click menu to the user for the 3d scene choice, see maxon::nodes::PREVIEW::SCENETYPE.
	maxon::BaseArray<maxon::Id> materialPreviewNodeIds;
	materialPreviewNodeIds.Append(maxonexample::NODE::ENDNODE::GetId()) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::MATERIALPREVIEWIDS, std::move(materialPreviewNodeIds)) iferr_return;

	spaceData.Set(maxon::nodes::NODESPACE::NODESYSTEMCLASS, _class) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::IMAGENODEASSETID, maxon::Id()) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::IMAGENODEPORTS, maxon::nodes::NODESPACE::ImageNodePortTuple()) iferr_return;

	// We define the viewport and exchange support.
	maxon::BaseArray<maxon::Id> materialExchangeBundleIds;
	materialExchangeBundleIds.Append(maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::GetId()) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::MATERIALEXCHANGEBUNDLEIDS, std::move(materialExchangeBundleIds)) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::MATERIALEXCHANGECLASS, NodeSpaceViewportMaterialExample::GetClass()) iferr_return;

	spaceData.Set(maxon::nodes::NODESPACE::NODEMATERIALIMPORTCLASS, ExampleNodeMaterialImport::GetClass()) iferr_return;
	spaceData.Set(maxon::nodes::NODESPACE::NODEMATERIALEXPORTCLASS, ExampleNodeMaterialExport::GetClass()) iferr_return;

	spaceData.Set(maxon::nodes::NODESPACE::MATERIALMESSAGEHANDLERFUNC, maxon::nodes::NODESPACE::MaterialMessageHandlerFunc(NodeMaterialMessageHandler)) iferr_return;

	super.Init(spaceData) iferr_return;
	return maxon::OK;
}
		
} // namespace maxonsdk
