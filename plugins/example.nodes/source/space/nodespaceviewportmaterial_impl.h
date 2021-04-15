#ifndef NODESPACEVIEWPORTMATERIAL_IMPL_H__
#define NODESPACEVIEWPORTMATERIAL_IMPL_H__

#include "maxon/node_spaces.h"

namespace maxonsdk
{
	
class NodeSpaceViewportMaterialExample : public maxon::Component<NodeSpaceViewportMaterialExample, maxon::nodes::MaterialExchangeInterface>
{
	MAXON_COMPONENT(NORMAL);
public:

	NodeSpaceViewportMaterialExample();

	virtual ~NodeSpaceViewportMaterialExample();
	
	MAXON_METHOD maxon::Result<void> Initialize(const maxon::Id& intent, const maxon::Id& materialType, const maxon::nodes::NodesGraphModelRef& graph, const maxon::NodePath& endNodePath, const maxon::NodePath& soloNodePath);

	MAXON_METHOD maxon::Result<maxon::DataDictionary> GetMaterialParameters();

	MAXON_METHOD maxon::Result<void> ConfigureTextureProviderRequest(maxon::DataDictionaryObjectRef request);

private:

	maxon::Result<maxon::DataDictionary> GetEndNodeParameters(const maxon::GraphNode& endNode);

	maxon::Result<maxon::DataDictionary> GetSoloParameters(const maxon::GraphNode & endNode);

	maxon::DataDictionary _bakeColors;
	maxon::nodes::NodesGraphModelRef _graph;

	maxon::Id _materialType;
	maxon::NodePath _endNodePath;
	maxon::NodePath _soloNodePath;
	maxon::Bool _isValueDynamic = false; // This is a rather random toggle to test dynamic values in Solo mode.

	maxon::GenericData _presetChangedRegistration;
};
	
} // namespace maxonsdk
#endif // NODESPACEVIEWPORTMATERIAL_IMPL_H__
