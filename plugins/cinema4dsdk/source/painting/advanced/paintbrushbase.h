#ifndef PAINTBRUSHBASE_H__
#define PAINTBRUSHBASE_H__

#include "lib_sculpt.h"
#include "lib_sculptbrush.h"
#include "c4d_symbols.h"
#include "c4d_resource.h"
#include "paintbrushids.h"
#include "toolpaintbrushbase.h"

class PaintBrushBase : public SculptBrushToolData
{
public:
	explicit PaintBrushBase(SculptBrushParams *pParams) : SculptBrushToolData(pParams) { }
	virtual ~PaintBrushBase() { }

	virtual Int32 GetToolPluginId();
	virtual const String GetResourceSymbol();

	void StartStroke(Int32 strokeCount, const BaseContainer &data);
	void EndStroke();

	static Bool MovePointsFunc(BrushDabData *dab);
};

#endif // PAINTBRUSHBASE_H__
