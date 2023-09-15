// ------------------------------------------------------------------------
/// This file contains the declaration of a published object.
/// This published object is used to access a specific implementation of
/// BlendFunctionInterface.
// ------------------------------------------------------------------------

#ifndef BLENDFUNCTION_DECLARATIONS_H__
#define BLENDFUNCTION_DECLARATIONS_H__

// MAXON API header file
#include "maxon/blend_function.h"

namespace maxon
{
namespace BlendFunctions
{
	// ------------------------------------------------------------------------
	/// The published object "MaxonSDKStep" gives access to an implementation 
	/// of BlendFunctionInterface.
	// ------------------------------------------------------------------------
	MAXON_DECLARATION(BlendFunctionRef, MaxonSDKStep, "net.maxonexample.blendfunction.step");
}
}

#endif // BLENDFUNCTION_DECLARATIONS_H__
