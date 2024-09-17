#ifndef NODEMATERIALEXPORT_IMPL_H__
#define NODEMATERIALEXPORT_IMPL_H__

#include "maxon/nodematerialimport.h"
#include "maxon/nodematerialexchange.h"
#include "maxon/material/materialparameter.h"
#include "lib_substance.h"

namespace maxonsdk
{
class ExampleNodeMaterialExport : public maxon::Component<ExampleNodeMaterialExport, maxon::nodes::NodeMaterialExportInterface>
{
	MAXON_COMPONENT(NORMAL);
public:

	MAXON_METHOD maxon::Result<void> Initialize(maxon::nodes::NodesGraphModelRef& graph, cinema::BaseDocument& baseDocument, const maxon::DataDictionary& config);

	MAXON_METHOD maxon::Result<maxon::Tuple<maxon::Id, maxon::DataDictionary>> GetParameters();

	MAXON_METHOD maxon::Result<maxon::HashMap<maxon::Id, maxon::Data>> GetTextures(const maxon::HashSet<maxon::Id>& texturedChannels);
};

} // namespace maxonsdk

#endif // NODEMATERIALEXPORT_IMPL_H__
