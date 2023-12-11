#include "c4d_basebitmap.h"
#include "c4d_basematerial.h"
#include "c4d_general.h"
#include "c4d_shader.h"
#include "c4d_videopost.h"
#include "lib_description.h"
#include "lib_scene_color_converter.h"

#include "maxon/gfx_image.h"

#include "maxon/assets.h"
#include "maxon/gfx_image_colorprofile.h"
#include "maxon/gfx_image_colorspaces.h"
#include "maxon/gfx_image_pixelformats.h"
#include "maxon/iostreams.h"

#include "mmaterial.h"
#include "vpocioawarerenderer.h"

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

	// Allocate and initialize the converter, currently it is not possible to retrieve the linear 
	// and non-linear input color space names "input-low" and "input-high" that are associated
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

	// When converting a whole document, one should mark it as already converted when the conversion
	// did succeed.
	doc->SetParameter(ConstDescID(DescLevel(DOCUMENT_COLOR_MANAGEMENT_OCIO_CONVERTED)), GeData(true), DESCFLAGS_SET::NONE);

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
	// convenient manner to copy all color management settings from one document to another. This
	// does NOT entail a conversion of the colors of scene elements to OCIO, see 
	// ConvertSceneOrElements() for that.
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
	// between the color spaces associated with the OCIO configuration of a document, for its 
	// initialization to succeed, the document must be in OCIO color management mode.
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
	// will be finally shown on the screen of a user. With the default OCIO settings of Cinema 4D 
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

//! [ConvertOcioColorsArbitrarily]
maxon::Result<void> ConvertOcioColorsArbitrarily(BaseDocument* doc)
{
	iferr_scope;

	if (MAXON_UNLIKELY(!doc))
		return maxon::NullptrError(MAXON_SOURCE_LOCATION, "Invalid document pointer"_s);

	// Make sure that #doc is in OCIO color management mode.
	BaseContainer* bc = doc->GetDataInstance();
	if (bc->GetInt32(DOCUMENT_COLOR_MANAGEMENT) != DOCUMENT_COLOR_MANAGEMENT_OCIO)
	{
		bc->SetInt32(DOCUMENT_COLOR_MANAGEMENT, DOCUMENT_COLOR_MANAGEMENT_OCIO);
		doc->UpdateOcioColorSpaces();
		EventAdd();
	}

	// Load the "sRGB2014" ICC profile located next to this file.
	const maxon::Url directory = maxon::Url(maxon::String(MAXON_FILE)).GetDirectory();
	const maxon::Url sRgb2014File = (directory + "sRGB2014.icc"_s) iferr_return;
	if (sRgb2014File.IoDetect() == maxon::IODETECT::NONEXISTENT)
		return maxon::IoError(
			MAXON_SOURCE_LOCATION, sRgb2014File, "Could not access sRgb2014File.icc profile."_s);

	const maxon::ColorProfile srgb2014Profile = maxon::ColorProfileInterface::OpenProfileFromFile(
		sRgb2014File) iferr_return;

	// Get the profile for the builtin linear RGB space. This profile is required because 
	// conversions with OCIO profiles are only supported between profiles that originate from the 
	// same OCIO config and the profiles for the builtin linear or non-linear sRGB space (given that
	// the OCIO config file defines the sRGB space; which is the case for the Cinema 4D profiles). 
	// For all other conversions, an intermediate step must be taken, e.g., in our case:
	//
	//		sRGB2014 -> lin-RGB -> Render space
	//		Render space -> lin-RGB -> sRGB2014
	const maxon::ColorProfile linearRgbSpaceProfile = maxon::ColorSpaces::RGBspace(
		).GetDefaultLinearColorProfile();

	// Get the profile and name for the active Render space in #doc.
	maxon::ColorProfile renderSpaceProfile, _;
	doc->GetOcioProfiles(renderSpaceProfile, _, _, _);
	maxon::CString renderSpaceName, __;
	doc->GetActiveOcioColorSpacesNames(renderSpaceName, __, __, __);

	// Define the pixel format to operate with and check if it is supported by the profiles.
	const maxon::PixelFormat rgbFormat = maxon::PixelFormats::RGB::F32();
	if (!(srgb2014Profile.CheckCompatiblePixelFormat(rgbFormat) || 
				linearRgbSpaceProfile.CheckCompatiblePixelFormat(rgbFormat) ||
				renderSpaceProfile.CheckCompatiblePixelFormat(rgbFormat)))
		return maxon::UnexpectedError(
			MAXON_SOURCE_LOCATION, "Profile conversion path does not support RGB::F32 pixel format"_s);

	// Construct two converters, one for converting from the ICC profile to the intermediate linear
	// RGB space, and one for converting from that intermediate space to the Render space.
	const maxon::ColorProfileConvert preConverter = maxon::ColorProfileConvertInterface::Init(
		rgbFormat, srgb2014Profile, rgbFormat, linearRgbSpaceProfile,
		maxon::COLORCONVERSIONINTENT::ABSOLUTE_COLORIMETRIC,
		maxon::COLORCONVERSIONFLAGS::NONE) iferr_return;
	const maxon::ColorProfileConvert ocioConverter = maxon::ColorProfileConvertInterface::Init(
		rgbFormat, linearRgbSpaceProfile, rgbFormat, renderSpaceProfile,
		maxon::COLORCONVERSIONINTENT::ABSOLUTE_COLORIMETRIC,
		maxon::COLORCONVERSIONFLAGS::NONE) iferr_return;

	// Initialize the data to convert, an input and an output buffer, as well as two buffer 
	// handlers for them. This could also be done by using three buffers (in, intermediate, out) and
	// four handlers (const in, const intermediate, mutable intermediate, mutable out) to avoid 
	// having to copy data from the output buffer to the input buffer after the first conversion.
	maxon::Color32 colorData ( 1, 0, 0 );
	maxon::Color32 inputBuffer (colorData);
	maxon::Color32 outputBuffer (0, 0, 0);

	maxon::ImageConstBuffer inputBufferHandler = maxon::ImageConstBuffer(
		(const maxon::Pix*)&inputBuffer.r, maxon::PixelFormats::RGB::F32());
	maxon::ImageMutableBuffer outputBufferHandler = maxon::ImageMutableBuffer(
		(maxon::Pix*)&outputBuffer.r, maxon::PixelFormats::RGB::F32());

	// Carry out the conversion sRGB2014.icc -> linear-RGB -> Render space for the data (1, 0, 0).
	preConverter.Convert(inputBufferHandler, outputBufferHandler, 1) iferr_return;
	ApplicationOutput("\n@():", MAXON_FUNCTIONNAME);
	ApplicationOutput("\tIn (@): @, Out (@): @", 
										srgb2014Profile, colorData, linearRgbSpaceProfile, outputBuffer);

	inputBuffer = outputBuffer;
	ocioConverter.Convert(inputBufferHandler, outputBufferHandler, 1) iferr_return;
	ApplicationOutput("\tIn (@): @, Out (@): @",
										srgb2014Profile, colorData, renderSpaceName, outputBuffer);

	return maxon::OK;
}
//! [ConvertOcioColorsArbitrarily]

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
	ApplicationOutput("\tAll OCIO transform names: @", doc->GetOcioColorSpaceNames());
	ApplicationOutput("\tOCIO render transform names: @", doc->GetOcioRenderingColorSpaceNames());
	ApplicationOutput("\tOCIO view transform names: @", doc->GetOcioViewTransformNames());
	ApplicationOutput("\tOCIO display transform names: @", doc->GetOcioDisplayColorSpaceNames());

	// Since an OCIO configuration file can contain any combination of color spaces and transforms,
	// the description IDs for the render space, display space, view transform, and view thumbnail
	// transform parameters must be dynamic IDs.
	for (maxon::CString s : doc->GetOcioRenderingColorSpaceNames())
	{
		ApplicationOutput("\t\tThe render transform label '@' corresponds to the ID '@'.", 
			s, doc->GetColorSpaceIdFromName(DOCUMENT_OCIO_RENDER_COLORSPACE, s));
	}

	// So, this would be the pattern to set for example the render space to a specific space name,
	// here the 'ACES2065 - 1' render space contained in the default OCIO 2.0 config file.
	// The method GetColorSpaceIdFromName() will return #NOTOK to indicate unknown space labels.
	maxon::Int32 id = doc->GetColorSpaceIdFromName(DOCUMENT_OCIO_RENDER_COLORSPACE, "ACES2065-1"_cs);
	if (id == NOTOK)
		return maxon::IllegalArgumentError(MAXON_SOURCE_LOCATION,
			"OCIO configuration does not contain a 'ACES2065 - 1' render space."_s);

	doc->SetParameter(ConstDescID(DescLevel(DOCUMENT_OCIO_RENDER_COLORSPACE)), GeData(id), DESCFLAGS_SET::NONE);
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
	
	// Get the names of the active OCIO spaces in #doc for #ApplicationOutput calls invoked below.
	maxon::CString renderSpace, viewTransform, _;
	doc->GetActiveOcioColorSpacesNames(renderSpace, _, viewTransform, _);

	// When a document is in OCIO mode, all color parameters of scene elements are implicitly in 
	// Render space. When for example the default color for objects is being retrieved from a 
	// document, the value is expressed in Render space.
	GeData data;
	doc->GetParameter(ConstDescID(DescLevel(DOCUMENT_DEFAULTMATERIAL_COLOR)), data, DESCFLAGS_GET::NONE);
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
	const DescID colorChannel = ConstDescID(DescLevel(MATERIAL_COLOR_COLOR));

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
	// Textures/HDR/Legacy and retrieve its URL. It is important to use here an HDR image (the
	// dynamic range, not the necessarily the format), as the view transform is disabled for SDR
	// content in the Picture Viewer (as of 2023.1, might change in future releases).
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


Bool OcioAwareRenderer::Init(GeListNode* node, Bool isCloneInit)
{
	BaseList2D* const item = static_cast<BaseList2D*>(node);
	if (!item)
		return false;

	BaseContainer* data = item->GetDataInstance();

	data->SetVector(ID_RENDER_COLOR, Vector(.75, .25, .0));
	data->SetBool(ID_PRETRANSFORM_OCIO_OUTPUT, true);
	data->SetBool(ID_OVERWRITE_OCIO_DISPLAY_SPACE, false);
	data->SetBool(ID_OVERWRITE_OCIO_VIEW_TRANSFORM, false);

	return true; 
}

//! [OcioAwareRenderer]
void OcioAwareRenderer::GetColorProfileInfo(
	BaseVideoPost* node, VideoPostStruct* vps, ColorProfileInfo& info)
{
	if (!node)
		return;

	// GetColorProfileInfo allows us to react to or modify the color profiles of an upcoming OCIO 
	// rendering.Here we use it to null the display and view transform by overwriting them with the
	// render transform when so indicated, causing the rendered image to be treated as "raw".
	BaseContainer* data = node->GetDataInstance();
	if (data->GetBool(ID_OVERWRITE_OCIO_DISPLAY_SPACE))
		info.displayColorSpace = info.renderingColorSpace;
	if (data->GetBool(ID_OVERWRITE_OCIO_VIEW_TRANSFORM))
		info.viewColorSpace = info.renderingColorSpace;
}

RENDERRESULT OcioAwareRenderer::Execute(BaseVideoPost* node, VideoPostStruct* vps)
{
	iferr_scope_handler
	{
		return RENDERRESULT::FAILED;
	};

	// Bail when important data is not accessible, a rendering error did occur, or the user did stop 
	// the renderer.
	if (!vps || !vps->doc || !vps->doc->GetDataInstance() || !vps->render)
		return RENDERRESULT::OUTOFMEMORY;

	if (*vps->error != RENDERRESULT::OK)
		return RENDERRESULT::FAILED;

	if (vps->thread && vps->thread->TestBreak())
		return RENDERRESULT::USERBREAK;

	// Skip all calls except for a full frame being finished.
	if (vps->vp == VIDEOPOSTCALL::FRAME || vps->open)
		return RENDERRESULT::OK;

	// Get the RGBA buffer of the rendering.
	VPBuffer* const rgbaBuffer = vps->render->GetBuffer(VPBUFFER_RGBA, NOTOK);
	if (!rgbaBuffer)
		return RENDERRESULT::OUTOFMEMORY;

	if (rgbaBuffer->GetBt() != 32)
		return RENDERRESULT::FAILED;

	// Get the color to "render" from the VideoPost node.
	BaseContainer* nodeData = node->GetDataInstance();
	Vector color = nodeData->GetVector(ID_RENDER_COLOR);

	// By default, just as in other places of the API, all color computations are implicitly in
	// Render space once a document is in OCIO mode. When a video post plugin is not meant to operate
	// in that space, all outputs must be transformed manually.
	BaseContainer* docData = vps->doc->GetDataInstance();
	if ((docData->GetInt32(DOCUMENT_COLOR_MANAGEMENT) == DOCUMENT_COLOR_MANAGEMENT_OCIO) &&
			nodeData->GetBool(ID_PRETRANSFORM_OCIO_OUTPUT))
	{
		const OcioConverter* converter = OcioConverter::Init(vps->doc) iferr_return;
		if (!converter)
			return RENDERRESULT::OUTOFMEMORY;

		// Convert the color chosen by the user from sRGB to Render space. I.e., the chosen color is 
		// always interpreted as an sRGB value which is then converted to and written as a Render space
		// value in the output buffer.
		color = converter->TransformColor(color, COLORSPACETRANSFORMATION::OCIO_SRGB_TO_RENDERING);
	}

	// Fill the whole buffer with the color.
	const Int32 width = rgbaBuffer->GetBw();
	const Int32 height = rgbaBuffer->GetBh();
	const Int32 bytesPerPixel = rgbaBuffer->GetCpp();

	Float32* lineBuffer = NewMemClear(Float32, bytesPerPixel * width).GetPointer();
	for (Int32 ix = 0; ix < width; ix++)
	{
		lineBuffer[ix*bytesPerPixel+0] = (Float32)color.x;
		lineBuffer[ix*bytesPerPixel+1] = (Float32)color.y;
		lineBuffer[ix*bytesPerPixel+2] = (Float32)color.z;
	}
	for (Int32 iy = 0; iy < height; iy++)
		rgbaBuffer->SetLine(0, iy, width, lineBuffer, 32, false);
	DeleteMem(lineBuffer);

	return RENDERRESULT::OK;
}
//! [OcioAwareRenderer]