#include "c4d.h"

using namespace cinema;

static const Int32 PluginID = 99990000;

#include "customerror_interface.h"

// Dummy test function
maxon::Result<void> TestFunction(maxon::Int * val);
maxon::Result<void> TestFunction(maxon::Int * val)
{
	iferr_scope;
	
	if (!val)
		return CustomError(MAXON_SOURCE_LOCATION, 4242);
	
	ApplicationOutput("Value is @", *val);
	
	return maxon::OK;
}

// Command to test the custom error
class CustomErrorExample : public CommandData
{
public:
	
#if API_VERSION >= 20000 && API_VERSION < 21000
	Bool Execute(BaseDocument* doc)
#elif API_VERSION >= 21000
	Bool Execute(BaseDocument* doc, GeDialog* parentManager)
#endif
	{
		iferr_scope_handler
		{
			return false;
		};
		
		if (!doc)
			return false;
		
		iferr (TestFunction(nullptr))
			ApplicationOutput("Error: @", err);
		
		maxon::Int a = 100;
		
		iferr (TestFunction(&a))
			ApplicationOutput("Error: @", err);
				
		return true;
	}
	
	static CommandData* Alloc() { return NewObj(CustomErrorExample) iferr_ignore("Unexpected failure on allocating CustomErrorExample plugin."_s); }
};

Bool RegisterCustomErrorExample();
Bool RegisterCustomErrorExample()
{
	return RegisterCommandPlugin(PluginID, "C++ SDK - Custom Error Test"_s, PLUGINFLAG_COMMAND_OPTION_DIALOG, nullptr, ""_s, CustomErrorExample::Alloc());
}

