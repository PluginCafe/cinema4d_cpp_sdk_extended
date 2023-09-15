// ------------------------------------------------------------------------
/// This file contains an implementation of the BlendFunctionInterface.
///
/// "BlendFunctionStepImpl" is the implementation class that defines the
/// implementation behaviour.
// ------------------------------------------------------------------------

// MAXON API header files
#include "maxon/blend_function.h"
#include "maxon/errortypes.h"
#include "maxon/lib_math.h"

// local header files
#include "blendfunction_declarations.h"

namespace maxon
{
// ------------------------------------------------------------------------
/// An implementation of BlendFunctionInterface that implements a "step" function
// ------------------------------------------------------------------------
class BlendFunctionStepImpl : public Component<BlendFunctionStepImpl, BlendFunctionInterface>
{
	MAXON_COMPONENT();

public:
	MAXON_METHOD Result<Data> MapValue(Float x, const Data& startValue, const Data& endValue)
	{
		// implement this function to define the blending algorithm

		iferr_scope;

		// validate input data by checking the input data types
		const DataType& startType = startValue.GetType();
		const DataType& endType = endValue.GetType();

		// data types must match
		if (startType != endType)
		{
			const String errorMessage = FormatString("Different types are not supported: @, @", startType, endType);
			return IllegalArgumentError(MAXON_SOURCE_LOCATION, errorMessage);
		}

		// check for valid type
		// to easily support more data types one could use function templates
		if (startType == GetDataType<Float32>())
		{
			// get values
			const Float32 start = startValue.Get<Float32>() iferr_return;
			const Float32 end = endValue.Get<Float32>() iferr_return;

			// step
			Float32 result = start;
			if (x > 0.5)
				result = end;

			// return result
			return Data(result);
		}
		else if (startType == GetDataType<Float64>())
		{
			// get values
			const Float64 start = startValue.Get<Float64>() iferr_return;
			const Float64 end = endValue.Get<Float64>() iferr_return;

			// step
			Float64 result = start;
			if (x > 0.5)
				result = end;

			// return result
			return Data(result);
		}

		// if no data could be returned, something must have gone wrong
		return UnsupportedOperationError(MAXON_SOURCE_LOCATION);
	}
};

// ------------------------------------------------------------------------
/// Registers the implementation at the given published object.
// ------------------------------------------------------------------------
MAXON_COMPONENT_OBJECT_REGISTER(BlendFunctionStepImpl, BlendFunctions::MaxonSDKStep);
}
