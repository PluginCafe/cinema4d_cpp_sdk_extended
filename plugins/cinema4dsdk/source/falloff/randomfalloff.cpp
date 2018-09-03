#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_falloffplugin.h"
#include "ofalloff_random.h"
#include "main.h"

class RandomFalloff : public FalloffData
{
public:
	Random rnd;

	virtual Bool Init(FalloffDataData& falldata, BaseContainer* bc);
	virtual Bool InitFalloff(BaseContainer* bc, FalloffDataData& falldata);
	virtual void Sample(const Vector& p, const FalloffDataData& data, Float* res);

	static FalloffData* Alloc() { return NewObjClear(RandomFalloff); }
};

Bool RandomFalloff::Init(FalloffDataData& falldata, BaseContainer* bc)
{
	if (!bc)
		return false;

	if (bc->GetData(RANDOMFALLOFF_SEED).GetType() == DA_NIL)
		bc->SetInt32(RANDOMFALLOFF_SEED, 1234567);

	return true;
}

Bool RandomFalloff::InitFalloff(BaseContainer* bc, FalloffDataData& falldata)
{
	if (!bc)
		return false;

	rnd.Init(bc->GetInt32(RANDOMFALLOFF_SEED));

	return true;
}

void RandomFalloff::Sample(const Vector& p, const FalloffDataData& data, Float* res)
{
	(*res) = rnd.Get01();
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_RANDOMFALLOFF 1019569

Bool RegisterRandomFalloff()
{
	return RegisterFalloffPlugin(ID_RANDOMFALLOFF, GeLoadString(IDS_RANDOMFALLOFF), 0, RandomFalloff::Alloc, "ofalloff_random"_s);
}
