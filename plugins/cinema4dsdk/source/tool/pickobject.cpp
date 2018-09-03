#include "c4d.h"
#include "c4d_symbols.h"
#include "toolpickobjectsdk.h"
#include "main.h"

#define ID_SAMPLE_PICK_OBJECT_TOOL 450000263

class PickObjectTool : public DescriptionToolData
{
public:
	virtual Int32	GetToolPluginId() { return ID_SAMPLE_PICK_OBJECT_TOOL; }
	virtual const String GetResourceSymbol() { return String("ToolPickObjectSDK"); }

	virtual void InitDefaultSettings(BaseDocument* doc, BaseContainer& data);
	virtual Bool MouseInput(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, EditorWindow* win, const BaseContainer& msg);
	virtual Int32	GetState(BaseDocument* doc);
	virtual Bool GetCursorInfo(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, Float x, Float y, BaseContainer& bc);
	virtual TOOLDRAW Draw(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, BaseDrawHelp* bh, BaseThread* bt, TOOLDRAWFLAGS flags);
	virtual Bool GetDDescription(BaseDocument* doc, BaseContainer& data, Description* description, DESCFLAGS_DESC& flags);

private:
	Vector GetWorldCoordinates(BaseDraw* bd, const Matrix4d& m, Float x, Float y, Float z);

	BaseDraw* _lastBaseDraw;
	Int32			_mouseX, _mouseY;
};

void PickObjectTool::InitDefaultSettings(BaseDocument* doc, BaseContainer& data)
{
	_lastBaseDraw = nullptr;
	_mouseX = -1;
	_mouseY = -1;

	data.SetInt32(MDATA_PICKOBJECT_MODE, MDATA_PICKOBJECT_MODE_CIRCLE);
	data.SetInt32(MDATA_PICKOBJECT_CIRCLE_RAD, 40);
	data.SetInt32(MDATA_PICKOBJECT_RECT_W, 50);
	data.SetInt32(MDATA_PICKOBJECT_RECT_H, 30);

	DescriptionToolData::InitDefaultSettings(doc, data);
}

Bool PickObjectTool::MouseInput(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, EditorWindow* win, const BaseContainer& msg)
{
	Int32						mode = data.GetInt32(MDATA_PICKOBJECT_MODE);
	Int32						x, y, l, xr = 0, yr = 0, wr = 0, hr = 0;
	Matrix4d				m;
	const ViewportPixel*const* pix = nullptr;
	String					str;
	char ch[200];
	Bool ret = false;
	AutoAlloc<C4DObjectList> list;
	if (!list)
		return false;

	VIEWPORT_PICK_FLAGS flags = VIEWPORT_PICK_FLAGS::ALLOW_OGL | VIEWPORT_PICK_FLAGS::USE_SEL_FILTER;
	if (data.GetBool(MDATA_PICKOBJECT_ONLY_VISIBLE))
		flags |= VIEWPORT_PICK_FLAGS::OGL_ONLY_VISIBLE;
	x = msg.GetInt32(BFM_INPUT_X);
	y = msg.GetInt32(BFM_INPUT_Y);
	Float64 timer = 0.0;
	if (mode == MDATA_PICKOBJECT_MODE_CIRCLE)
	{
		Int32 rad = data.GetInt32(MDATA_PICKOBJECT_CIRCLE_RAD);
		timer = GeGetMilliSeconds();
		ret = ViewportSelect::PickObject(bd, doc, x, y, rad, xr, yr, wr, hr, pix, flags, nullptr, list, &m);
		timer = GeGetMilliSeconds() - timer;
	}
	else if (mode == MDATA_PICKOBJECT_MODE_RECTANGLE)
	{
		Int32 width	 = data.GetInt32(MDATA_PICKOBJECT_RECT_W);
		Int32 height = data.GetInt32(MDATA_PICKOBJECT_RECT_H);
		x -= width / 2;
		y -= height / 2;
		timer = GeGetMilliSeconds();
		ret = ViewportSelect::PickObject(bd, doc, x, y, x + width, y + height, xr, yr, wr, hr, pix, flags, nullptr, list, &m);
		timer = GeGetMilliSeconds() - timer;
	}
	if (ret)
	{
		sprintf(ch, "Picking region from (%d, %d), size (%d, %d)|", xr, yr, wr, hr);
		str += ch;
		for (l = 0; l < list->GetCount(); l++)
		{
			sprintf(ch, ", z = %.4f|", list->GetZ(l));
			str += "Found Object " + list->GetObject(l)->GetName() + ch;
		}
	}
	else
	{
		str	= "PickObject failed";
	}
	sprintf(ch, "|Time: %.2f us", float(timer) * 1000.0f);
	str += ch;

	DeleteMem(pix);
	GeOutString(str, GEMB::OK);

	return true;
}

Int32 PickObjectTool::GetState(BaseDocument* doc)
{
	return CMD_ENABLED;
}

Vector PickObjectTool::GetWorldCoordinates(BaseDraw* bd, const Matrix4d& m, Float x, Float y, Float z)
{
	// pick object returns the view-projection matrix. This transforms a point in camera space into clip space.

	Int32		 l, t, r, b, w, h;
	Vector4d pos;
	Vector	 posWorld;

	bd->GetFrame(&l, &t, &r, &b);
	if (l == r || b == t)
		return Vector(0.0);

	w = r - l;
	h = b - t;

	// first, transform the points into clip space
	pos.x = (x - Float(l)) / Float(w);
	pos.y = (y - Float(t)) / Float(h);
	pos.z = z;
	pos.w = 1.0;
	pos = pos * 2.0f - Vector4d(1.0f);
	pos.y = -pos.y;

	// apply the inverse view transform
	Matrix4d im = GetGlInverseMatrix(m);
	pos = GlMultiply(im, pos);
	pos.NormalizeW();

	// convert it into a 3-tuple
	posWorld = bd->GetMg() * pos.GetVector3();

	return posWorld;
}

Bool PickObjectTool::GetCursorInfo(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, Float x, Float y, BaseContainer& bc)
{
	if (bc.GetId() == BFM_CURSORINFO_REMOVE)
	{
		_lastBaseDraw = nullptr;
	}
	else
	{
		_lastBaseDraw = bd;
		_mouseX = (Int32)x;
		_mouseY = (Int32)y;

		AutoAlloc<C4DObjectList> list;
		if (list)
		{
			// get the z position of the topmost object. The z range for objects is from -1 to 1.
			Float		 z = 1.0;
			String	 str;
			Matrix4d m;
			ViewportSelect::PickObject(bd, doc, _mouseX, _mouseY, 1, VIEWPORT_PICK_FLAGS::ALLOW_OGL | VIEWPORT_PICK_FLAGS::USE_SEL_FILTER | VIEWPORT_PICK_FLAGS::OGL_ONLY_TOPMOST, nullptr, list, &m);
			if (list->GetCount() > 0)
				z = list->GetZ(0);
			if (z < 1.0)
			{
				Vector v = GetWorldCoordinates(bd, m, x, y, z);
				char	 ch[200];
				sprintf(ch, "Mouse coordinates: (%d, %d), world coordinates: (%.4f, %.4f, %.4f)", _mouseX, _mouseY, v.x, v.y, v.z);
				str = ch;
			}
			else
			{
				str = "Mouse cursor is not over an object";
			}
			StatusSetText(str);
		}
	}
	SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);
	return true;
}

TOOLDRAW PickObjectTool::Draw(BaseDocument* doc, BaseContainer& data, BaseDraw* bd, BaseDrawHelp* bh, BaseThread* bt, TOOLDRAWFLAGS flags)
{
	if ((flags & TOOLDRAWFLAGS::HIGHLIGHT) && _lastBaseDraw == bd)
	{
		Int32	 mode = data.GetInt32(MDATA_PICKOBJECT_MODE);
		Vector col(1.0);
		bd->SetMatrix_Screen();
		bd->SetPen(col);
		if (mode == MDATA_PICKOBJECT_MODE_CIRCLE)
		{
			Matrix m;
			Float	 rad = (Float)data.GetInt32(MDATA_PICKOBJECT_CIRCLE_RAD);
			m.off = Vector((Float)_mouseX, (Float)_mouseY, 0.0);
			m.sqmat.v1 *= rad;
			m.sqmat.v2 *= rad;
			bd->DrawCircle(m);
		}
		else if (mode == MDATA_PICKOBJECT_MODE_RECTANGLE)
		{
			Int32 width	 = data.GetInt32(MDATA_PICKOBJECT_RECT_W);
			Int32 height = data.GetInt32(MDATA_PICKOBJECT_RECT_H);
			Int32 x1 = _mouseX - width / 2;
			Int32 y1 = _mouseY - height / 2;
			Int32 x2 = x1 + width;
			Int32 y2 = y1 + height;
			bd->LineStripBegin();
			bd->LineStrip(Vector((Float)x1, (Float)y1, 0.0), col, 0);
			bd->LineStrip(Vector((Float)x2, (Float)y1, 0.0), col, 0);
			bd->LineStrip(Vector((Float)x2, (Float)y2, 0.0), col, 0);
			bd->LineStrip(Vector((Float)x1, (Float)y2, 0.0), col, 0);
			bd->LineStrip(Vector((Float)x1, (Float)y1, 0.0), col, 0);
			bd->LineStripEnd();
		}
		return TOOLDRAW::HIGHLIGHTS;
	}
	return TOOLDRAW::NONE;
}

Bool PickObjectTool::GetDDescription(BaseDocument* doc, BaseContainer& data, Description* description, DESCFLAGS_DESC& flags)
{
	Bool res = DescriptionToolData::GetDDescription(doc, data, description, flags);
	if (flags & DESCFLAGS_DESC::LOADED)
	{
		BaseContainer* bc;

		Int32 mode = data.GetInt32(MDATA_PICKOBJECT_MODE);

		bc = description->GetParameterI(DescLevel(MDATA_PICKOBJECT_CIRCLE_RAD), nullptr);
		if (bc)
			bc->SetInt32(DESC_HIDE, mode != MDATA_PICKOBJECT_MODE_CIRCLE);

		bc = description->GetParameterI(DescLevel(MDATA_PICKOBJECT_RECT_W), nullptr);
		if (bc)
			bc->SetInt32(DESC_HIDE, mode == MDATA_PICKOBJECT_MODE_CIRCLE);
		bc = description->GetParameterI(DescLevel(MDATA_PICKOBJECT_RECT_H), nullptr);
		if (bc)
			bc->SetInt32(DESC_HIDE, mode == MDATA_PICKOBJECT_MODE_CIRCLE);
	}
	return res;
}

Bool RegisterPickObjectTool()
{
	return RegisterToolPlugin(ID_SAMPLE_PICK_OBJECT_TOOL, GeLoadString(IDS_PICKOBJECT_SDK), PLUGINFLAG_TOOL_OBJECTHIGHLIGHT, nullptr, GeLoadString(IDS_HELP_PICKOBJECT_SDK), NewObjClear(PickObjectTool));
}
