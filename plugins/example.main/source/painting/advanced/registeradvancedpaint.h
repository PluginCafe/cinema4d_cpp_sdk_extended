#ifndef REGISTERADVANCEDPAINT_H__
#define REGISTERADVANCEDPAINT_H__

#include "c4d.h"

cinema::Bool RegisterPaintBrushBase();
cinema::Bool RegisterPaintBrushSculpt();
cinema::Bool RegisterPaintUndoSystem();
cinema::Bool FreePaintUndoSystem();

cinema::Bool RegisterPaintAdvanced();
cinema::Bool FreePaintAdvanced();

#endif // REGISTERADVANCEDPAINT_H__
