#include "c4d_basebitmap.h"
#include "c4d_basematerial.h"
#include "c4d_general.h"
#include "c4d_shader.h"
#include "lib_description.h"
#include "lib_scene_color_converter.h"

#include "maxon/assets.h"
#include "maxon/gfx_image_colorprofile.h"
#include "maxon/iostreams.h"

#include "Mmaterial.h"

#include "examples_ocio.h"


//! [ConvertSceneOrElements]
maxon::Result<void> ConvertSceneOrElements(BaseDocument* doc)
{
	iferr_scope;

	// Using SceneColorConverter is only necessary when the operation must be carried out silently 
	// with no GUI. If showing a popup dialog is acceptable or even desired, one can simply call
	// the command plugin for it.

	// Int32 idConvertSceneCommand = 1059360;
	// CallCommand(idConvertSceneCommand);

	// Ensure that the document is in OCIO color management mode.
	BaseContainer* bc = doc->GetDataInstance();
	if (bc->GetInt32(DOCUMENT_COLOR_MANAGEMENT) != DOCUMENT_COLOR_MANAGEMENT_OCIO)
	{
		bc->SetInt32(DOCUMENT_COLOR_MANAGEMENT, DOCUMENT_COLOR_MANAGEMENT_OCIO);
		doc->UpdateOcioColorSpaces();
		EventAdd();
	}

	// Get then OCIO render space name to convert to, we could also define it manually here.
	maxon::CString renderSpaceName, _dummy;
	doc->GetActiveOcioColorSpacesNames(renderSpaceName, _dummy, _dummy, _dummy);

	// Allocate the converter and initialize the converter, currently it is not possible to retrieve
	// the linear and non-linear input color space names input-low and input-high that are associated
	// with an OCIO config file. Instead they must be hardcoded. Except for the the target render
	// space, the Init call uses here the default values of the native dialog.
	AutoAlloc<SceneColorConverter> converter;
	if (!converter)
		return maxon::OutOfMemoryError(MAXON_SOURCE_LOCATION, "Could not allocate converter interface."_s);

	converter->Init(doc, "sRGB"_cs, "scene-linear Rec.709-sRGB"_cs, renderSpaceName) iferr_return;

	// Carry out the conversion for the whole document, the second argument is the conversion target,
	// here the whole document. But it could for example also be a singular BaseMaterial. Multiple
	// objects in a single document can be converted at once with ::ConvertObjects().
	maxon::HashSet<BaseList2D*> result;
	converter->ConvertObject(doc, doc, result) iferr_return;

	// When converting a whole document, we should mark it as already converted.
	doc->SetParameter(DOCUMENT_COLOR_MANAGEMENT_OCIO_CONVERTED, GeData(true), DESCFLAGS_SET::NONE);

	ApplicationOutput("\n@():", MAXON_FUNCTIONNAME);
	ApplicationOutput("\tConverted @ elements in '@' to OCIO Render space named '@'.", 
		result.GetCount(), doc, renderSpaceName);

	return maxon::OK;
}
//! [ConvertSceneOrElements]

//! [CopyColorManagementSettings]
maxon::Result<void> CopyColorManagementSettings(BaseDocument* doc)
{
	iferr_scope;

	// Allocate a new document and copy over the settings from #doc to #newDoc. This is just a 
	// convenient way to copy all color management settings from one document to another. This
	// does NOT entail a conversion of scene elements to OCIO, see ConvertSceneOrElements() for that.
	AutoAlloc<BaseDocument> newDoc;
	BaseDocument::CopyLinearWorkflow(doc, newDoc, false);
	newDoc->SetName(doc->GetName() + "(Copy)");

	ApplicationOutput("\n@():", MAXON_FUNCTIONNAME);
	ApplicationOutput("\tCopied color management settings from '@' to '@'.", 
		doc->GetName(), newDoc->GetName());

	newDoc.Free();
	return maxon::OK;
}
//! [CopyColorManagementSettings]

//! [ConvertOcioColors]
maxon::Result<void> ConvertOcioColors(BaseDocument* doc)
{
	iferr_scope;

	if (MAXON_UNLIKELY(!doc))
		return maxon::NullptrError(MAXON_SOURCE_LOCATION, "Invalid document pointer"_s);

	// OcioConverter can only initialized with a document which is in OCIO color management mode.
	BaseContainer* bc = doc->GetDataInstance();
	if (bc->GetInt32(DOCUMENT_COLOR_MANAGEMENT) != DOCUMENT_COLOR_MANAGEMENT_OCIO)
	{
		bc->SetInt32(DOCUMENT_COLOR_MANAGEMENT, DOCUMENT_COLOR_MANAGEMENT_OCIO);
		doc->UpdateOcioColorSpaces();
		EventAdd();
	}

	// Initialize an OcioConverter for #doc. OcioConverter is a helper interface to convert colors
	// between the color spaces associated with the OCIO configuration of a document.
	const OcioConverter* converter = OcioConverter::Init(doc) iferr_return;
	if (!converter)
		return maxon::NullptrError(MAXON_SOURCE_LOCATION, "Could not init OCIO converter for document."_s);

	// The to be converted color, it has no implicitly defined color space.
	const maxon::Vector64 colorInput { 1, 0, 0 };

	// Convert the color from sRGB space to render space. With the default OCIO settings of Cinema 
	// 4D 2023.1, this will convert to ACEScg space. Doing this is only necessary when computation
	// results or color definitions are explicitly outside of the render space.
	const maxon::Vector64 colorRender = converter->TransformColor(
		colorInput, COLORSPACETRANSFORMATION::OCIO_SRGB_TO_RENDERING);

	// Convert the color from render space to display space, i.e., the space the physical display 
	// device operates in. With the default OCIO settings of Cinema 4D 2023.1, this will convert 
	// from ACEScg to sRGB space.
	const maxon::Vector64 colorDisplay = converter->TransformColor(
		colorRender, COLORSPACETRANSFORMATION::OCIO_RENDERING_TO_DISPLAY);

	// Convert a color from render space to view transform (space), i.e., to the value which
	// will be finally shown on screen to the user. With the default OCIO settings of Cinema 4D 
	// 2023.1, this will convert from ACEScg to ACES 1.0 SDR-video space.
	const maxon::Vector64 colorView = converter->TransformColor(
		colorRender, COLORSPACETRANSFORMATION::OCIO_RENDERING_TO_VIEW);

	// There are multiple other conversion paths to be found in #COLORSPACETRANSFORMATION, including
	// inverse operations, as for example converting a color in view space back to the render space.
	const maxon::Vector64 colorRenderViewRender = converter->TransformColor(
		colorView, COLORSPACETRANSFORMATION::OCIO_VIEW_TO_RENDERING);
	
	// Print the results.
	ApplicationOutput("\n@():", MAXON_FUNCTIONNAME);
	ApplicationOutput("\tinput: @", colorInput);
	ApplicationOutput("\tinput->render: @", colorRender);
	ApplicationOutput("\trender->display: @", colorDisplay);
	ApplicationOutput("\trender->view: @", colorView);
	ApplicationOutput("\trender->view->render: @", colorRenderViewRender);

	return maxon::OK;
}
//! [ConvertOcioColors]

//! [GetSetColorManagementSettings]
maxon::Result<void> GetSetColorManagementSettings(BaseDocument* doc)
{
	iferr_scope;

	if (MAXON_UNLIKELY(!doc))
		return maxon::NullptrError(MAXON_SOURCE_LOCATION, "Invalid document pointer"_s);
	
	// Ensure that the document is in OCIO color management mode.
	BaseContainer* bc = doc->GetDataInstance();
	if (bc->GetInt32(DOCUMENT_COLOR_MANAGEMENT) != DOCUMENT_COLOR_MANAGEMENT_OCIO)
	{
		bc->SetInt32(DOCUMENT_COLOR_MANAGEMENT, DOCUMENT_COLOR_MANAGEMENT_OCIO);
		doc->UpdateOcioColorSpaces();
		EventAdd();
	}

	// Print the names for all OCIO color spaces and transforms that are available in the OCIO
	// configuration file loaded by the document.
	ApplicationOutput("\n@():", MAXON_FUNCTIONNAME);
	ApplicationOutput("\tAll OCIO transform names: @", doc->GetOCIOColorSpaceNames());
	ApplicationOutput("\tOCIO render transform names: @", doc->GetOCIORenderingColorSpaceNames());
	ApplicationOutput("\tOCIO view transform names: @", doc->GetOCIOViewTransformNames());
	ApplicationOutput("\tOCIO display transform names: @", doc->GetOCIODisplayColorSpaceNames());

	// Since an OCIO configuration file can contain any combination of color spaces and transforms,
	// the description IDs for the render space, display space, view transform, and view thumbnail
	// transform parameters must be dynamic description IDs.
	for (maxon::CString s : doc->GetOCIORenderingColorSpaceNames())
	{
		ApplicationOutput("\t\tThe render transform label '@' corresponds to the id '@'.", 
			s, doc->GetColorSpaceIdFromName(DOCUMENT_OCIO_RENDER_COLORSPACE, s));
	}

	// So, this would be the pattern for example to set the render space to a specific space name,
	// as for example the 'ACES2065 - 1' render space contained in the default OCIO 2.0 config file.
	// The method GetColorSpaceIdFromName() will return NOTOK to indicate unknown space labels.
	maxon::Int32 id = doc->GetColorSpaceIdFromName(DOCUMENT_OCIO_RENDER_COLORSPACE, "ACES2065-1"_cs);
	if (id == NOTOK)
		return maxon::IllegalArgumentError(MAXON_SOURCE_LOCATION,
			"OCIO configuration does not contain a 'ACES2065 - 1' render space."_s);

	doc->SetParameter(DescID(DOCUMENT_OCIO_RENDER_COLORSPACE), GeData(id), DESCFLAGS_SET::NONE);
	doc->UpdateOcioColorSpaces();
	EventAdd();

	return maxon::OK;
}
//! [GetSetColorManagementSettings]

//! [GetSetColorValuesInOcioDocuments]
maxon::Result<void> GetSetColorValuesInOcioDocuments(BaseDocument* doc)
{
	iferr_scope;

	if (MAXON_UNLIKELY(!doc))
		return maxon::NullptrError(MAXON_SOURCE_LOCATION, "Invalid document pointer"_s);

	// Ensure that the document is in OCIO color management mode.
	BaseContainer* bc = doc->GetDataInstance();
	if (bc->GetInt32(DOCUMENT_COLOR_MANAGEMENT) != DOCUMENT_COLOR_MANAGEMENT_OCIO)
	{
		bc->SetInt32(DOCUMENT_COLOR_MANAGEMENT, DOCUMENT_COLOR_MANAGEMENT_OCIO);
		doc->UpdateOcioColorSpaces();
		EventAdd();
	}

	ApplicationOutput("\n@():", MAXON_FUNCTIONNAME);
	
	// Get then OCIO space names for #ApplicationOutput calls invoked below.
	maxon::CString renderSpace, viewTransform, _;
	doc->GetActiveOcioColorSpacesNames(renderSpace, _, viewTransform, _);

	// When a document is in OCIO mode, all color parameters of scene elements are implicitly in 
	// Render space. When for example the default color for objects is being retrieved from a 
	// document, the value is expressed in Render space.
	GeData data;
	doc->GetParameter(DescID(DOCUMENT_DEFAULTMATERIAL_COLOR), data, DESCFLAGS_GET::NONE);
	const Vector defaultColor = data.GetVector();
	ApplicationOutput("\tdefaultColor(@): @", renderSpace, defaultColor);

	// Unlike in the color chooser GUI, there is no direct way to read color values in any other 
	// space, as an sRGB value for example. Such conversions must be carried out with the type 
	// OcioConverter manually. The argument #doc to OcioConverter::Init must be a document in OCIO 
	// mode.
	const OcioConverter* converter = OcioConverter::Init(doc) iferr_return;
	if (!converter)
		return maxon::NullptrError(MAXON_SOURCE_LOCATION, "Could not init OCIO converter for document."_s);

	// Transform #defaultColor from Render space to sRGB space.
	const Vector colorSrgb = converter->TransformColor(
		defaultColor, COLORSPACETRANSFORMATION::OCIO_RENDERING_TO_SRGB);
	ApplicationOutput("\tdefaultColor(@): @", "sRGB"_s, colorSrgb);
	
	// Allocate three materials to write color values to.
	BaseMaterial* m1 = BaseMaterial::Alloc(Mmaterial);
	BaseMaterial* m2 = BaseMaterial::Alloc(Mmaterial);
	BaseMaterial* m3 = BaseMaterial::Alloc(Mmaterial);
	if (MAXON_UNLIKELY(!(m1 && m2 && m3)))
		return maxon::OutOfMemoryError(MAXON_SOURCE_LOCATION, "Could not allocate materials."_s);

	// Analogously, when color values are written, they are interpreted as Render space values by
	// default, and all other input spaces must be converted first.
	const Vector color { 1, 0, 0 };
	const DescID colorChannel (MATERIAL_COLOR_COLOR);

	// Write the color channel color of m1 as (1, 0, 0) in Render space.
	m1->SetParameter(colorChannel, GeData(color), DESCFLAGS_SET::NONE);

	// Write the color channel color of m2 as (1, 0, 0) in sRGB space.
	Vector fromSrgb = converter->TransformColor(color, COLORSPACETRANSFORMATION::OCIO_SRGB_TO_RENDERING);
	m2->SetParameter(colorChannel, GeData(fromSrgb), DESCFLAGS_SET::NONE);

	// Write the color channel color of m3 as (1, 0, 0) in View space, i.e., what the color chooser 
	// does in its Display tab.
	Vector fromView = converter->TransformColor(color, COLORSPACETRANSFORMATION::OCIO_VIEW_TO_RENDERING);
	m3->SetParameter(colorChannel, GeData(fromView), DESCFLAGS_SET::NONE);

	ApplicationOutput("\tMATERIAL_COLOR_COLOR(@): @", renderSpace, color);
	ApplicationOutput("\tMATERIAL_COLOR_COLOR(@): @", "sRGB"_s, fromSrgb);
	ApplicationOutput("\tMATERIAL_COLOR_COLOR(@): @", viewTransform, fromView);

	// Set the name of and insert all three materials.
	m1->SetName("Material (Render Space)"_s);
	m2->SetName("Material (sRGB)"_s);
	m3->SetName("Material (View Transform)"_s);
	doc->InsertMaterial(m1);
	doc->InsertMaterial(m2);
	doc->InsertMaterial(m3);
	EventAdd();

	return maxon::OK;
}
//! [GetSetColorValuesInOcioDocuments]

//! [GetSetBitmapOcioProfiles]
maxon::Result<void> GetSetBitmapOcioProfiles(BaseDocument* doc)
{
	BaseBitmap* clone;

	iferr_scope;
	finally { BaseBitmap::Free(clone); };

	if (MAXON_UNLIKELY(!doc))
		return maxon::NullptrError(MAXON_SOURCE_LOCATION, "Invalid document pointer"_s);

	// Ensure that the document is in OCIO color management mode.
	BaseContainer* bc = doc->GetDataInstance();
	if (bc->GetInt32(DOCUMENT_COLOR_MANAGEMENT) != DOCUMENT_COLOR_MANAGEMENT_OCIO)
	{
		bc->SetInt32(DOCUMENT_COLOR_MANAGEMENT, DOCUMENT_COLOR_MANAGEMENT_OCIO);
		doc->UpdateOcioColorSpaces();
		EventAdd();
	}

	// Get the Asset API user preferences repository to get the "HDR004.hdr" texture asset in 
	// Textures/HDR/Legacy and retrieve its URL.
	maxon::AssetRepositoryRef repository = maxon::AssetInterface::GetUserPrefsRepository();
	if (MAXON_UNLIKELY(!repository))
		return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Could not retrieve user repository."_s);

	const maxon::Id assetId("file_9748feafc2c00be8");
	const maxon::AssetDescription asset = repository.FindLatestAsset(
		maxon::Id(), assetId, maxon::Id(), maxon::ASSET_FIND_MODE::LATEST) iferr_return;
	const maxon::Url textureUrl = maxon::AssetInterface::GetAssetUrl(asset, true) iferr_return;

	// --- Start of Image API related code ----------------------------------------------------------
	
	// Allocate a bitmap and load the texture asset.
	AutoAlloc<BaseBitmap> bitmap;
	if (MAXON_UNLIKELY(!bitmap))
		return maxon::OutOfMemoryError(MAXON_SOURCE_LOCATION, "Could not allocate bitmap."_s);

	if (bitmap->Init(MaxonConvert(textureUrl)) != IMAGERESULT::OK)
		return maxon::IoError(MAXON_SOURCE_LOCATION, textureUrl, "Could not load image file."_s);

	// Clone the loaded bitmap.
	clone = bitmap->GetClone();
	if (MAXON_UNLIKELY(!clone))
		return maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Could not clone bitmap."_s);

	// Get the OCIO Render space color profile associated with the bitmap.
	const ColorProfile* const profile = clone->GetColorProfile(BaseBitmap::COLORPROFILE_INDEX_RENDERSPACE);

	// Null the Display color space and the View Transform by overwriting them with the Render space
	// profile, the data will be displayed "raw".
	clone->SetColorProfile(profile, BaseBitmap::COLORPROFILE_INDEX_DISPLAYSPACE);
	clone->SetColorProfile(profile, BaseBitmap::COLORPROFILE_INDEX_VIEW_TRANSFORM);

	// Display both bitmaps in the picture viewer.
	ShowBitmap(bitmap);
	ShowBitmap(clone);

	return maxon::OK;
}
//! [GetSetBitmapOcioProfiles]