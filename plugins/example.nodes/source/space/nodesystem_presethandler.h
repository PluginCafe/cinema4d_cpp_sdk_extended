#ifndef NODESYSTEM_PRESETHANDLER_H__
#define NODESYSTEM_PRESETHANDLER_H__

#include "nodesystem_observer.h"

namespace maxonsdk
{

class NodeSystemPresetChangedHandler : public NodeSystemChangedHandler
{
public:

	NodeSystemPresetChangedHandler();

	virtual ~NodeSystemPresetChangedHandler() override;

	virtual maxon::Result<void> HandleGraphChanged(const maxon::nodes::NodesGraphModelRef& graph) override;

private:

	static maxon::Result<void> ApplyMaterialPreset(maxon::GraphNode& endNode);

	static maxon::Result<void> ApplyPlasticPreset(maxon::GraphNode& inputs);

	static maxon::Result<void> ApplyGlassPreset(maxon::GraphNode& inputs);

	static maxon::Result<void> ApplyMetalPreset(maxon::GraphNode& inputs);

	static maxon::Result<void> ApplyLavaPreset(maxon::GraphNode& inputs);

	maxon::TimeStamp _lastTimeStamp;
};

} // namespace maxonsdk

#endif // NODESYSTEM_PRESETHANDLER_H__