#include "registeradvancedpaint.h"

cinema::Bool RegisterPaintAdvanced()
{
	RegisterPaintUndoSystem();
	RegisterPaintBrushBase();
	RegisterPaintBrushSculpt();
	return true;
}

cinema::Bool FreePaintAdvanced()
{
	FreePaintUndoSystem();
	return true;
}
