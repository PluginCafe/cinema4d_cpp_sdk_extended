#ifndef CUSTOMDATA_TAGS_H__
#define CUSTOMDATA_TAGS_H__

#include "lib_customdatatag.h"

#include "maxon/apibase.h"
#include "maxon/datatype.h"
#include "maxon/mesh_attribute_base.h"
#include "maxon/vector4d.h"

static const Int32 ID_CUSTOMDATA_TAG_VC = 431000189;
static const Int32 ID_CUSTOMDATA_TAG_FL = 431000190;
static const Int32 ID_CUSTOMDATA_TAG_COMMAND = 1039842;

namespace maxon
{

MAXON_MESHATTRIBUTE(ColorA32, VERTEXCOLOR);
MAXON_DATATYPE(VERTEXCOLOR_MESHATTRIBUTE, "net.maxonexample.meshattribute.vertexcolor");

MAXON_MESHATTRIBUTE(Float, FLOATTYPE);
MAXON_DATATYPE(FLOATTYPE_MESHATTRIBUTE, "net.maxonexample.meshattribute.floattype");

namespace CustomDataTagClasses
{
	MAXON_DECLARATION(CustomDataTagClasses::EntryType, VC, "net.maxonexample.mesh_misc.customdatatagclass.vc");
	MAXON_DECLARATION(CustomDataTagClasses::EntryType, FLOAT, "net.maxonexample.mesh_misc.customdatatagclass.float");
}
	
namespace CustomDataTagDisplayClasses
{
	MAXON_DECLARATION(CustomDataTagDisplayClasses::EntryType, VCDISPLAY, "net.maxonexample.mesh_misc.customdatatagdisplay.vertexcolor");
	MAXON_DECLARATION(CustomDataTagDisplayClasses::EntryType, FLOATDISPLAY, "net.maxonexample.mesh_misc.customdatatagdisplay.float");
}

} // namespace maxon

Bool RegisterCustomDataTagDescription();
Bool RegisterCustomDataTagCommand();

#endif // CUSTOMDATA_TAGS_H__
