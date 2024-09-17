// ------------------------------------------------------------------------
/// This file contains the declaration of a published object.
/// This published object is used to access a specific implementation of
/// MediaOutputUrlInterface.
// ------------------------------------------------------------------------

#ifndef MEDIAOUTPUT_DECLARATIONS_H__
#define MEDIAOUTPUT_DECLARATIONS_H__

// Maxon API header file
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

#endif // MEDIAOUTPUT_DECLARATIONS_H__
