#include "maxon/vm.h"
#include "maxon/cpython.h"
#include "maxon/blockarray.h"
#include "maxon/logger.h"

#include "c4d.h"
#include "main.h"

class PythonRegexCommand : public CommandData
{
public:

	virtual Bool Execute(BaseDocument* doc, GeDialog* parentManager)
	{
		iferr_scope_handler
		{
			err.CritStop();
			return false;
		};

		auto executeVm = [](const maxon::VirtualMachineRef& vm, const maxon::LoggerRef& logger) -> maxon::Result<void>
		{
			iferr_scope;

			auto scope = vm.CreateScope() iferr_return;

			maxon::String code = "import re\n"
				"def main(str):\n"
				"    return bool(re.match('[\\w\\.-]+@[\\w\\.-]+', str))\n"_s;

			scope.Init("regextest"_s, code, maxon::ERRORHANDLING::PRINT, nullptr) iferr_return;

			// unused because the scope itself already prints and handles exceptions
			scope.Execute() iferr_return;

			auto arg0 = maxon::Data("email@example.com"_s);

			maxon::BaseArray<maxon::Data*> args;
			args.Append(&arg0) iferr_return;

			maxon::BlockArray<maxon::Data> helperStack;
			auto* res = scope.PrivateInvoke("main"_s, helperStack, maxon::GetDataType<maxon::Bool>(), &args.ToBlock()) iferr_return;
			if (res && res->Get<Bool>().GetValue() == true)
				MAXON_WARN_MUTE_UNUSED logger.Write(maxon::TARGETAUDIENCE::ALL, FormatString("@: The entered email address is valid"_s, vm.GetVersion()), MAXON_SOURCE_LOCATION, maxon::WRITEMETA::DEFAULT);
			else
				MAXON_WARN_MUTE_UNUSED logger.Write(maxon::TARGETAUDIENCE::ALL, FormatString("@: The entered email address failed to verify"_s, vm.GetVersion()), MAXON_SOURCE_LOCATION, maxon::WRITEMETA::DEFAULT);

			return maxon::OK;
		};

		MAXON_WARN_MUTE_UNUSED executeVm(MAXON_CPYTHON37VM(), maxon::Loggers::Get(maxon::ID_LOGGER_PYTHON));

		return true;
	}
};

Bool RegisterPythonRegexCommand()
{
	if (!RegisterCommandPlugin(8436385, "Python Execution Example"_s, 0, nullptr, "Sends a string to Python and executes a regex operation on it"_s, NewObjClear(PythonRegexCommand)))
		return false;

	return true;
}


