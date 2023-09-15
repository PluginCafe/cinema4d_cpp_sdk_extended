#include "paintbrushbase.h"
#include "registeradvancedpaint.h"

class PaintBrushSculpt : public PaintBrushBase
{
public:
	explicit PaintBrushSculpt(SculptBrushParams *pParams) : PaintBrushBase(pParams) { }
	virtual ~PaintBrushSculpt() { }

	virtual Int32 GetToolPluginId() const;
	virtual const String GetResourceSymbol() const;
};

Int32 PaintBrushSculpt::GetToolPluginId() const
{
	return ID_PAINT_BRUSH_SCULPT;
}

const String PaintBrushSculpt::GetResourceSymbol() const
{
	return String("toolpaintbrushsculpt");
}

// This method does all the work for the brush. Every time a dab is placed down on the surface this method is called. It will
// be called for every symmetrical dab as well, but you do not need to worry about them at all.
static Bool SculptMovePointsFunc(BrushDabData *dab)
{
	// Paint First
	PaintBrushBase::MovePointsFunc(dab);

	// If the brush strength is 0 then we don't need to do anything.
	// This is the brush strength from the UI. This has been automatically adjusted by any FX settings and tablet pressure values.
	if (dab->GetBrushStrength() == 0) 
		return true;

	// Get the radius of the brush. This is the brush size from the UI. This has been automatically adjusted by any FX settings and tablet pressure values.
	Float brushRadius = dab->GetBrushRadius();

	// If the brush radius is 0 then we don't need to do anything since there won't be any data.
	if (brushRadius <= 0) 
		return false;

	// Get the BaseContainer for the brush settings.
	const BaseContainer *brushData = dab->GetData();

	// Get the buildup slider value and adjust it to a usable range.
	Float buildup = brushData->GetFloat(MDATA_SCULPTBRUSH_SETTINGS_BUILDUP) * 0.002;

	// Is the dab a preview dab? A Preview dab occurs when in the DragDab or DragRect DrawModes.
	// This draws to a temporary preview layer while the user is interactively moving the mouse on the surface of the model.
	Bool usePreview = dab->IsPreviewDab();

	// Get the PolygonObject for the current sculpt object. This will be the sculpt object at the current subdivision level.
	// We are getting access to the PolygonObject so that we can determine its size in the scene. The size will need to be taken
	// into account to adjust the strength of the offsets.
	PolygonObject *polyObj = dab->GetPolygonObject();

	// If for some reason it does not exist then return. This should never happen but it is better to check and be safe than to just assume everything is ok.
	if (!polyObj) 
		return false;

	// Get the radius of the PolygonObject and use this in the calculation to adjust the brush strength.
	// Very large objects will need to move their points further otherwise the dabs will not be noticeable on the surface of the object.
	Float dim = polyObj->GetRad().GetLength() * 0.005;

	// Create a multiply vector to move the points by using the brushes strength, normal and the size of the object.
	Float pressurePreMult = dab->GetBrushStrength() * 10.0 * buildup * dim;

	// The normal is the average normal of all the vertices for this dab.
	Vector multPreMult = dab->GetNormal() * pressurePreMult;

	// If the user is holding down the Ctrl Key then the OVERRIDE::INVERT flag will be set. If it is set then we invert the direction of the multiplier vector.
	if (dab->GetBrushOverride() & OVERRIDE::INVERT)
	{
		multPreMult = -multPreMult;
	}

	// The user may also have the invert checkbox enabled in the UI. Check for this and then invert the direction of the multiplier vector again if we need to.
	if (brushData->GetBool(MDATA_SCULPTBRUSH_SETTINGS_INVERT))
	{
		multPreMult = -multPreMult;
	}

	// Loop over very point for this dab and move it if we need to.
	Int32 a;
	Int32 count = dab->GetPointCount();
	const BrushPointData *pPointData = dab->GetPointData();
	for (a = 0; a < count; ++a)
	{
		// Get the index of the point on the PolygonObject.
		Int32 pointIndex = pPointData[a].pointIndex;

		// Get the falloff for this point. This will always be a value from 0 to 1.
		// The value returned is a combination of the following values all multiplied together to give a final value.
		//	- The falloff curve.
		//	- The color of the stamp with its color value averaged to gray and adjusted by the Gray Value.
		//	- The color of the stencil with its color value averaged to gray and adjusted by the Gray Value.
		Float fallOff = dab->GetBrushFalloff(a);

		// If the falloff value is 0 then we don't have to do anything at all.
		if (fallOff == 0) 
			continue;

		// Multiply the falloff value with the multiply vector we calculated early. This will result in an offset vector that we want to move the vertex on the model by.
		Vector res = fallOff * multPreMult;

		// If the brush is not in preview mode (preview mode happens with in DragDab or DragRect mode) then we can offset the final point on the selected layer.
		if (!usePreview)
		{
			dab->OffsetPoint(pointIndex, res);
		}
		// Otherwise we apply the offset to the preview layer.
		else
		{
			dab->OffsetPreviewPoint(pointIndex, res);
		}
	}
	return true;
}

Bool RegisterPaintBrushSculpt()
{
	SculptBrushParams *pParams = SculptBrushParams::Alloc();
	if (!pParams) 
		return false;

	pParams->EnableInvertCheckbox(true);
	pParams->EnableBrushAccess(true);
	pParams->SetUndoType(SCULPTBRUSHDATATYPE::POINT);
	pParams->SetMovePointFunc(&SculptMovePointsFunc);

	String name = GeLoadString(IDS_PAINT_BRUSH_SCULPT); 
	if (!name.IsPopulated()) 
		return true;
	return RegisterToolPlugin(ID_PAINT_BRUSH_SCULPT, name, PLUGINFLAG_TOOL_SCULPTBRUSH|PLUGINFLAG_TOOL_NO_OBJECTOUTLINE | PLUGINFLAG_TOOL_NO_TOPOLOGY_EDIT, nullptr, GeLoadString(IDS_PAINT_BRUSH_SCULPT_HELP), NewObjClear(PaintBrushSculpt, pParams));
}
