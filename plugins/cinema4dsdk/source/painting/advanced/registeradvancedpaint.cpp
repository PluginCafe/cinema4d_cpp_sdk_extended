#include "registeradvancedpaint.h"

Bool RegisterPaintAdvanced()
{
	RegisterPaintUndoSystem();
	RegisterPaintBrushBase();
	RegisterPaintBrushSculpt();
	return true;
}

Bool FreePaintAdvanced()
{
	FreePaintUndoSystem();
	return true;
}
