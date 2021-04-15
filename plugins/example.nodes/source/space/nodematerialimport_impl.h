#ifndef NODEMATERIALIMPORT_IMPL_H__
#define NODEMATERIALIMPORT_IMPL_H__

#include "maxon/nodematerialimport.h"
#include "maxon/nodematerialexchange.h"
#include "maxon/material/materialparameter.h"
#include "lib_substance.h"

namespace maxonsdk
{
class ExampleNodeMaterialImport : public maxon::Component<ExampleNodeMaterialImport, maxon::nodes::NodeMaterialImportInterface>
{
	MAXON_COMPONENT(NORMAL);
public:

	MAXON_METHOD maxon::Result<void> Import(maxon::nodes::NodesGraphModelRef& graph, const maxon::material::MaterialExchangeData& materialData, BaseDocument& baseDocument);

private:
	static BaseList2D* FindSubstanceAsset(BaseDocument& baseDocument, const String& assetName);
};

} // namespace maxonsdk

#endif // NODEMATERIALIMPORT_IMPL_H__