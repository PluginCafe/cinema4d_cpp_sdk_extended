#include "c4d.h"
#include "toolsculpting.h"
#include "c4d_symbols.h"
#include "main.h"

#if 0

//#define USE_TIMER

#define ID_SCULPTING_TOOL	450000250

using namespace cinema;

class SculptingTool : public DescriptionToolData
{
public:
	SculptingTool();
	virtual ~SculptingTool();

private:
	virtual Int32 GetToolPluginId() const { return ID_SCULPTING_TOOL; }
	virtual const String GetResourceSymbol() const { return String("ToolSculpting"); }
	virtual Bool InitTool(BaseDocument* pDoc, BaseContainer& data, BaseThread* bt);
	virtual void FreeTool(BaseDocument* pDoc, BaseContainer& data);
	virtual void InitDefaultSettings(BaseDocument* doc, BaseContainer& data);
	virtual Bool GetCursorInfo(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, Float x, Float y, BaseContainer& bc);
	virtual Bool MouseInput(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, EditorWindow* win, const BaseContainer& msg);

	Bool ValidateViewport(BaseDocument* pDoc, BaseDraw* pDraw);
	void UpdateObject(Vector* pPoints, Float rMouseX, Float rMouseY, Float rRadius, const Vector& vDelta, Bool bAllowVBOUpdate, UInt32 ulUpdateFlags);

	ViewportSelect* m_pViewportSelect;
	Bool						m_bViewportValid;
	BaseDraw*				m_pLastBaseDraw;
	Int32						m_lLastWidth, m_lLastHeight;
	PolygonObject*	m_pLastObject;
	Int32						m_lLastDirty;
	UInt32					m_lLastEditorCameraDirty;
	BaseDocument*		m_pDoc;
	Float						m_rLastMouseX, m_rLastMouseY;
};

SculptingTool::SculptingTool()
{
	m_pViewportSelect = nullptr;
	m_pLastBaseDraw	 = nullptr;
	m_bViewportValid = false;
	m_pLastObject = nullptr;
	m_lLastDirty	= 0;
	m_pDoc = nullptr;
}

SculptingTool::~SculptingTool()
{
	ViewportSelect::Free(m_pViewportSelect);
}

Bool SculptingTool::InitTool(BaseDocument* pDoc, BaseContainer& data, BaseThread* bt)
{
	if (!DescriptionToolData::InitTool(pDoc, data, bt))
		return false;

	m_rLastMouseX = m_rLastMouseY = -1.0f;
	m_bViewportValid = false;
	m_pLastBaseDraw	 = nullptr;
	ViewportSelect::Free(m_pViewportSelect);
	m_lLastWidth	= m_lLastHeight = -1;
	m_pLastObject = nullptr;
	m_lLastDirty	= 0;
	m_pDoc = nullptr;
	return true;
}

void SculptingTool::FreeTool(BaseDocument* pDoc, BaseContainer& data)
{
	m_pLastObject = nullptr;
	m_lLastDirty	= 0;
	ViewportSelect::Free(m_pViewportSelect);
	DescriptionToolData::FreeTool(pDoc, data);
}

void SculptingTool::InitDefaultSettings(BaseDocument* doc, BaseContainer& data)
{
	data.SetFloat(SCULPTING_RADIUS, 40.0);
	data.SetVector(SCULPTING_VECTOR, Vector(0.0, 50.0, 0.0));
	data.SetBool(SCULPTING_DO_VBO_UPDATE, false);
	DescriptionToolData::InitDefaultSettings(doc, data);
}

Bool SculptingTool::GetCursorInfo(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, Float x, Float y, BaseContainer& bc)
{
	if (bc.GetId() != BFM_CURSORINFO_REMOVE)
	{
		if (!pDoc)
			return true;

		if (!pDoc->IsCacheBuilt())
			return true;

		if (pDraw != pDoc->GetActiveBaseDraw())
			return true;

		if (!ValidateViewport(pDoc, pDraw))
			return false;
	}
	else
	{
		SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);	// FIX[40724]
	}

	return true;
}

Bool SculptingTool::MouseInput(BaseDocument* pDoc, BaseContainer& data, BaseDraw* pDraw, EditorWindow* win, const BaseContainer& msg)
{
	if (!pDoc)
		return true;
	if (pDraw != pDoc->GetActiveBaseDraw())
		return true;

	if (!m_pLastObject)
		return true;

	Int32 lMouseX = msg.GetInt32(BFM_INPUT_X);
	Int32 lMouseY = msg.GetInt32(BFM_INPUT_Y);
	Int32 lLeft, lTop, lRight, lBottom;
	pDraw->GetFrame(&lLeft, &lTop, &lRight, &lBottom);

	if (!ValidateViewport(pDoc, pDraw))
		return false;

	Float					rMouseX = (Float)lMouseX;	Float rMouseY = (Float)lMouseY;
	Float					dx, dy;
	Bool					bFirst = true;
	BaseContainer bcDevice;
	Vector*				pvPoints = m_pLastObject->GetPointW();
	Float					rRadius	 = data.GetFloat(SCULPTING_RADIUS);
	Bool					bAllowVBOUpdate = data.GetBool(SCULPTING_DO_VBO_UPDATE);
	Vector				vMove = data.GetVector(SCULPTING_VECTOR);
	UInt32				ulUpdateFlags;

	if ((ulUpdateFlags = m_pLastObject->VBOInitUpdate(pDraw)) == 0)
	{
		// update the object so that triangle strips are deleted
		m_pLastObject->Message(MSG_UPDATE);
		DrawViews(DRAWFLAGS::ONLY_ACTIVE_VIEW | DRAWFLAGS::NO_THREAD | DRAWFLAGS::NO_ANIMATION);
		if ((ulUpdateFlags = m_pLastObject->VBOInitUpdate(pDraw)) == 0)
			return false;
	}

	win->MouseDragStart(KEY_MLEFT, rMouseX, rMouseY, MOUSEDRAGFLAGS::DONTHIDEMOUSE);
	pDoc->StartUndo();
	pDoc->AddUndo(UNDOTYPE::CHANGE, m_pLastObject);
	while (win->MouseDrag(&dx, &dy, &bcDevice) == MOUSEDRAGRESULT::CONTINUE)
	{
		if (!bFirst && dx == 0.0f && dy == 0.0f)
			continue;
		bFirst	 = false;
		rMouseX += dx;
		rMouseY += dy;
		UpdateObject(pvPoints, rMouseX, rMouseY, rRadius, vMove, bAllowVBOUpdate, ulUpdateFlags);
#ifdef USE_TIMER
		Int32 lTimer = GeGetTimer();
#endif
		DrawViews(DRAWFLAGS::ONLY_ACTIVE_VIEW | DRAWFLAGS::NO_THREAD | DRAWFLAGS::NO_ANIMATION);
#ifdef USE_TIMER
		ApplicationOutput("DrawViews: " + String::IntToString(GeGetTimer() - lTimer));
#endif
	}
	win->MouseDragEnd();
	m_pLastObject->Message(MSG_UPDATE);
	pDoc->EndUndo();

	m_pLastObject->VBOFreeUpdate();

	DrawViews(DRAWFLAGS::ONLY_ACTIVE_VIEW | DRAWFLAGS::NO_THREAD | DRAWFLAGS::NO_ANIMATION);
	SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);
	return true;
}

Bool SculptingTool::ValidateViewport(BaseDocument* pDoc, BaseDraw* pDraw)
{
	Int32 lLeft, lTop, lRight, lBottom;
	pDraw->GetFrame(&lLeft, &lTop, &lRight, &lBottom);
	lRight	= lRight - lLeft + 1;
	lBottom = lBottom - lTop + 1;
	BaseObject* pCam = pDraw->GetSceneCamera(pDoc) ? pDraw->GetSceneCamera(pDoc) : pDraw->GetEditorCamera();
	Int32				lDirty;
	BaseObject* pSelected;

	if (!pDoc->IsCacheBuilt())
		return false;

	pSelected = pDoc->GetActiveObject();
	if (pSelected && pSelected->IsInstanceOf(Opolygon))
	{
		lDirty = pSelected->GetDirty(DIRTYFLAGS::DATA | DIRTYFLAGS::MATRIX);
	}
	else
	{
		pSelected = nullptr;
		lDirty = 0;
		m_pLastObject = nullptr;
	}

	if (pDraw != m_pLastBaseDraw)
		m_bViewportValid = false;
	if (m_bViewportValid && (lRight != m_lLastWidth || lBottom != m_lLastHeight))
		m_bViewportValid = false;
	if (m_bViewportValid && !(pSelected == m_pLastObject && lDirty == m_lLastDirty))
		m_bViewportValid = false;
	if (m_bViewportValid && m_lLastEditorCameraDirty != pCam->GetDirty(DIRTYFLAGS::MATRIX | DIRTYFLAGS::DATA))
		m_bViewportValid = false;
	if (m_bViewportValid && m_pDoc != pDoc)
		m_bViewportValid = false;

	if (!m_bViewportValid)
	{
		ViewportSelect::Free(m_pViewportSelect);
	}

	if (!m_pViewportSelect && pSelected)
	{
		m_pLastObject = (PolygonObject*)pSelected;
		m_lLastDirty	= lDirty;
		m_lLastWidth	= lRight;
		m_lLastHeight = lBottom;
		m_pLastBaseDraw = pDraw;
		m_pViewportSelect = ViewportSelect::Alloc();
		m_bViewportValid	= true;
		m_rLastMouseX = m_rLastMouseY = -1.0f;
		m_lLastEditorCameraDirty = pCam->GetDirty(DIRTYFLAGS::DATA | DIRTYFLAGS::MATRIX);
		m_pDoc = pDoc;
		if (!m_pViewportSelect)
			return false;
		if (m_pLastObject)
		{
			if (!m_pViewportSelect->Init(m_lLastWidth, m_lLastHeight, pDraw, m_pLastObject, Mpoints, false, VIEWPORTSELECTFLAGS::NONE))
				return false;
		}
	}
	return true;
}

static Float SQR(Float r)
{
	return r * r;
}

void SculptingTool::UpdateObject(Vector* pvPoints, Float rMouseX, Float rMouseY, Float rRadius, const Vector& vDelta, Bool bAllowVBOUpdate, UInt32 ulUpdateFlags)
{
	Int32 x1 = LMax(0, (Int32)(rMouseX - rRadius));
	Int32 x2 = LMin(m_lLastWidth - 1, (Int32)(rMouseX + rRadius));
	Int32 y1 = LMax(0, (Int32)(rMouseY - rRadius));
	Int32 y2 = LMin(m_lLastHeight - 1, (Int32)(rMouseY + rRadius));
	Int32 x, y;
	const GlVertexBufferAttributeInfo* pVertexInfo;
	Float rRadSqr = rRadius * rRadius;

	if (rRadSqr < 1.0)
		return;

#ifdef USE_TIMER
	Int32 lTimer = GeGetTimer(), lTimer1 = lTimer;
#endif
	if (bAllowVBOUpdate)
	{
		if (!m_pLastObject->VBOStartUpdate(m_pLastBaseDraw, VBWriteOnly, true))
		{
			bAllowVBOUpdate = false;
		}
		else
		{
#ifdef USE_TIMER
			ApplicationOutput("VBOStartUpdate: " + String::IntToString(GeGetTimer() - lTimer));
#endif
		}
	}

#ifdef USE_TIMER
	lTimer = GeGetTimer();
#endif
	if (ulUpdateFlags & POLYOBJECT_VBO_VERTEX)
		pVertexInfo = m_pLastObject->VBOUpdateVectorGetAttribute(POLYOBJECT_VBO_VERTEX);
	else
		pVertexInfo = nullptr;
	for (x = x1; x <= x2; x++)
	{
		for (y = y1; y <= y2; y++)
		{
			Float rSqrDist = SQR(x - rMouseX) + SQR(y - rMouseY);
			if (rSqrDist > rRadSqr)
				continue;

			const ViewportPixel* pPixel = m_pViewportSelect->GetPixelInfoPoint(x, y);
			while (pPixel)
			{
				if (pPixel->op == m_pLastObject)
				{
					pvPoints[pPixel->i] += vDelta * Smoothstep(0.0, 1.0, (1.0 - rSqrDist / rRadSqr));
					if (bAllowVBOUpdate)
					{
						if (pVertexInfo)
							m_pLastObject->VBOUpdateVector(pPixel->i, (Vector32)pvPoints[pPixel->i], pVertexInfo);
						// updating the normals is left as a task for the user ;-)
					}
				}
				pPixel = pPixel->next;
			}
		}
	}

#ifdef USE_TIMER
	if (bAllowVBOUpdate)
		ApplicationOutput("VBOUpdateVector: " + String::IntToString(GeGetTimer() - lTimer));
#endif

	if (bAllowVBOUpdate)
	{
		m_pLastObject->Message(MSG_UPDATE);	// must be called before VBOEndUpdate
#ifdef USE_TIMER
		lTimer = GeGetTimer();
#endif

		m_pLastObject->VBOEndUpdate(m_pLastBaseDraw);

#ifdef USE_TIMER
		ApplicationOutput("VBOEndUpdate: " + String::IntToString(GeGetTimer() - lTimer));
#endif
	}
	else
	{
		m_pLastObject->Message(MSG_UPDATE);
	}

#ifdef USE_TIMER
	ApplicationOutput("SculptingTool::UpdateObject " + String::IntToString(GeGetTimer() - lTimer1));
#endif
}

#endif

cinema::Bool RegisterSculptingTool()
{
	return true;
	//return RegisterToolPlugin(ID_SCULPTING_TOOL, GeLoadString(IDS_SCULPTING_TOOL), PLUGINFLAG_TOOL_NO_WIREFRAME, nullptr, GeLoadString(IDS_SCULPTING_TOOL), NewObjClear(SculptingTool));
}
