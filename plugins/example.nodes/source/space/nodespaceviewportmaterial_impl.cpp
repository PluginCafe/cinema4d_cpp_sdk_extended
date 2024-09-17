#include "nodespaceviewportmaterial_impl.h"
#include "maxon/datadescription_nodes.h"
#include "maxon/datadescription_node_spaces.h"
#include "maxon/derived_nodeattributes.h"
#include "customnode-customnodespace_descriptions.h"
#include "viewporttextureprovider_impl.h"
#include "nodesystem_presethandler.h"

#include "maxon/thread.h"
#include "maxon/weakref.h"

namespace maxonsdk
{
	
NodeSpaceViewportMaterialExample::NodeSpaceViewportMaterialExample()
{

}

NodeSpaceViewportMaterialExample::~NodeSpaceViewportMaterialExample()
{

}

maxon::Result<void> NodeSpaceViewportMaterialExample::Initialize(const maxon::Id& intent, const maxon::Id& materialType, const maxon::nodes::NodesGraphModelRef& graph, const maxon::NodePath& endNodePath, const maxon::NodePath& soloNodePath)
{
	iferr_scope;

	if (intent == maxon::nodes::MATERIALEXCHANGE::INTENT_EXPORT)
	{
		// This request indicates an ongoing export operation.
		// We could safely invest a bit of time here for better material export quality.
		DiagnosticOutput("Material is being exported.");
	}

	_materialType = materialType;
	_graph = graph;
	_endNodePath = endNodePath;
	_soloNodePath = soloNodePath;

	NodeSystemChangedHandlerRef changeHandler = NewObj(NodeSystemPresetChangedHandler) iferr_return;
	_presetChangedRegistration = NodeSystemObserverManager::RegisterChangeHandler(_graph, std::move(changeHandler)) iferr_return;
	
	return maxon::OK;
}

maxon::Result<maxon::DataDictionary> NodeSpaceViewportMaterialExample::GetMaterialParameters()
{
	iferr_scope_handler
	{
		err.DbgStop();
		return maxon::nodes::MaterialExchangeInterface::LoadMaterialDefaults(_materialType);
	};

	// We only support the viewport material.
	CheckState(_materialType == maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::GetId());

	// A populated solo node path indicates the request to show a particular node (or sub graph) without shading in the viewport.
	// Note: the viewport maps this to the luminance channel internally which allows the user to see 'raw data' as RGB color.
	const maxon::NodePath rootNodePath = (_soloNodePath.IsPopulated() == true) ? _soloNodePath : _endNodePath;

	if (rootNodePath.IsEmpty() == true)
	{
		// If there's no valid solo node or node end node, we return the defaults.
		return maxon::nodes::MaterialExchangeInterface::LoadMaterialDefaults(_materialType);
	}

	const maxon::GraphNode rootNode = _graph.GetNode(rootNodePath);

	const maxon::IdAndVersion& rootNodeIdAndVersion = rootNode.GetValue(maxon::nodes::AssetId).GetOrDefault() iferr_return;
	const maxon::Id& rootNodeId = rootNodeIdAndVersion.Get<0>();

	_isValueDynamic = !_isValueDynamic;
	if (rootNodeId == maxonexample::NODE::ENDNODE::GetId())
	{
		// Note that we also create a BRDF mapping if solo mode is requested, but points to an end node.
		return GetEndNodeParameters(rootNode);
	}
	else
	{
		// We just grab some value for an unlit result.
		return GetSoloParameters(rootNode);
	}
}

maxon::Result<void> NodeSpaceViewportMaterialExample::ConfigureTextureProviderRequest(maxon::DataDictionaryObjectRef request)
{
	iferr_scope;

	// This method is called when we signal a non-constant parameter above and the viewport actually requests textures for redraw.
	// We set our provider class and be done with it.
	// We could of course add additional metadata to the dictionary, if we needed to.
	request.Set(maxon::nodes::PREVIEWIMAGEREQUEST::PROVIDER, ViewportTextureProviderExample::GetClass()) iferr_return;

	request.Set(maxon::Id("net.maxonexample.viewporttextureprovider.bakecolors"), _bakeColors) iferr_return;

	return maxon::OK;
}

maxon::Result<maxon::DataDictionary> NodeSpaceViewportMaterialExample::GetEndNodeParameters(const maxon::GraphNode& endNode)
{
	iferr_scope;
	
	const maxon::GraphNode inputs = endNode.GetInputs() iferr_return;
	
	const maxon::Bool isMetal = inputs.FindChild(maxonexample::NODE::ENDNODE::METAL).GetEffectivePortValue<maxon::Bool>().GetOrDefault() iferr_return;
	
	const maxon::Color specularColor = inputs.FindChild(maxonexample::NODE::ENDNODE::SPECULARCOLOR).GetEffectivePortValue<maxon::Color>().GetOrDefault() iferr_return;
	const maxon::Float specularColorWeight = inputs.FindChild(maxonexample::NODE::ENDNODE::SPECULARCOLORINTENSITY).GetEffectivePortValue<maxon::Float>().GetOrDefault() iferr_return;
	const maxon::Float specularRoughness = inputs.FindChild(maxonexample::NODE::ENDNODE::SPECULARROUGHNESS).GetEffectivePortValue<maxon::Float>().GetOrDefault() iferr_return;
	const maxon::Float specularIOR = inputs.FindChild(maxonexample::NODE::ENDNODE::SPECULARIOR).GetEffectivePortValue<maxon::Float>().GetOrDefault() iferr_return;
	
	const maxon::Color baseColor = inputs.FindChild(maxonexample::NODE::ENDNODE::BASECOLOR).GetEffectivePortValue<maxon::Color>().GetOrDefault() iferr_return;
	const maxon::Float baseColorWeight = inputs.FindChild(maxonexample::NODE::ENDNODE::BASECOLORINTENSITY).GetEffectivePortValue<maxon::Float>().GetOrDefault() iferr_return;
	
	const maxon::Color emissionColor = inputs.FindChild(maxonexample::NODE::ENDNODE::EMISSIONCOLOR).GetEffectivePortValue<maxon::Color>().GetOrDefault() iferr_return;
	const maxon::Float emissionColorWeight = inputs.FindChild(maxonexample::NODE::ENDNODE::EMISSIONCOLORINTENSITY).GetEffectivePortValue<maxon::Float>().GetOrDefault() iferr_return;
	
	const maxon::Float refractionIntensity = inputs.FindChild(maxonexample::NODE::ENDNODE::REFRACTIONINTENSITY).GetEffectivePortValue<maxon::Float>().GetOrDefault() iferr_return;
	
	const maxon::Float surfaceAlpha = inputs.FindChild(maxonexample::NODE::ENDNODE::SURFACEALPHA).GetEffectivePortValue<maxon::Float>().GetOrDefault() iferr_return;
	
	maxon::DataDictionary parameters;
	
	if (isMetal == true)
	{
		maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::TYPE, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::TYPE_ENUM_METAL, true, 0) iferr_return;
	}
	else
	{
		maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::TYPE, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::TYPE_ENUM_DIELECTRIC, true, 0) iferr_return;
	}

	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::SPECULAR_COLOR, specularColor, true, 0) iferr_return;
	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::SPECULAR_COLOR_WEIGHT, specularColorWeight, true, 0) iferr_return;
	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::SPECULAR_ROUGHNESS, specularRoughness, true, 0) iferr_return;
	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::SPECULAR_IOR, specularIOR, true, 0) iferr_return;

	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::BASE_COLOR, baseColor, true, 0) iferr_return;
	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::BASE_COLOR_WEIGHT, baseColorWeight, true, 0) iferr_return;

	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::EMISSION_COLOR, emissionColor, true, 0) iferr_return;
	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::EMISSION_COLOR_WEIGHT, emissionColorWeight, true, 0) iferr_return;

	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::TRANSMISSIVITY, refractionIntensity, true, 0) iferr_return;

	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::ALPHA, surfaceAlpha, true, 0) iferr_return;

	return parameters;
}

maxon::Result<maxon::DataDictionary> NodeSpaceViewportMaterialExample::GetSoloParameters(const maxon::GraphNode & endNode)
{
	iferr_scope;
	
	maxon::HashMap<maxon::Id, maxon::nodes::SampleFloat32> test;

	// What we are doing right now is a bit stupid: we mark any change within the node system as a potential change of the below dynamic textures.

	// Note: only return a changed timestamp when the parameter really has changed.
	// We should avoid uploading data to the GPU that hasn't changed.
	const maxon::UInt timestamp = endNode.GetGraph().GetModificationStamp();
	
	maxon::Color constantColor = maxon::Color(1, 0, 1);
	
	// Let's just grab the first Color value we can find on this node.
	const maxon::GraphNode inputs = endNode.GetInputs() iferr_return;
	inputs.GetChildren([&constantColor](const maxon::GraphNode & input) -> maxon::Result<maxon::Bool>
	{
	 iferr_scope;
	 
	 maxon::ConstDataPtr inputData = input.GetValue(maxon::EffectivePortValue, maxon::DataType::DefaultValue()) iferr_return;
	 if (inputData.GetType().Is<maxon::Color>())
	 {
		 constantColor = inputData.Get< maxon::Color>() iferr_return;
		 return false;
	 }
	 else if (inputData.GetType().Is<maxon::ColorA>())
	 {
		 const maxon::ColorA inputColorA = inputData.Get<maxon::ColorA>() iferr_return;
		 constantColor = inputColorA.GetColor3();
		 return false;
	 }
	 
	 // Continue searching.
	 return true;
	 
	}, maxon::NODE_KIND::INPORT) iferr_return;

	maxon::DataDictionary parameters;
	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::TYPE, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::TYPE_ENUM_CONSTANT, true, 0) iferr_return;
	maxon::nodes::MaterialExchangeInterface::Insert(parameters, maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::EMISSION_COLOR, constantColor, _isValueDynamic, timestamp) iferr_return;

	// We store the identified color for mockup baking later.
	_bakeColors.Set(maxon::Id(maxon::NODESPACE::EXCHANGE::BUNDLE::VIEWPORTMATERIAL::EMISSION_COLOR.Get()), constantColor) iferr_return;
	
	return parameters;
}

} // namespace maxonsdk
