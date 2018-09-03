// this example demonstrates how to implement a simple value driven type effector

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_baseeffectorplugin.h"
#include "main.h"
#include "oenoise.h"

class NoiseEffector : public EffectorData
{
public:
	Float size;

	virtual Bool InitEffector(GeListNode* node);

	virtual void InitPoints(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, MoData* md, BaseThread* thread);

	virtual void CalcPointValue(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, Int32 index, MoData* md, const Vector& globalpos, Float fall_weight);
	virtual Vector CalcPointColor(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, Int32 index, MoData* md, const Vector& globalpos, Float fall_weight);

	static NodeData* Alloc() { return NewObjClear(NoiseEffector); }
};

// Called when effector is first created
Bool NoiseEffector::InitEffector(GeListNode* node)
{
	BaseObject* op = (BaseObject*)node;
	if (!op)
		return false;

	BaseContainer* bc = op->GetDataInstance();
	if (!bc)
		return false;

	bc->SetFloat(ID_MG_BASEEFFECTOR_MINSTRENGTH, -1.0);
	bc->SetBool(ID_MG_BASEEFFECTOR_POSITION_ACTIVE, true);
	bc->SetVector(ID_MG_BASEEFFECTOR_POSITION, Vector(50.0));
	bc->SetFloat(NOISEEFFECTOR_SCALE, 0.01);

	return true;
}

// Called just before points calculation
void NoiseEffector::InitPoints(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, MoData* md, BaseThread* thread)
{
	BaseContainer* bc = op->GetDataInstance();
	if (!bc)
		return;

	size = bc->GetFloat(NOISEEFFECTOR_SCALE);
}

// This example shows two methods to setting the values that drive the effector
// change this value to 1 to try the second method
const Int32 EffectorMethod = 0;

void NoiseEffector::CalcPointValue(BaseObject* op, BaseObject* gen,	BaseDocument* doc, EffectorDataStruct* data, Int32 index, MoData* md, const Vector& globalpos, Float fall_weight)
{
	switch (EffectorMethod)
	{
		case 0:
		{
			// First Method, this one just iterates through the "blends" which are the raw driver values and is slightly faster
			Int32	 i = 0;
			Float* buf = data->strengths;
			for (i = 0; i < BLEND_COUNT; i++, buf++)
			{
				(*buf) = Noise((globalpos + Vector(i * 20.0)) * size);
			}
			break;
		}
		
		case 1:
		{
			// Method 2, this casts the the more user friendly EffectorStrengths structure for setting of values
			EffectorStrengths* es = (EffectorStrengths*)data->strengths;
			// Position
			es->pos = Vector(Noise(globalpos * size),
									Noise((globalpos + Vector(10.0)) * size),
									Noise((globalpos - Vector(10.0)) * size));
			// Rotation
			es->rot = Vector(Noise((globalpos + Vector(20.0, 60.0, -90.0)) * size),
									Noise((globalpos + Vector(-20.0, 60.0, 90.0)) * size),
									Noise((globalpos + Vector(20.0, -60.0, 90.0)) * size));
			// Scale
			es->scale = Vector(Noise((globalpos + Vector(120.0, -160.0, -190.0)) * size),
										Noise((globalpos + Vector(100.0, 160.0, 190.0)) * size),
										Noise((globalpos + Vector(160.0, 160.0, 190.0)) * size));
			// Color opacity
			es->col = Vector(Noise(globalpos * size));
			// Others, x = U, y = V, z = Visibility
			es->other = Vector(Noise((globalpos + Vector(40.0, 70.0, -10.0)) * size),
										Noise((globalpos + Vector(-40.0, 70.0, 10.0)) * size),
										Noise((globalpos + Vector(40.0, -70.0, 10.0)) * size));
			// Others2, x = weight, y = clone index, z = time
			es->other2 = Vector(Noise((globalpos + Vector(90.0, 20.0, -50.0)) * size),
										 Noise((globalpos + Vector(-90.0, 20.0, 50.0)) * size),
										 Noise((globalpos + Vector(90.0, -20.0, 50.0)) * size));
			// Others3 <reserved>
			es->other3 = Vector(Noise((globalpos + Vector(90.0, 20.0, -50.0)) * size),
										 Noise((globalpos + Vector(-90.0, 20.0, 50.0)) * size),
										 Noise((globalpos + Vector(90.0, -20.0, 50.0)) * size));
			break;
		}
		
		default: 
			break;
	}
}

// Calculcate the effector color
Vector NoiseEffector::CalcPointColor(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, Int32 index, MoData* md, const Vector& globalpos, Float fall_weight)
{
	return Vector(Noise(globalpos), Noise(globalpos + Vector(20.0)), Noise(globalpos - Vector(20.0)));
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_NOISEEFFECTOR 1019570

Bool RegisterNoiseEffector()
{
	return RegisterEffectorPlugin(ID_NOISEEFFECTOR, GeLoadString(IDS_NOISEEFFECTOR), OBJECT_CALL_ADDEXECUTION, NoiseEffector::Alloc, "oenoise"_s, AutoBitmap("noiseeffector.tif"_s), 0);
}
