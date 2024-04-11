#include "nodematerialexport_impl.h"

#include "maxon/material/materialexport.h"
#include "maxon/material/datadescription_material_standard_surface.h"

namespace maxonsdk
{

maxon::Result<void> ExampleNodeMaterialExport::Initialize(maxon::nodes::NodesGraphModelRef& graph, BaseDocument& baseDocument, const maxon::DataDictionary& config)
{
	iferr_scope;

	maxon::material::MaterialTypesMap materialTypes = config.Get(maxon::material::EXPORT::CONFIG::MATERIALTYPESWITHSUPPORT) iferr_return;

	// We only support Standard Surface in this example.
	CheckArgument(materialTypes.Contains(maxon::MATERIAL::PORTBUNDLE::STANDARDSURFACE::GetId()) == true);

	return maxon::OK;
}

maxon::Result<maxon::Tuple<maxon::Id, maxon::DataDictionary>> ExampleNodeMaterialExport::GetParameters()
{
	iferr_scope;

	maxon::Tuple<maxon::Id, maxon::DataDictionary> result;
	result.first = maxon::MATERIAL::PORTBUNDLE::STANDARDSURFACE::GetId();
	result.second = maxon::material::ParameterStorageInterface::LoadDefaults(maxon::MATERIAL::PORTBUNDLE::STANDARDSURFACE::GetId()) iferr_return;

	return result;
}

maxon::Result<maxon::HashMap<maxon::Id, maxon::Data>> ExampleNodeMaterialExport::GetTextures(const maxon::HashSet<maxon::Id>& texturedChannels)
{
	maxon::HashMap<maxon::Id, maxon::Data> textures;
	return textures;
}

} // namespace maxonsdk
