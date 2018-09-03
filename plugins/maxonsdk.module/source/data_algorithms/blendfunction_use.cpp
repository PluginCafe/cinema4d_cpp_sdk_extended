// ------------------------------------------------------------------------
/// This file contains some example code that uses an implementation of
/// BlendFunctionInterface. The code is executed using an
/// ExecutionInterface implementation.
// ------------------------------------------------------------------------

// MAXON API header files
#include "maxon/blend_function.h"
#include "maxon/execution.h"
#include "maxon/configuration.h"

// local header files
#include "blendfunction_declarations.h"

namespace maxonsdk
{
// ------------------------------------------------------------------------
/// A configuration variable to enable the execution of BlendFunctionExecution.
// ------------------------------------------------------------------------
MAXON_CONFIGURATION_BOOL(g_maxonsdk_blendfunction, false, maxon::CONFIGURATION_CATEGORY::DEVELOPMENT, "Execute example blend function.");

// ------------------------------------------------------------------------
/// An implementation of ExecutionInterface that will execute some BlendFunction
/// test code on start-up.
// ------------------------------------------------------------------------
class BlendFunctionExecution : public maxon::ExecutionInterface<BlendFunctionExecution>
{
public:
	maxon::Result<void> operator ()()
	{
		iferr_scope;

		// only execute if g_maxonsdk_blendfunction is set
		if (!g_maxonsdk_blendfunction)
			return maxon::OK;

		// get blend function object
		const maxon::BlendFunctionRef& step = maxon::BlendFunctions::MaxonSDKStep();

		// prepare sampling
		const maxon::Int	 count = 100;
		const maxon::Float stepSize = 1.0_f / maxon::Float(count);

		const maxon::Float32 start(0.0);
		const maxon::Float32 end(1.0);

		// sample blend function
		for (maxon::Int i = 0; i <= count; ++i)
		{
			const maxon::Float inputValue = i * stepSize;

			const maxon::Data		 res = step.MapValue(inputValue, maxon::Data(start), maxon::Data(end)) iferr_return;
			const maxon::Float32 outputValue = res.Get<maxon::Float32>() iferr_return;

			DiagnosticOutput("Input: @, Output: @", inputValue, outputValue);
		}

		return maxon::OK;
	}
};
}

namespace maxon
{
// ------------------------------------------------------------------------
/// Registers the implementation at ExecutionJobs.
// ------------------------------------------------------------------------
MAXON_DECLARATION_REGISTER(ExecutionJobs, "net.maxonexample.execution.blendfunction")
{
	return NewObj(maxonsdk::BlendFunctionExecution);
}
}
