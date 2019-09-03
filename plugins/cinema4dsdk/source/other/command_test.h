#ifndef COMMAND_TEST_H__
#define COMMAND_TEST_H__

#include "maxon/apibase.h"
#include "maxon/commandbase.h"

namespace maxon
{

enum class POLYLINE_DRAW
{
	LINE = 0,
	BOX,
} MAXON_ENUM_LIST(POLYLINE_DRAW);

namespace CommandClasses
{
	MAXON_DECLARATION(CommandClasses::EntryType, PAINT, "net.maxonexample.command.paint");
}

namespace COMMAND
{
	namespace POLYLINE
	{
		MAXON_ATTRIBUTE(BaseArray<Vector>, POSITIONS, "net.maxonexample.command.paint.positions");
		MAXON_ATTRIBUTE(Vector, START, "net.maxonexample.command.paint.start", RESOURCE_DEFAULT(Vector()));
		MAXON_ATTRIBUTE(Vector, END, "net.maxonexample.command.paint.end", RESOURCE_DEFAULT(Vector(200.0, 200.0, 0.0)));
		MAXON_ATTRIBUTE(POLYLINE_DRAW, DRAW, "net.maxonexample.command.paint.draw");
	}
}

#include "command_test1.hxx"
#include "command_test2.hxx"

} // namespace maxon

#endif // COMMAND_TEST_H__
