#include "simplematerialexport.h"
#include "maxon/material/datadescription_material_standard_surface.h"

namespace maxonsdk
{

maxon::Result<void> SimpleMaterialExport::Initialize(const BaseMaterial& baseMaterial, const maxon::DataDictionary& config)
{
	iferr_scope;

	_baseMaterial = &baseMaterial;
	maxon::material::MaterialTypesMap materialTypes = config.Get(maxon::material::EXPORT::CONFIG::MATERIALTYPESWITHSUPPORT) iferr_return;

	// We only support Standard Surface in this example.
	CheckArgument(materialTypes.Contains(maxon::MATERIAL::PORTBUNDLE::STANDARDSURFACE::GetId()) == true);

	return maxon::OK;
}

maxon::Result<maxon::Tuple<maxon::Id, maxon::DataDictionary>>SimpleMaterialExport:: GetParameters()
{
	iferr_scope;

	maxon::Tuple<maxon::Id, maxon::DataDictionary> result;
	result.first = maxon::MATERIAL::PORTBUNDLE::STANDARDSURFACE::GetId();
	result.second = maxon::material::ParameterStorageInterface::LoadDefaults(maxon::MATERIAL::PORTBUNDLE::STANDARDSURFACE::GetId()) iferr_return;

	if (_baseMaterial != nullptr)
	{
		const Int32 SIMPLEMATERIAL_COLOR = 1000;
		const maxon::Color simpleMaterialColor = _baseMaterial->GetDataInstanceRef().GetVector(SIMPLEMATERIAL_COLOR).GetColor();
		maxon::material::ParameterStorageInterface::Insert(result.second, maxon::MATERIAL::PORTBUNDLE::STANDARDSURFACE::BASE_COLOR, simpleMaterialColor, true, 0) iferr_return;
	}

	return result;
}

maxon::Result<maxon::HashMap<maxon::Id, maxon::Data>> SimpleMaterialExport::GetTextures(const maxon::HashSet<maxon::Id>& texturedChannels)
{
	maxon::HashMap<maxon::Id, maxon::Data> textures;

	// No textures to declare.
	return textures;
}
static maxon::GenericData g_materialExportRegistration;
maxon::Result<void> SimpleMaterialExport::Register()
{
	iferr_scope;

	const Int32 ID_SIMPLEMAT = 1001164;

	maxon::material::MaterialExportDescription description;
	description._type = ID_SIMPLEMAT;
	description._class = SimpleMaterialExport::GetClass();
	g_materialExportRegistration = maxon::material::MaterialExporters::Register(SimpleMaterialExport::GetDescriptor().GetId(), std::move(description)) iferr_return;
	return maxon::OK;
}

void SimpleMaterialExport::Free()
{
	g_materialExportRegistration = maxon::GenericData();
}

MAXON_COMPONENT_OBJECT_REGISTER(SimpleMaterialExport, "net.maxonsdk.class.simplematerialexport");

} // namespace maxonsdk
