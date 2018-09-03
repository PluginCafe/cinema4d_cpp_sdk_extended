// ------------------------------------------------------------------------
/// This file contains the declaration of a published object.
/// This published object is used to access a specific implementation of
/// MediaOutputUrlInterface.
// ------------------------------------------------------------------------

#ifndef MAXONSDK_IMAGE_EXPORT_H__
#define MAXONSDK_IMAGE_EXPORT_H__

// MAXON API header file
#include "maxon/mediasession_image_export.h"

namespace maxon
{
namespace ImageSaverClasses
{
	// ------------------------------------------------------------------------
	/// The published object "MaxonSDKImage" gives access to an implementation of MediaOutputUrlInterface
	// ------------------------------------------------------------------------
	MAXON_DECLARATION(ImageSaverClasses::EntryType, MaxonSDKImage, "net.maxonexample.mediasession.image.export");
}
}

#endif
