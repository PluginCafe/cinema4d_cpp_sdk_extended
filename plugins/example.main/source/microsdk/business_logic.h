#ifndef BUSINESS_LOGIC_H__
#define BUSINESS_LOGIC_H__

// MAXON API header files
#include "maxon/errorbase.h"

// forward declaration to avoid including the actual header file
class BaseObject;

namespace microsdk
{
//-------------------------------------------------------------------------------------
/// Checks if the configuration variable "g_executeMicroExample" is set to true. If so,
/// a new cube object is created and inserted into the currently active BaseDocument.
//-------------------------------------------------------------------------------------
void ExecuteMicroExampleCode();

//----------------------------------------------------------------------------------------
/// Static function that creates a cube object.
/// @return												The created cube object or an error.
//----------------------------------------------------------------------------------------
maxon::Result<BaseObject*> MakeCube();
}

#endif // BUSINESS_LOGIC_H__
