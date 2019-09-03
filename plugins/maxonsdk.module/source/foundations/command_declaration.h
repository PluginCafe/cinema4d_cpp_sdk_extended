// ------------------------------------------------------------------------
/// This file contains the declaration of several published objects.
/// These objects declare settings, a default context and two command implementations.
// ------------------------------------------------------------------------

#ifndef COMMAND_DECLARATION_H__
#define COMMAND_DECLARATION_H__

// MAXON API header file
#include "maxon/commandbase.h"


namespace MEANSETTINGS
{
	// ------------------------------------------------------------------------
	/// An array of maxon::Float values.
	// ------------------------------------------------------------------------
	MAXON_ATTRIBUTE(maxon::BaseArray<maxon::Float>, VALUES, "net.maxonexample.mean.values");
	// ------------------------------------------------------------------------
	/// The result mean value.
	// ------------------------------------------------------------------------
	MAXON_ATTRIBUTE(maxon::Float, RESULT, "net.maxonexample.mean.result");
}

namespace maxon
{
namespace CommandDataClasses
{
	// ------------------------------------------------------------------------
	/// An example data object.
	// ------------------------------------------------------------------------
	MAXON_DECLARATION(maxon::CommandDataClasses::EntryType, MAXONSDKDATA, "net.maxonexample.commanddata.example");
}

namespace CommandClasses
{
	// ------------------------------------------------------------------------
	/// Command to calculate the average of the given values.
	// ------------------------------------------------------------------------
	MAXON_DECLARATION(maxon::CommandClasses::EntryType, MAXONSDKMEAN_AVERAGE, "net.maxonexample.command.mean.average");
	// ------------------------------------------------------------------------
	/// Command to calculate the median of the given values.
	// ------------------------------------------------------------------------
	MAXON_DECLARATION(maxon::CommandClasses::EntryType, MAXONSDKMEAN_MEDIAN, "net.maxonexample.command.mean.median");
}
}

#include "command_declaration1.hxx"
#include "command_declaration2.hxx"

#endif // COMMAND_DECLARATION_H__


