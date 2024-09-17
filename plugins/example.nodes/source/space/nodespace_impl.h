#ifndef NODESPACE_IMPL_H__
#define NODESPACE_IMPL_H__

#include "customnode-customnodespace_descriptions.h"
#include "maxon/node_spaces.h"
#include "maxon/nodesystem_class.h"

namespace maxonsdk
{

MAXON_DEPENDENCY(customnode_main);

class NodeSpaceExample : public maxon::Component<NodeSpaceExample, maxon::nodes::NodeSpaceInterface>
{
	MAXON_COMPONENT(NORMAL, maxon::nodes::NodeSpaceBaseClass);
	
public:
	
	static maxon::Result<void> CreateMaterialGraph(const maxon::nodes::NodesGraphModelRef& graph);
	
	static maxon::Result<void> ConfigurePreviewImageRequest(maxon::DataDictionaryObjectRef request);

	static maxon::Result<maxon::Bool> NodeMaterialMessageHandler(const maxon::nodes::NodesGraphModelRef& graph, maxon::Int32 messageId, void* messageData, void* nodeMaterial);
	
	MAXON_METHOD maxon::Result<void> Init(maxon::DataDictionary spaceData);
	
private:
	maxon::nodes::NodeSystemClass _class;
};

} // namespace maxonsdk
#endif // NODESPACE_IMPL_H__
