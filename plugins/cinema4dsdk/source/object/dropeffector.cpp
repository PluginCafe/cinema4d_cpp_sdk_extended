// this example demonstrates how to implement a more complex direct control effector
// and utilize falloff and strength directly
// the effector drops the particles to a surface

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_collider.h"
#include "c4d_baseeffectorplugin.h"
#include "c4d_falloffplugin.h"
#include "oedrop.h"
#include "main.h"

struct DropEffectorData
{
	BaseObject* target;
	Int32				mode;
	Float				maxdist;
	Matrix			genmg, igenmg;
	Matrix			targmg, itargmg;
};

class DropEffector : public EffectorData
{
public:
	DropEffectorData				 ed;
	AutoAlloc<GeRayCollider> rcol;
	GeRayColResult					 rcolres;

	virtual Bool InitEffector(GeListNode* node);

	virtual void InitPoints(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, MoData* md, BaseThread* thread);
	virtual void ModifyPoints(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, MoData* md, BaseThread* thread);

	static NodeData* Alloc() { return NewObjClear(DropEffector); }
};

Bool DropEffector::InitEffector(GeListNode* node)
{
	if (!rcol || !node)
		return false;

	BaseObject* op = (BaseObject*)node;
	if (!op)
		return false;

	BaseContainer* bc = op->GetDataInstance();
	if (!bc)
		return false;

	bc->SetFloat(DROPEFFECTOR_DISTANCE, 1000.0);

	return true;
}

void DropEffector::InitPoints(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, MoData* md, BaseThread* thread)
{
	BaseContainer* bc = op->GetDataInstance();
	if (!bc)
		return;

	if (!rcol)
		return;

	ed.mode = bc->GetInt32(DROPEFFECTOR_MODE);
	ed.maxdist = bc->GetFloat(DROPEFFECTOR_DISTANCE);
	ed.target	 = bc->GetObjectLink(DROPEFFECTOR_TARGET, doc);
	if (!ed.target)
		return;

	ed.targmg	 = ed.target->GetMg();
	ed.itargmg = ~ed.targmg;
	ed.genmg	= gen->GetMg();
	ed.igenmg = ~ed.genmg;

	// Add a dependency so that the effector will update if the target changes
	AddEffectorDependence(ed.target);

	// Can't init raycollider or the target isn't polygonal, then skip
	if (!rcol->Init(ed.target))
		ed.target = nullptr;
	else if (!ed.target->IsInstanceOf(Opolygon))
		ed.target = nullptr;
}

void DropEffector::ModifyPoints(BaseObject* op, BaseObject* gen, BaseDocument* doc, EffectorDataStruct* data, MoData* md, BaseThread* thread)
{
	if (!ed.target || !rcol || data->strength == 0.0)
		return;

	C4D_Falloff* falloff = GetFalloff();
	if (!falloff)
		return;

	Int32	 i = 0;
	Float	 fall	 = 0.0;
	Vector off	 = Vector(0.0);
	Vector ray_p = Vector(0.0), ray_dir = Vector(0.0);
	Vector targ_off = Vector(0.0), targ_hpb = Vector(0.0);

	MDArray<Int32>	flag_array = md->GetLongArray(MODATA_FLAGS);
	MDArray<Matrix> mat_array	 = md->GetMatrixArray(MODATA_MATRIX);
	MDArray<Float>	weight_array = md->GetRealArray(MODATA_WEIGHT);

	if (!mat_array)
		return;

	Int32 mdcount = (Int32)md->GetCount();
	for (i = 0; i < mdcount; i++)
	{
		// If the particle isn't visible, don't calculate
		if (!(flag_array[i] & MOGENFLAG_CLONE_ON) || (flag_array[i] & MOGENFLAG_DISABLE))
			continue;

		// Multiply into global space
		off = mat_array[i].off;
		off = ed.genmg * off;

		// Sample the falloff
		falloff->Sample(off, &fall, true, weight_array[i]);
		if (fall == 0.0)
			continue;

		// Set up the ray for the collision
		ray_p = ed.itargmg * off;
		switch (ed.mode)
		{
			default:
			case DROPEFFECTOR_MODE_PNORMAL:		ray_dir = ed.genmg.sqmat * mat_array[i].sqmat.v3; break;
			case DROPEFFECTOR_MODE_NNORMAL:		ray_dir = -(ed.genmg.sqmat * mat_array[i].sqmat.v3); break;
			case DROPEFFECTOR_MODE_AXIS:			ray_dir = (ed.targmg.off - off); break;
			case DROPEFFECTOR_MODE_SELFAXIS:	ray_dir = (ed.genmg.off - off);	break;
			case DROPEFFECTOR_MODE_PX:				ray_dir = Vector(1.0, 0.0, 0.0); break;
			case DROPEFFECTOR_MODE_PY:				ray_dir = Vector(0.0, 1.0, 0.0); break;
			case DROPEFFECTOR_MODE_PZ:				ray_dir = Vector(0.0, 0.0, 1.0); break;
			case DROPEFFECTOR_MODE_NX:				ray_dir = Vector(-1.0, 0.0, 0.0);	break;
			case DROPEFFECTOR_MODE_NY:				ray_dir = Vector(0.0, -1.0, 0.0);	break;
			case DROPEFFECTOR_MODE_NZ:				ray_dir = Vector(0.0, 0.0, -1.0);	break;
		}
		ray_dir = ed.itargmg.sqmat * ray_dir;

		// Calculate an intersection
		if (rcol->Intersect(ray_p, !ray_dir, ed.maxdist, false))
		{
			if (rcol->GetNearestIntersection(&rcolres))
			{
				fall *= data->strength;

				targ_off = Blend(mat_array[i].off, ed.igenmg * (ed.targmg * rcolres.hitpos), fall);
				targ_hpb = VectorToHPB(ed.igenmg.sqmat * (ed.targmg.sqmat * rcolres.s_normal));

				mat_array[i] = HPBToMatrix(Blend(MatrixToHPB(mat_array[i], ROTATIONORDER::DEFAULT), targ_hpb, fall), ROTATIONORDER::DEFAULT);
				mat_array[i].off = targ_off;
			}
		}
	}
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_DROPEFFECTOR 1019571

Bool RegisterDropEffector()
{
	return RegisterEffectorPlugin(ID_DROPEFFECTOR, GeLoadString(IDS_DROPEFFECTOR), OBJECT_CALL_ADDEXECUTION, DropEffector::Alloc, "oedrop"_s, AutoBitmap("dropeffector.tif"_s), 0);
}
