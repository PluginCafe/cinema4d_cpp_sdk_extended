#ifndef GRADIENTUSERAREA_H__
#define GRADIENTUSERAREA_H__

#include "ge_math.h"

#define MAXGRADIENT	20

namespace cinema
{

class BaseBitmap;
class GeUserArea;

} // namespace cinema

struct SDKGradient
{
	cinema::Vector col;
	cinema::Float	 pos;
	cinema::Int32	 id;
};

class SDKGradientGadget
{
private:
	cinema::Int32				 iw, ih, xmin, active, *count, *interpol, maxgrad;
	cinema::Int32				 dragx, dragy, dragid;
	cinema::Vector			 dragcol;
	SDKGradient* g;
	cinema::GeUserArea*	 ua;

	cinema::Float YtoP(cinema::Int32 y);
	cinema::Int32 PtoY(cinema::Float pos);
	void GetBoxPosition(cinema::Int32 num, cinema::Int32* x, cinema::Int32* y);
	cinema::Int32 InsertBox(cinema::Vector col, cinema::Float per, cinema::Int32 id);
	cinema::Int32 FindID();
	void RemoveBox(cinema::Int32 num);

public:
	cinema::BaseBitmap* col;

	SDKGradientGadget();
	~SDKGradientGadget();

	void Init(cinema::GeUserArea* a_ua, SDKGradient* a_g, cinema::Int32* a_count, cinema::Int32* a_interpol, cinema::Int32 a_maxgrad);
	cinema::Bool InitDim(cinema::Int32 x, cinema::Int32 y);

	cinema::Bool MouseDown(cinema::Int32 x, cinema::Int32 y, cinema::Bool dbl);
	void MouseDrag(cinema::Int32 x, cinema::Int32 y);

	void SetPosition(cinema::Float per);
	cinema::Bool GetPosition(cinema::Float* per);

	void CalcImage();
};

cinema::Vector CalcGradientMix(const cinema::Vector& g1, const cinema::Vector& g2, cinema::Float per, cinema::Int32 interpol);

#endif // GRADIENTUSERAREA_H__
