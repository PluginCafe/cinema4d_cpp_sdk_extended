#include "c4d_resource.h"
#include "lib_sn.h"
#include "c4d_symbols.h"
#include "main.h"

class ExampleSNHookClass : public SNHookClass
{
public:
	String name;

	ExampleSNHookClass()
	{
		name = GeLoadString(IDS_SERIAL_HOOK);
		if (!name.IsPopulated())
			name = "C++ SDK - Example Serial Hook";
	}

	virtual ~ExampleSNHookClass() { }

	virtual Int32 SNCheck(const String& c4dsn, const String& sn, Int32 regdate, Int32 curdate)
	{
		return sn == String("123456789-abcdef") ? SN_OKAY : SN_WRONGNUMBER;
	}

	virtual const String& GetTitle()
	{
		return name;
	}
};

ExampleSNHookClass* g_snhook = nullptr;

Bool RegisterExampleSNHook()
{
	g_snhook = NewObjClear(ExampleSNHookClass);
	if (!g_snhook->Register(450000241, SNFLAG_OWN))
		return false;
	return true;
}

void FreeExampleSNHook()
{
	if (g_snhook)
		DeleteObj(g_snhook);
}
