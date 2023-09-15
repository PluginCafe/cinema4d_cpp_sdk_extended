/*
	Image API Examples - OCIO
	2022, (C) MAXON Computer GmbH

	Author: Ferdinand Hoppe
	Date: 02/11/2022
	SDK: 2023.100

	Demonstrates how to interact with the OCIO settings and color spaces in an OCIO document.
*/

#ifndef EXAMPLES_OCIO_H__
#define EXAMPLES_OCIO_H__

#include "c4d_basedocument.h"
#include "maxon/apibase.h"
#include "c4d_symbols.h"

/// @brief Convert all colors contained in a document or contained in a set of elements in a 
/// document to a new OCIO Render space.
maxon::Result<void> ConvertSceneOrElements(BaseDocument* doc);

/// @brief Copy all color management settings from one document to another.
/// @note This does not entail any scene element color space conversions, one must instead use 
/// #SceneColorConverter as demonstrated in #ConvertSceneOrElements for this.
maxon::Result<void> CopyColorManagementSettings(BaseDocument* doc);

/// @brief Read and write the OCIO color management settings stored in a document.
maxon::Result<void> GetSetColorManagementSettings(BaseDocument* doc);

/// @brief Convert colors along OCIO conversion paths defined by the OCIO color spaces associated 
/// with a document.
maxon::Result<void> ConvertOcioColors(BaseDocument* doc);

/// @brief Convert colors from and to OCIO color spaces using arbitrary in- and output spaces.
maxon::Result<void> ConvertOcioColorsArbitrarily(BaseDocument* doc);

/// @brief Read and write color parameters in an OCIO document either in the native Render space of
/// the document or other spaces.
maxon::Result<void> GetSetColorValuesInOcioDocuments(BaseDocument* doc);

/// @brief Read and write the OCIO color spaces associated with #BaseBitmap instances to change or 
/// nullify parts of an OCIO conversion path for them.
/// @etails This makes it for example possible to disable the Display and View transform for a 
/// singular bitmap when displayed in the Picture Viewer.
maxon::Result<void> GetSetBitmapOcioProfiles(BaseDocument* doc);

/// @brief Realizes a renderer which manipulates the OCIO profiles of an upcoming rendering.
class OcioAwareRenderer : public VideoPostData
{
	INSTANCEOF(OcioAwareRenderer, VideoPostData)

public:
	static NodeData* Alloc() { return NewObjClear(OcioAwareRenderer); }
	virtual Bool Init(GeListNode* node, Bool isCloneInit);

	/// @brief Called by Cinema 4D to let a renderer modify the OCIO profiles of an upcoming rendering.
	virtual void GetColorProfileInfo(BaseVideoPost* node, VideoPostStruct* vps, ColorProfileInfo& info);
	/// @brief Called by Cinema 4D to execute the renderer.
	virtual RENDERRESULT Execute(BaseVideoPost* node, VideoPostStruct* vps);
};
#endif // EXAMPLES_OCIO_H__