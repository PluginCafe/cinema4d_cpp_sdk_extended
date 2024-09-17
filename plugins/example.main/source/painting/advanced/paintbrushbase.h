#ifndef PAINTBRUSHBASE_H__
#define PAINTBRUSHBASE_H__

#include "lib_sculpt.h"
#include "lib_sculptbrush.h"
#include "c4d_symbols.h"
#include "c4d_resource.h"
#include "paintbrushids.h"
#include "toolpaintbrushbase.h"

class PaintBrushBase : public cinema::SculptBrushToolData
{
public:
	explicit PaintBrushBase(cinema::SculptBrushParams *pParams) : cinema::SculptBrushToolData(pParams) { }
	virtual ~PaintBrushBase() { }

	virtual cinema::Int32 GetToolPluginId() const;
	virtual const cinema::String GetResourceSymbol() const;

	void StartStroke(cinema::Int32 strokeCount, const cinema::BaseContainer &data);
	void EndStroke();

	static cinema::Bool MovePointsFunc(cinema::BrushDabData *dab);
};

#endif // PAINTBRUSHBASE_H__
