// ------------------------------------------------------------------------
/// This file contains the declaration of a published object and an attribute.
/// The published object is used to access a specific implementation of
/// StreamConversionInterface. The attribute is used to configure an instance
/// of that interface.
// ------------------------------------------------------------------------

#ifndef STREAMCONVERSION_DECLARATIONS_H__
#define STREAMCONVERSION_DECLARATIONS_H__

// Maxon API header file
#include "maxon/streamconversion.h"

namespace maxon
{
namespace MAXONSDK_CAESAR_CIPHER_OPTIONS
{
	// ------------------------------------------------------------------------
	/// Shift value for Caesar cipher
	// ------------------------------------------------------------------------
	MAXON_ATTRIBUTE(Int32, SHIFT, "net.maxonexample.streamconversion.caesar.shift");
}

namespace StreamConversions
{
	// ------------------------------------------------------------------------
	/// The published object "MaxonSDKCaesar" gives access to an implementation 
	/// of StreamConversionInterface.
	// ------------------------------------------------------------------------
	MAXON_DECLARATION(StreamConversionFactory, MaxonSDKCaesarCipher, "net.maxonexample.streamconversion.caesar");
}

// includes needed for MAXON_ATTRIBUTE
#include "streamconversion_declarations1.hxx"
#include "streamconversion_declarations2.hxx"
}
#endif // STREAMCONVERSION_DECLARATIONS_H__

