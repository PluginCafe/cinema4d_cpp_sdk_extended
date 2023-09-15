#ifndef HYBRIDDATATYPE_H__
#define HYBRIDDATATYPE_H__


#include "maxon/apibase.h"
#include "maxon/interfacebase.h"
#include "maxon/lib_math.h"
#include "maxon/datatype.h"
#include "maxon/dataserialize.h"

#include "c4d_customdatatype.h"
#include "c4d_baselist.h"

#define HYBRIDDATATYPE_ID 1053472

namespace maxonsdk
{

// We model a dependency for the point in time where HybridDataType is registered.
// This way we ensure that our node space registration is performed afterwards.
MAXON_DEPENDENCY(HybridDataTypeRegistration);

// This data type is a bit special: it's part of the 'classical' and 'new' APIs of Cinema4D.
// Its purpose is to show how custom data types can be used in the context of nodes.
class HybridDataType : public iCustomDataType<HybridDataType, HYBRIDDATATYPE_ID>
{
public:

	HybridDataType() = default;

	explicit HybridDataType(Float value);

	Bool operator ==(const HybridDataType& b) const
	{
		return _value == b._value;
	}

	Bool operator <(const HybridDataType& b) const
	{
		return _value < b._value;
	}

	maxon::HashInt GetHashCode() const
	{
		return maxon::DefaultCompare::GetHashCode(_value);
	}

	// This method is used to persist static, non-animated data.
	// This includes the storage of the default value inside the JSON file.
	// Key-framed data is stored via the 'classical' ReadData/WriteData functions on HybridDataTypeClass.
	static maxon::Result<void> DescribeIO(const maxon::DataSerializeInterface& stream);

	// The accessors that we use.
	Float GetValue() const;
	void SetValue(Float value);

private:

	// In order to assert that we don't wrongly reinterpret with maxon::Float, we add some rather random padding.
	Char _startPadding = 'A';
	Float _value = 1.23; // The actual value that can be modified in the GUI.
	Char _endPadding = 'Z';
};

MAXON_DATATYPE(HybridDataType, "net.maxonsdk.datatype.hybriddatatype");

// This class handles our custom data type with respect to the 'classical' data type API.
class HybridDataTypeClass : public CustomDataTypeClass
{
	INSTANCEOF(HybridDataTypeClass, CustomDataTypeClass)

public:
	virtual Int32 GetId() override;

	virtual CustomDataType* AllocData() override;

	virtual void FreeData(CustomDataType* data) override;

	virtual Bool CopyData(const CustomDataType* src, CustomDataType* dst, AliasTrans* aliastrans) override;

	virtual Int32 Compare(const CustomDataType* a, const CustomDataType* b) override;

	virtual Bool WriteData(const CustomDataType* t_d, HyperFile* hf) override;

	virtual Bool ReadData(CustomDataType* t_d, HyperFile* hf, Int32 level) override;

	virtual Bool InterpolateKeys(GeData& res, const GeData& t_dataA, const GeData& t_dataB, Float mix, Int32 flags) override;

	virtual const Char* GetResourceSym() override;

	virtual void GetDefaultProperties(BaseContainer& data) override;

	static maxon::Result<void> RegisterDataType();
};

#include "hybriddatatype1.hxx"
#include "hybriddatatype2.hxx"

} // namespace maxonsdk

#endif // HYBRIDDATATYPE_H__
