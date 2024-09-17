#include "nodematerialimport_impl.h"


namespace maxonsdk
{

maxon::Result<void> ExampleNodeMaterialImport::Import(maxon::nodes::NodesGraphModelRef& graph, const maxon::material::MaterialExchangeData& materialData, cinema::BaseDocument& baseDocument)
{
	iferr_scope;

	MAXON_SCOPE // We print some information on texture references and substances to the console.
	{
		const maxon::HashMap<maxon::Id, maxon::Data>& textureReferences = materialData._textures;
		for (const auto& textureEntry : textureReferences)
		{
			const maxon::Id& textureId = textureEntry.GetKey();
			const maxon::Data& textureData = textureEntry.GetValue();
			const maxon::DataType textureType = textureData.GetType();

			if (textureType == maxon::GetDataType<maxon::material::ImageReference>())
			{
				const maxon::material::ImageReference& imageReference = textureData.Get<maxon::material::ImageReference>() iferr_return;
				DiagnosticOutput("Parameter '@' has image reference to file '@'.", textureId, imageReference._absolutePath);
			}
			else if (textureType == maxon::GetDataType<maxon::material::SubstanceReference>())
			{
				const maxon::material::SubstanceReference& substanceReference = textureData.Get<maxon::material::SubstanceReference>() iferr_return;
				DiagnosticOutput("Parameter '@' has substance reference named '@'->'@' to file '@'.", textureId, substanceReference._assetName, substanceReference._outputChannelName, substanceReference._absolutePath);

				// The information stored in substanceReference should suffice to reference a specific instance of an asset instance through the API
				// defined in 'lib_substance.h'.
				cinema::BaseList2D* substanceAsset = FindSubstanceAsset(baseDocument, substanceReference._assetName);
				DiagnosticOutput("Cinema4d Substance Asset at address '@'.", (void*)substanceAsset);
			}
			else
			{
				DiagnosticOutput("Parameter '@' has texture reference of unknown type '@'.", textureId, textureType);
			}
		}
	}

	// In reality, we would want to create appropriate nodes or put the values directly on our end node inputs.
	// For example, MATERIAL::PORTBUNDLE::STANDARDSURFACE::EMISSION_COLOR -> maxonexample::NODE::ENDNODE::EMISSIONCOLOR

	MAXON_SCOPE // We implement the default import that would be available if we did register this class in our node space.
	{
		maxon::nodes::NodeMaterialImportHelperRef helper = maxon::nodes::NodeMaterialImportHelperInterface::CreateAndInitialize(graph, materialData._materialTypeId) iferr_return;
		helper.AddParameters(materialData._parameters) iferr_return;
		const maxon::nodes::NodeMaterialImportHelperInterface::GroupNodeData groupData = helper.Finalize() iferr_return;

		// This example space does not support image node, otherwise we could use this helper.
		// maxon::nodes::NodeMaterialImportHelperInterface::AddConnectedTextureNodes(groupData, materialData) iferr_return;
	}
	return maxon::OK;
}

cinema::BaseList2D* ExampleNodeMaterialImport::FindSubstanceAsset(cinema::BaseDocument& baseDocument, const cinema::String& assetName)
{
	cinema::AutoAlloc<cinema::AtomArray> substances;
	if (substances == nullptr)
		return nullptr;

	const cinema::Bool onlySelected = false; // Get all Substances in the document
	GetSubstances(&baseDocument, substances, onlySelected);
	for (cinema::Int32 index = 0; index < substances->GetCount(); ++index)
	{
		cinema::BaseList2D* asset = static_cast<cinema::BaseList2D*>(substances->GetIndex(index));
		if (asset == nullptr)
			continue; // This should not happen, but let's be safe.

		if (asset->GetName() == assetName)
			return asset;
	}
	return nullptr;
}

} // namespace maxonsdk
