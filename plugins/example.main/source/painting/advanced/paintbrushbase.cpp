#include "paintbrushbase.h"
#include "paintundo.h"
#include "rasterize_bary.h"
#include "registeradvancedpaint.h"
#include "lib_paint.h"

Int32 PaintBrushBase::GetToolPluginId() const
{
	return ID_PAINT_BRUSH_BASE;
}

const String PaintBrushBase::GetResourceSymbol() const
{
	return String("toolpaintbrushbase");
}

Bool PaintBrushBase::MovePointsFunc(BrushDabData *dab)
{
	if (dab->GetBrushStrength() == 0) 
		return true;

	PolygonObject *pPoly = dab->GetPolygonObject();
	if (!pPoly) 
		return false;

	UVWTag *pUVs = (UVWTag*)pPoly->GetTag(Tuvw);
	if (!pUVs) 
		return false;

	BaseDocument *pDoc = pPoly->GetDocument();
	if (!pDoc) 
		return false;

	Filename docpath = pDoc->GetDocumentPath();

	PaintChannels channels;
	PaintTexture *theTexture = PaintTexture::GetSelectedTexture();
	if (theTexture)
	{
		PaintLayer *layer = theTexture->GetActive();
		if (layer && layer->IsInstanceOf(OBJECT_PAINTLAYERBMP))
		{
			channels.channel = (PaintLayerBmp*)layer;
		}
	}
	else
	{
		return false;
	}

	// Get the color for the currently selected channel.
	BPSingleColorSettings *colorSettings = BPColorSettingsHelpers::GetSelectedSingleColorSettings(false);
	if (colorSettings)
	{
		Float h;
		colorSettings->GetRGB(channels.fgColor, h);
	}
	else
	{
		channels.fgColor = Vector(1, 0, 0);
	}

	const CPolygon *polygons = pPoly->GetPolygonR();
	const Vector *points = dab->GetPoints();

	Int32 count = dab->GetPolyCount();
	const BrushPolyData *pPolyData = dab->GetPolyData();

	channels.useStencil = dab->GetData()->GetBool(MDATA_SCULPTBRUSH_STENCIL);
	channels.useStamp = dab->GetData()->GetBool(MDATA_SCULPTBRUSH_STAMP);
	channels.strength = maxon::Clamp01(dab->GetBrushStrength());

	Int32 drawMode = dab->GetData()->GetInt32(MDATA_SCULPTBRUSH_SETTINGS_DRAWMODE);
	channels.fillTool = (drawMode == MDATA_SCULPTBRUSH_SETTINGS_DRAWMODE_LASSO_FILL || drawMode == MDATA_SCULPTBRUSH_SETTINGS_DRAWMODE_POLY_FILL || drawMode == MDATA_SCULPTBRUSH_SETTINGS_DRAWMODE_RECTANGLE_FILL);

	if (!channels.Init()) 
		return false;

	for (Int32 a = 0; a < count; ++a)
	{
		Int32 polyIndex = pPolyData[a].polyIndex;
		const CPolygon &p = polygons[polyIndex];
		UVWStruct polyUVs = pUVs->GetSlow(polyIndex);

		channels.SetupPoly_Bary(dab, p, polyUVs, points);
		DrawTriangle_Bary(dab, &channels, &channels.triangle[0]);
		if (p.c != p.d)
		{
			DrawTriangle_Bary(dab, &channels, &channels.triangle[1]);
		}
	}
	channels.UpdateBitmaps();

	return true;
}

void PaintBrushBase::StartStroke(Int32 strokeCount, const BaseContainer &data)
{
	PaintUndoSystem *pPaintSystem = GetPaintUndoSystem(GetActiveDocument());
	if (pPaintSystem)
	{
		pPaintSystem->StartStroke();
	}
}

void PaintBrushBase::EndStroke()
{
	PaintUndoSystem *pPaintSystem = GetPaintUndoSystem(GetActiveDocument());
	if (pPaintSystem)
	{
		pPaintSystem->EndStroke();
	}
}

Bool RegisterPaintBrushBase()
{
	SculptBrushParams *pParams = SculptBrushParams::Alloc();
	if (!pParams) 
		return false;

	pParams->EnableInvertCheckbox(false);
	pParams->EnableBrushAccess(true);
	pParams->EnableToolSpecificSmooth(true);
	pParams->SetUndoType(SCULPTBRUSHDATATYPE::NONE);
	pParams->SetMovePointFunc(&PaintBrushBase::MovePointsFunc);

	String name = GeLoadString(IDS_PAINT_BRUSH_BASE); 
	if (!name.IsPopulated()) 
		return true;
	return RegisterToolPlugin(ID_PAINT_BRUSH_BASE, name, PLUGINFLAG_TOOL_SCULPTBRUSH|PLUGINFLAG_TOOL_NO_OBJECTOUTLINE | PLUGINFLAG_TOOL_NO_TOPOLOGY_EDIT, nullptr, GeLoadString(IDS_PAINT_BRUSH_BASE_HELP), NewObjClear(PaintBrushBase, pParams));
}
