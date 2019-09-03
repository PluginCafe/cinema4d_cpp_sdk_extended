#ifndef CUSTOMDATATAGS_H__
#define CUSTOMDATATAGS_H__

#include "lib_customdatatag.h"

#include "maxon/apibase.h"
#include "maxon/datatype.h"
#include "maxon/mesh_attribute_base.h"
#include "maxon/vector4d.h"

static const Int32 ID_CUSTOMDATA_TAG_VC = 431000189;
static const Int32 ID_CUSTOMDATA_TAG_FL = 431000190;
static const Int32 ID_CUSTOMDATA_TAG_COMMAND = 1039842;

static const Int32 ID_POLYOBJ = 1052873;
static const Int32 ID_POINT_INDEX_TAG = 1052874;

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

// This class provides a way to store in a custom data tag an int32 index for each component.
// Internally the modeling system uses some bits to define some special behaviours.
// For this reason we shift _index by 4 bytes using _privateBuffer.
class PointIndex
{
public:

	PointIndex() = default;

	MAXON_IMPLICIT PointIndex(Int32 index) : _privateBuffer(0), _index(index)  {	}

	PointIndex& operator =(const PointIndex& src)
	{
		_index = src._index;
		_privateBuffer = src._privateBuffer;
		return *this;
	}

	inline Bool operator ==(const PointIndex& other) const
	{
		return _index == other._index;
	}

	inline Bool operator <(const PointIndex& other) const
	{
		return _index < other._index;
	}

	MAXON_OPERATOR_COMPARISON(PointIndex);

	inline String ToString(const FormatStatement* formatStatement) const
	{
		return String::IntToString((Int32)_index);
	}

	Int32 _privateBuffer = 0;
	Int32 _index = NOTOK; // this is the point index.
};

// Declare first the basic class datatype
MAXON_DATATYPE(PointIndex, "net.maxonexample.meshattribute.pointindebase");

// Declare then a mesh attribute which contains the point index
// so the modeling system knows how to deal with it.
MAXON_MESHATTRIBUTE(PointIndex, POINTINDEX);
MAXON_DATATYPE(POINTINDEX_MESHATTRIBUTE, "net.maxonexample.meshattribute.pointindexattribute");

// Declare the custom data tag implementation under CustomDataTagClasses register
namespace CustomDataTagClasses
{
	MAXON_DECLARATION(CustomDataTagClasses::EntryType, POINTINDEX, "net.maxonexample.mesh_misc.customdatatagclass.pointindex");
}

} // namespace maxon

Bool RegisterCustomDataTagDescription();
Bool RegisterCustomDataTagCommand();
Bool RegisterPolyExample();

#endif // CUSTOMDATATAGS_H__
