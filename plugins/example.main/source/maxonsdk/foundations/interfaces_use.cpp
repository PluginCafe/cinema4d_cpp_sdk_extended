// ------------------------------------------------------------------------
/// This file contains some example code that uses various example interface implementations.
/// It shows how to use published objects and HierarchyObjectInterface based elements.
// ------------------------------------------------------------------------

// Maxon API header files
#include "maxon/execution.h"
#include "maxon/configuration.h"

// local header files
#include "interfaces_declarations.h"

namespace maxonsdk
{
// ------------------------------------------------------------------------
/// A configuration variable to enable the execution of the test code.
// ------------------------------------------------------------------------
MAXON_CONFIGURATION_BOOL(g_maxonsdk_interfaces, false, maxon::CONFIGURATION_CATEGORY::DEVELOPMENT, "Execute interfaces tests.");

// ------------------------------------------------------------------------
/// An implementation of ExecutionInterface that will execute some test code on start-up.
// ------------------------------------------------------------------------
class InterfacesExecution : public maxon::ExecutionInterface<InterfacesExecution>
{
public:
	maxon::Result<void> operator ()()
	{
		iferr_scope;

		// only execute if g_maxonsdk_interfaces is set
		if (!g_maxonsdk_interfaces)
			return maxon::OK;

		// use SimpleNumberInterface

		MAXON_SCOPE
		{
			// use published object to create a new instance
			const maxonsdk::SimpleNumberRef simpleNumber = SimpleNumber().Create() iferr_return;

			// use instance
			simpleNumber.SetNumber(123);

			// access instance
			const maxon::Int	 number = simpleNumber.GetNumber();
			const maxon::Float floatNumber = simpleNumber.GetFloat();
			DiagnosticOutput("@, @", number, floatNumber);
		}

		MAXON_SCOPE
		{
			// use factory to create a new instance
			const maxonsdk::SimpleNumberRef simpleNumber = maxonsdk::SimpleNumberFactory().Create(123) iferr_return;

			// access instance
			const maxon::Int	 number = simpleNumber.GetNumber();
			const maxon::Float floatNumber = simpleNumber.GetFloat();
			DiagnosticOutput("@, @", number, floatNumber);
		}

		// use AdvancedNumberInterface

		MAXON_SCOPE
		{
			// use published object to create a new instance
			const maxonsdk::EvenOddNumberRef evenOddNumber = maxonsdk::EvenOddNumber().Create() iferr_return;

			// use instance
			evenOddNumber.SetNumber(123);

			// access instance
			const maxon::Int	number = evenOddNumber.GetNumber();
			const maxon::Bool even = evenOddNumber.IsEven();
			const maxon::Bool odd	 = evenOddNumber.IsOdd();

			DiagnosticOutput("@, @, @", number, even, odd);
		}

		// use SequenceOperationInterface

		MAXON_SCOPE
		{
			// use Summation

			// use published object to create a new instance
			const maxonsdk::SequenceOperationRef summation = maxonsdk::Summation().Create() iferr_return;

			// use instance
			summation.AddNumber(1.0) iferr_return;
			summation.AddNumber(2.0) iferr_return;
			summation.AddNumber(3.0) iferr_return;

			const maxon::Float res = summation.GetResult();

			DiagnosticOutput("@", res);
		}

		MAXON_SCOPE
		{
			// use Multiplication

			// use published object to create a new instance
			const maxonsdk::SequenceOperationRef multiplication = maxonsdk::Multiplication().Create() iferr_return;

			// use instance
			multiplication.AddNumber(1.0) iferr_return;
			multiplication.AddNumber(2.0) iferr_return;
			multiplication.AddNumber(3.0) iferr_return;

			const maxon::Float res = multiplication.GetResult();

			DiagnosticOutput("@", res);
		}

		// use DirectoryElementInterface

		MAXON_SCOPE
		{
			// create tree elements using the published object
			const maxonsdk::DirectoryElementRef root = DirectoryElement().Create() iferr_return;
			root.SetFolder("c:"_s);

			const maxonsdk::DirectoryElementRef programs = DirectoryElement().Create() iferr_return;
			programs.SetFolder("programs"_s);

			const maxonsdk::DirectoryElementRef application = DirectoryElement().Create() iferr_return;
			application.SetFolder("application"_s);

			// create tree
			root.InsertChildAsFirst(programs) iferr_return;
			programs.InsertChildAsFirst(application) iferr_return;

			// get full Url

			const maxon::Url fullUrl = application.GetFullUrl() iferr_return;
			DiagnosticOutput("@", fullUrl);
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
MAXON_DECLARATION_REGISTER(ExecutionJobs, "net.maxonexample.execution.interfaces")
{
	return NewObj(maxonsdk::InterfacesExecution);
}
}
