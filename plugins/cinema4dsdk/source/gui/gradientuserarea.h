#ifndef GRADIENTUSERAREA_H__
#define GRADIENTUSERAREA_H__

#include "ge_math.h"

#define MAXGRADIENT	20

class BaseBitmap;
class GeUserArea;

struct SDKGradient
{
	Vector col;
	Float	 pos;
	Int32	 id;
};

class SDKGradientGadget
{
private:
	Int32				 iw, ih, xmin, active, *count, *interpol, maxgrad;
	Int32				 dragx, dragy, dragid;
	Vector			 dragcol;
	SDKGradient* g;
	GeUserArea*	 ua;

	Float YtoP(Int32 y);
	Int32 PtoY(Float pos);
	void GetBoxPosition(Int32 num, Int32* x, Int32* y);
	Int32 InsertBox(Vector col, Float per, Int32 id);
	Int32 FindID();
	void RemoveBox(Int32 num);

public:
	BaseBitmap* col;

	SDKGradientGadget();
	~SDKGradientGadget();

	void Init(GeUserArea* a_ua, SDKGradient* a_g, Int32* a_count, Int32* a_interpol, Int32 a_maxgrad);
	Bool InitDim(Int32 x, Int32 y);

	Bool MouseDown(Int32 x, Int32 y, Bool dbl);
	void MouseDrag(Int32 x, Int32 y);

	void SetPosition(Float per);
	Bool GetPosition(Float* per);

	void CalcImage();
};

Vector CalcGradientMix(const Vector& g1, const Vector& g2, Float per, Int32 interpol);

#endif // GRADIENTUSERAREA_H__
