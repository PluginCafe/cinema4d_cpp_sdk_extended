#ifndef REGISTERADVANCEDPAINT_H__
#define REGISTERADVANCEDPAINT_H__

#include "c4d.h"

Bool RegisterPaintBrushBase();
Bool RegisterPaintBrushSculpt();
Bool RegisterPaintUndoSystem();
Bool FreePaintUndoSystem();

Bool RegisterPaintAdvanced();
Bool FreePaintAdvanced();

#endif // REGISTERADVANCEDPAINT_H__
