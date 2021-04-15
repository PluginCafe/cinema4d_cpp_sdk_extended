// example code for a menu/manager plugin

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_ACTIVEOBJECT 1000472

#include "c4d.h"
#include "c4d_symbols.h"
#include "main.h"
#include "maxon/sortedarray.h"
#include "maxon/utilities/sprintf_safe.h"

static void ShowObjectProps(BaseList2D* obj);

class ListHeader;
//---------------------------------------------
class ListObj
{
	ListObj*		next;
	ListObj*		prev;
	ListHeader* up;

public:
	ListObj();
	virtual ~ListObj();

	Bool AddObj(ListHeader* header);
	Bool RemObj(ListHeader* header, Bool delme);
	Bool FindObj(ListHeader* header);

	ListObj* GetNext();
	ListObj* GetPrev();
	ListHeader* GetUp();

	friend class ListHeader;
};
//---------------------------------------------
class ListHeader
{
	ListObj* first;
	ListObj* last;

public:
	ListHeader();
	~ListHeader();

	Bool AddObj(ListObj* newobj);
	Bool RemObj(ListObj* newobj, Bool delme);
	Bool FreeList(Bool delentries);

	ListObj* GetFirst();

	Int32 GetCount();
};

//---------------------------------------------
ListObj::ListObj()
{
	next = prev = nullptr;
	up = nullptr;
}
ListObj::~ListObj()
{
	if (up)
		RemObj(up, false);
	prev = next = nullptr;
	up = nullptr;
}
Bool ListObj::AddObj(ListHeader* header)
{
	return header->AddObj(this);
}
Bool ListObj::RemObj(ListHeader* header, Bool delme)
{
	return header->RemObj(this, delme);
}
Bool ListObj::FindObj(ListHeader* header)
{
	return false;
}
ListObj* ListObj::GetNext()
{
	return next;
}
ListObj* ListObj::GetPrev()
{
	return prev;
}
ListHeader* ListObj::GetUp()
{
	return up;
}
//---------------------------------------------
ListHeader::ListHeader()
{
	first = last = nullptr;
}
ListHeader::~ListHeader()
{
	FreeList(false);
}
Bool ListHeader::AddObj(ListObj* newobj)
{
	if (last)
		last->next = newobj;
	newobj->up = this;
	newobj->prev = last;
	last = newobj;
	if (!first)
		first = newobj;
	return true;
}
Bool ListHeader::RemObj(ListObj* oldobj, Bool delme)
{
	// ausklinken
	if (oldobj->prev)
		oldobj->prev->next = oldobj->next;
	if (oldobj->next)
		oldobj->next->prev = oldobj->prev;

	// header korrigieren
	if (oldobj == first)
		first = oldobj->next;
	if (oldobj == last)
		last = oldobj->prev;

	oldobj->prev = oldobj->next = nullptr;
	oldobj->up = nullptr;

	// wenn gewnscht, Speicher freigeben
	if (delme)
		DeleteObj(oldobj);

	return true;
}
ListObj* ListHeader::GetFirst()
{
	return first;
}
Bool ListHeader::FreeList(Bool delentries)
{
	while (first)
	{
		RemObj(first, delentries);
	}
	return true;
}
Int32 ListHeader::GetCount()
{
	ListObj* temp;
	Int32		 count = 0;
	for (temp = GetFirst(); temp; temp = temp->GetNext())
		count++;
	return count;
}
//---------------------------------------------




struct DebugNode : public ListObj
{
	ListHeader					down;
	GeListNode*					ptr;
	String							name;
	Bool								open;
	AutoAlloc<BaseLink> link;

	Bool								diff[(UInt32)HDIRTY_ID::MAX + 1];
	UInt32							hdirty[(UInt32)HDIRTY_ID::MAX + 1];
};

class DebugArray : public maxon::SortedArray<DebugArray, maxon::BaseArray<DebugNode*> >
{
public:
	DebugArray()
	{
	}

	DebugArray(DebugArray && src) : maxon::SortedArray<DebugArray, maxon::BaseArray<DebugNode*> >(std::move(src))
	{
	}

	MAXON_OPERATOR_MOVE_ASSIGNMENT(DebugArray)

	static Bool LessThan(DebugNode* a, DebugNode* b)
	{
		return a->ptr < b->ptr;
	}

	static Bool LessThan(GeListNode* a, DebugNode* b)
	{
		return a < b->ptr;
	}

	static Bool IsEqual(GeListNode* a, DebugNode* b)
	{
		return a == b->ptr;
	}

	void FlushAll()
	{
		Int32 i;
		for (i = 0; i < GetCount(); i++)
		{
			DebugNode* node = operator[](i);
			DeleteObj(node);
		}
		Flush();
	}
};

static Bool BuildTree(DebugNode* parent, DebugArray& oldlist, DebugArray& newlist, GeListNode* node, const String& defname)
{
	DebugNode** nn = oldlist.FindValue(node);
	DebugNode*	n	 = nn ? *nn : nullptr;
	if (n)
	{
		ListHeader* h = n->GetUp();
		if (h)
			h->RemObj(n, false);

		iferr (oldlist.Erase(oldlist.GetIndex(*nn)))	// GetIndex needs *nn instead of n
      return false;
	}

	if (!n)
	{
		n = NewObjClear(DebugNode);
		if (!n)
			return false;

		n->ptr	= node;
		n->open = false;
		n->link->SetLink(node);
	}

	iferr (newlist.Append(n))
    return false;
	parent->down.AddObj(n);

	if (node->IsInstanceOf(Tbasedocument))
		n->name = "DOC: " + ((BaseDocument*)node)->GetDocumentName().GetString();
	else if (node->IsInstanceOf(Tbaselist2d))
		n->name = defname + ((BaseList2D*)node)->GetName();
	else
		n->name = defname;

	// UIDs
	if (node->IsInstanceOf(Tbaselist2d))
	{
		const Char* mem = nullptr;
		Int32				uID = 0;
		Int					bytes = 0;
		Char				cTxt[3];

		// go through all available UIDs
		for (Int32 uIdx = 0; uIdx < ((BaseList2D*)node)->GetUniqueIDCount(); uIdx++)
		{
			iferr (DebugNode* uidNode = NewObj(DebugNode))
				return false;

			if (!uidNode)
				return false;

			uidNode->open = FALSE;

			// get the UID (size and data)
			if (((BaseList2D*)node)->GetUniqueIDIndex(uIdx, uID, mem, bytes))
			{
				uidNode->name = "UID [APPID " + String::IntToString( uID ) + "] - [";

				for (Int32 pos = 0; mem && pos < bytes; pos++)
				{
					Char cVal = mem[pos];

					// print as 2 digit hex value
					sprintf_safe(cTxt, sizeof(cTxt), "%02X", (UChar)cVal );
					uidNode->name += String(cTxt);

					// add "-" after 4 digits
					if (((pos + 1) % 2) == 0 && pos + 1 < bytes)
						uidNode->name += "-";
				}
				uidNode->name += "]";
			}
			else
			{
				uidNode->name = "UID <not found at index " + String::IntToString(uIdx) + ">";
			}

			iferr (newlist.Append(uidNode))
				return false;

			n->down.AddObj(uidNode);
		}
	}

	Int32	 i;
	UInt32 sum = 0;
	for (i = 0; i < (Int32)HDIRTY_ID::MAX; i++)
	{
		UInt32 hdirty = node->GetHDirty((HDIRTYFLAGS)((1 << 31) | (1 << i)));
		n->diff[i] = n->hdirty[i] != hdirty;
		n->hdirty[i] = hdirty;
		sum += hdirty;
	}
	sum += node->GetDirty(DIRTYFLAGS::ALL);

	n->diff[i] = n->hdirty[(Int32)HDIRTY_ID::MAX] != sum;
	n->hdirty[(Int32)HDIRTY_ID::MAX] = sum;

	// Handle Scene Nodes.
	// See in the plugin Project Hooks -> Scene Nodes -> CLASSIC_REPRESENTATION:
	static constexpr Int32 SCENEHOOK_NODE_SCENE_ID = 1054188;
	static constexpr Int32 MSG_UPDATE_LEGACY_OBJECTS = 180420109;

	if (node->GetType() == SCENEHOOK_NODE_SCENE_ID)
	{
		// Request Legacy Object representation
		BaseObject* sceneNodeRoot = nullptr;
		node->Message(MSG_UPDATE_LEGACY_OBJECTS, &sceneNodeRoot);

		// To have a nullptr here is no error, there's just no node scene graph.
		if (sceneNodeRoot != nullptr)
		{
			if (!BuildTree(n, oldlist, newlist, sceneNodeRoot, String("CLASSIC_REPRESENTATION: ")))
				return false;
		}
	}

	if (node->IsInstanceOf(Obase))
	{
		BaseObject* obj = static_cast<BaseObject*>(node);
		BaseObject* cc = obj->GetCache(nullptr);
		if (cc)
			if (!BuildTree(n, oldlist, newlist, cc, String("CACHE: ")))
				return false;

		cc = obj->GetDeformCache();
		if (cc)
			if (!BuildTree(n, oldlist, newlist, cc, String("DEFORM: ")))
				return false;

		cc = obj->GetIsoparm();
		if (cc)
			if (!BuildTree(n, oldlist, newlist, cc, String("ISOPARM: ")))
				return false;
	}

	if (node->GetDown())
	{
		GeListNode* c = node->GetDown();
		for (; c; c = c->GetNext())
		{
			if (!BuildTree(n, oldlist, newlist, c, String()))
				return false;
		}
	}

	BranchInfo info[20];
	Int32			 cnt = node->GetBranchInfo(info, 20, GETBRANCHINFO::NONE);
	for (i = 0; i < cnt; i++)
	{
		if (info[i].name.IsPopulated())
		{
			if (!BuildTree(n, oldlist, newlist, info[i].head, info[i].name))
				return false;
		}
		else
		{
			if (!BuildTree(n, oldlist, newlist, info[i].head, String("ListHead")))
				return false;
		}
	}

	return true;
}


class Function2 : public TreeViewFunctions
{
public:
	virtual void*	GetFirst(void* root, void* userdata)
	{
		DebugNode* node = (DebugNode*)root;
		return node->down.GetFirst();
	}

	virtual void*	GetNext(void* root, void* userdata, void* obj)
	{
		DebugNode* node = (DebugNode*)obj;
		return node->GetNext();
	}

	virtual void* GetPred(void* root, void* userdata, void* obj)
	{
		DebugNode* node = (DebugNode*)obj;
		return node->GetPrev();
	}

	virtual void*	GetDown(void* root, void* userdata, void* obj)
	{
		DebugNode* node = (DebugNode*)obj;
		return node->down.GetFirst();
	}

	virtual Bool IsSelected(void* root, void* userdata, void* obj)
	{
		//DebugNode *node = (DebugNode*)obj;
		return false;
	}

	virtual String GetName(void* root, void* userdata, void* obj)
	{
		DebugNode* node = (DebugNode*)obj;
		return node->name;
	}

	virtual Int GetId(void* root, void* userdata, void* obj)
	{
		return (Int)obj;
	}

	virtual Bool IsOpened(void* root, void* userdata, void* obj)
	{
		DebugNode* node = (DebugNode*)obj;
		return node->open;
	}

	virtual void Open(void* root, void* userdata, void* obj, Bool onoff)
	{
		DebugNode* node = (DebugNode*)obj;
		node->open = onoff;
	}

	virtual void Select(void* root, void* userdata, void* obj, Int32 mode)
	{
		DebugNode*	 node = (DebugNode*)obj;
		C4DAtomGoal* link = node->link->GetLinkAtom(nullptr);

		if (!link || !link->IsInstanceOf(Tbaselist2d))
			return;

		if (link->IsInstanceOf(Obase))
		{
			BaseObject*		op	= (BaseObject*)link;
			BaseDocument* doc = op->GetDocument();
			if (doc)
				doc->SetActiveObject(op, mode);
			EventAdd();
		}
		else if (link->IsInstanceOf(Mbase))
		{
			BaseMaterial* op	= (BaseMaterial*)link;
			BaseDocument* doc = op->GetDocument();
			if (doc)
				doc->SetActiveMaterial(op);
			EventAdd();
		}
		ShowObjectProps((BaseList2D*)link);
	}

	virtual Int32 AcceptDragObject(void* root, void* userdata, void* obj, Int32 dragtype, void* dragobject, Bool& bAllowCopy)
	{
		return 0;
	}

	virtual void InsertObject(void* root, void* userdata, void* obj, Int32 dragtype, void* dragobject, Int32 insertmode, Bool bCopy)
	{
	}

	virtual Int32 GetDragType(void* root, void* userdata, void* obj)
	{
		return NOTOK;
	}

	virtual Int32 DoubleClick(void* root, void* userdata, void* obj, Int32 col, MouseInfo* mouseinfo)
	{
		return false;
	}

	virtual Bool MouseDown(void* root, void* userdata, void* obj, Int32 col, MouseInfo* mouseinfo, Bool rightButton)
	{
		return false;
	}

	virtual Int32 GetHeaderColumnWidth(void* root, void* userdata, Int32 col, GeUserArea* ua)
	{
		if (!ua)
			return 0;
		return ua->DrawGetTextWidth("8888"_s) + 4;
	}

	virtual Int32 GetColumnWidth(void* root, void* userdata, void* obj, Int32 col, GeUserArea* pArea)
	{
		if (col == 'icon')
		{
			return pArea->DrawGetFontHeight() + 4;
		}
		if (col == 'addr' || col == 'drtL')
		{
			return pArea->DrawGetTextWidth("0x8888888888888888"_s) + 12;
		}
		if (col == 'type')
		{
			return pArea->DrawGetTextWidth("9999999"_s) + 12;
		}
		if ((col >> 8) == C4D_FOUR_BYTE(0, 'd', 'r', 't'))
		{
			return pArea->DrawGetTextWidth("8888"_s);
		}
		return 0;
	}

	virtual Int32 GetLineHeight(void* root, void* userdata, void* obj, Int32 col, GeUserArea* pArea)
	{
		return pArea->DrawGetFontHeight();
	}

	Bool DrawHeaderCell(void* root, void* userdata, Int32 col, DrawInfo* drawinfo)
	{
		String name;
		drawinfo->frame->DrawSetPen(COLOR_BG);
		drawinfo->frame->DrawRectangle(drawinfo->xpos, drawinfo->ypos, drawinfo->xpos + drawinfo->width - 1, drawinfo->ypos + drawinfo->height - 1);
		drawinfo->frame->DrawSetFont(FONT_STANDARD);

		switch (col)
		{
			case 'tree':	name = "Scene"; break;
			case 'icon':	name = String(); break;
			case 'type':	name = "Type"; break;
			case 'addr':	name = "Address"; break;
			case 'bits':	name = "Gen"; break;
			case 'drt0':	name = "All"; break;
			case 'drt1':	name = "Anim"; break;
			case 'drt2':	name = "Obj"; break;
			case 'drt3':	name = "OMatrix"; break;
			case 'drt4':	name = "OHierarchie"; break;
			case 'drt5':	name = "Tag"; break;
			case 'drt6':	name = "Mat"; break;
			case 'drt7':	name = "Shd"; break;
			case 'drt8':	name = "Rend"; break;
			case 'drt9':	name = "VP"; break;
			case 'drta':	name = "Filter"; break;
			case 'drtb':	name = "NBit"; break;
			case 'drtL':	name = "Mask"; break;
		}

		drawinfo->frame->DrawSetTextCol(COLOR_TEXT, COLOR_BG);
		drawinfo->frame->DrawText(name, drawinfo->xpos + 4, drawinfo->ypos);

		drawinfo->frame->DrawSetPen(COLOR_EDGEWH);
		drawinfo->frame->DrawLine(drawinfo->xpos, drawinfo->ypos, drawinfo->xpos + drawinfo->width - 1, drawinfo->ypos);
		drawinfo->frame->DrawLine(drawinfo->xpos, drawinfo->ypos, drawinfo->xpos, drawinfo->ypos + drawinfo->height - 1);

		drawinfo->frame->DrawSetPen(COLOR_EDGEDK);
		drawinfo->frame->DrawLine(drawinfo->xpos + drawinfo->width - 1, drawinfo->ypos, drawinfo->xpos + drawinfo->width - 1, drawinfo->ypos + drawinfo->height - 1);
		drawinfo->frame->DrawLine(drawinfo->xpos, drawinfo->ypos + drawinfo->height - 1, drawinfo->xpos + drawinfo->width - 1, drawinfo->ypos + drawinfo->height - 1);

		return true;
	}

	virtual void DrawCell(void* root, void* userdata, void* obj, Int32 col, DrawInfo* drawinfo, const GeData& bgColor)
	{
		DebugNode*	 node = (DebugNode*)obj;
		C4DAtomGoal* link = node->link->GetLinkAtom(nullptr);

		Int32 wx, wy, wh, ww;
		wx = drawinfo->xpos;
		wy = drawinfo->ypos;
		ww = drawinfo->width;
		wh = drawinfo->height;

		if (col == 'icon')
		{
			if (link && link->IsInstanceOf(Tbaselist2d))
			{
				BaseObject* op = (BaseObject*)link;
				IconData		icon;
				op->GetIcon(&icon);

				if (ww > 24)
				{
					wx += (ww - 24) / 2; ww = 24;
				}
				if (wh > 24)
				{
					wy += (wh - 24) / 2; wh = 24;
				}

				Int32 drawflags = BMP_ALLOWALPHA | BMP_NORMALSCALED;
				if (icon.flags & ICONDATAFLAGS::APPLYCOLORPROFILE)
					drawflags |= BMP_APPLY_COLORPROFILE;
				if (icon.flags & ICONDATAFLAGS::DISABLED)
					drawflags |= BMP_GRAYEDOUT;
				drawinfo->frame->DrawSetPen(bgColor);
				drawinfo->frame->DrawBitmap(icon.bmp, wx, wy, ww, wh, icon.x, icon.y, icon.w, icon.h, drawflags);
			}
		}
		else if (col == 'type')
		{
			if (link)
			{
				String t = String::IntToString(link->GetType());
				drawinfo->frame->DrawSetTextCol(COLOR_TEXT_DISABLED, bgColor);
				drawinfo->frame->DrawText(t, wx, wy);
			}
		}
		else if (col == 'addr')
		{
			if (link)
			{
				String hex = String::HexToString((UInt)link);
				drawinfo->frame->DrawSetTextCol(COLOR_TEXT_DISABLED, bgColor);
				drawinfo->frame->DrawText(hex, wx, wy);
			}
		}
		else if (col == 'drtL')
		{
			if (link)
			{
				GeData data;
				link->GetParameter(DescID(999999), data, DESCFLAGS_GET::NONE);
				String hex = String::HexToString(UInt32(data.GetInt32()));
				drawinfo->frame->DrawSetTextCol(COLOR_TEXT_DISABLED, bgColor);
				drawinfo->frame->DrawText(hex, wx, wy);
			}
		}
		else if (col == 'bits')
		{
			if (link && link->IsInstanceOf(Tbaselist2d))
			{
				Bool	 bit = ((BaseList2D*)link)->GetBit(BIT_CONTROLOBJECT);
				String hex = String::IntToString(bit);
				drawinfo->frame->DrawSetTextCol(COLOR_TEXT_DISABLED, bgColor);
				drawinfo->frame->DrawText(hex, wx, wy);
			}
		}
		else if ((col >> 8) == C4D_FOUR_BYTE(0, 'd', 'r', 't'))
		{
			if (link)
			{
				Int32	 id = 0;
				UInt32 dirty_cnt = (UInt32) - 1;
				Bool	 dirty = false;

				switch (col)
				{
					case 'drt0':	id = (Int32)HDIRTY_ID::MAX; break;
					case 'drt1':	id = (Int32)HDIRTY_ID::ANIMATION; break;
					case 'drt2':	id = (Int32)HDIRTY_ID::OBJECT; break;
					case 'drt3':	id = (Int32)HDIRTY_ID::OBJECT_MATRIX; break;
					case 'drt4':	id = (Int32)HDIRTY_ID::OBJECT_HIERARCHY; break;
					case 'drt5':	id = (Int32)HDIRTY_ID::TAG; break;
					case 'drt6':	id = (Int32)HDIRTY_ID::MATERIAL; break;
					case 'drt7':	id = (Int32)HDIRTY_ID::SHADER; break;
					case 'drt8':	id = (Int32)HDIRTY_ID::RENDERSETTINGS; break;
					case 'drt9':	id = (Int32)HDIRTY_ID::VP; break;
					case 'drta':	id = (Int32)HDIRTY_ID::FILTER; break;
					case 'drtb':	id = (Int32)HDIRTY_ID::NBITS; break;
				}
				dirty_cnt = node->hdirty[id];
				dirty = node->diff[id];

				if (dirty)
					drawinfo->frame->DrawSetTextCol(Vector(1, 0, 0), COLOR_EDGEWH);
				else
					drawinfo->frame->DrawSetTextCol(COLOR_TEXT, bgColor);

				drawinfo->frame->DrawText(String::UIntToString(dirty_cnt), wx, wy);
			}
		}
	}

	virtual void CreateContextMenu(void* root, void* userdata, void* obj, Int32 lColumn, BaseContainer* bc)
	{
		bc->FlushAll();

		DebugNode* node = (DebugNode*)obj;
		C4DAtomGoal* link = node->link->GetLinkAtom(nullptr);
		if (!link)
			return;

		switch (lColumn)
		{
			case 'tree':
				if (link->IsInstanceOf(Obase))
				{
					bc->InsData(EXTRACT_OBJECT_TO_SCENE, GeData("Extract to scene"));
				}
				break;
		}
	}

	Bool ContextMenuCall(void* root, void* userdata, void* obj, Int32 lColumn, Int32 lCommand)
	{
		DebugNode* node = (DebugNode*)obj;
		C4DAtomGoal* link = node->link->GetLinkAtom(nullptr);
		if (!link)
			return true;

		switch (lCommand)
		{
			case EXTRACT_OBJECT_TO_SCENE:
			{
				Bool needsEventAdd = false;
				if (link->IsInstanceOf(Obase))
				{
					BaseObject* object = static_cast<BaseObject*>(link);
					C4DAtom* clone = object->GetClone(COPYFLAGS::NONE, nullptr);
					if (clone)
					{
						BaseDocument* document = object->GetDocument();
						if (!document)
						{
							BaseObject* cacheParent = object->GetCacheTopParent();
							if (cacheParent)
								document = cacheParent->GetDocument();
						}
						if (!document)
							document = GetActiveDocument();

						if (document)
						{
							BaseObject* cloneObject = static_cast<BaseObject*>(clone);
							// BIT_EDITOBJECT is set internally to hide the polygon object in certain cases (e.g. if the object is under a SDS). 
							// Remove the flag here, otherwise the clone would not be visible in the viewport.
							cloneObject->DelBit(BIT_EDITOBJECT);
							document->InsertObject(cloneObject, nullptr, nullptr);
							needsEventAdd = true;
						}
					}
				}
				if (needsEventAdd)
				{
					EventAdd();
				}

				break;
			}
		}

		return true;
	}

private:
	static const Int32 EXTRACT_OBJECT_TO_SCENE = ID_TREEVIEW_FIRST_NEW_ID + 1;
};

Function2 g_functable;

enum
{
	IDC_AO_LOCK_ELEMENT = 1001,
	IDC_AO_DESCRIPTION	= 10000,
	IDC_AO_TREEVIEW			= 10001
};


class ActiveObjectDialog : public GeDialog
{
private:
	Bool							 lock;
	TreeViewCustomGui* tree;
	DebugNode					 root_of_docs;
	DebugArray				 oldlist;

public:
	~ActiveObjectDialog()
	{
		oldlist.FlushAll();
	}

	DescriptionCustomGui* gad;

	virtual Bool CreateLayout();
	virtual Bool InitValues();
	virtual void DestroyWindow();
	virtual Bool Command(Int32 id, const BaseContainer& msg);
	virtual Bool CoreMessage(Int32 id, const BaseContainer& msg);
};

void ActiveObjectDialog::DestroyWindow()
{
	tree = nullptr;
	gad	 = nullptr;
}

Bool ActiveObjectDialog::CreateLayout()
{
	// first call the parent instance
	Bool res = GeDialog::CreateLayout();

	SetTitle("C++SDK Demo - Active Object Properties"_s);

	GroupBegin(0, BFH_SCALEFIT | BFV_SCALEFIT, 0, 1, String(), BFV_GRIDGROUP_ALLOW_WEIGHTS);
	// you can also write "TREEVIEW id { BORDER; } in the resource file

	GroupBegin(0, BFH_SCALEFIT | BFV_SCALEFIT, 1, 0, String(), 0);
	AddCheckbox(IDC_AO_LOCK_ELEMENT, BFH_LEFT, 0, 0, "Lock Element"_s);

	BaseContainer treedata;
	treedata.SetBool(TREEVIEW_BORDER, true);
	treedata.SetBool(TREEVIEW_HAS_HEADER, true);
	treedata.SetBool(TREEVIEW_FIXED_LAYOUT, true);
	tree = (TreeViewCustomGui*)AddCustomGui(IDC_AO_TREEVIEW, CUSTOMGUI_TREEVIEW, String(), BFH_SCALEFIT | BFV_SCALEFIT, 0, 0, treedata);
	// you can also write "TREEVIEW id { BORDER; } in the resource file
	GroupEnd();

	BaseContainer customgui;
	customgui.SetBool(DESCRIPTION_ALLOWFOLDING, true);
	gad = (DescriptionCustomGui*)AddCustomGui(IDC_AO_DESCRIPTION, CUSTOMGUI_DESCRIPTION, String(), BFH_SCALEFIT | BFV_SCALEFIT, 0, 0, customgui);

	GroupEnd();

	if (gad && GetActiveDocument())
	{
		gad->SetObject(GetActiveDocument()->GetActiveObject());
	}

	if (tree)
	{
		BaseContainer layout;
		layout.SetInt32('tree', LV_TREE);
		layout.SetInt32('icon', LV_USER);
		layout.SetInt32('type', LV_USER);
		layout.SetInt32('addr', LV_USER);
		layout.SetInt32('bits', LV_USER);
		layout.SetInt32('drt0', LV_USER);
		layout.SetInt32('drt1', LV_USER);
		layout.SetInt32('drt2', LV_USER);
		layout.SetInt32('drt3', LV_USER);
		layout.SetInt32('drt4', LV_USER);
		layout.SetInt32('drt5', LV_USER);
		layout.SetInt32('drt6', LV_USER);
		layout.SetInt32('drt7', LV_USER);
		layout.SetInt32('drt8', LV_USER);
		layout.SetInt32('drt9', LV_USER);
		layout.SetInt32('drta', LV_USER);
		layout.SetInt32('drtb', LV_USER);
		layout.SetInt32('drtL', LV_USER);
		tree->SetLayout(18, layout);
	}

	return res;
}

Bool ActiveObjectDialog::InitValues()
{
	// first call the parent instance
	if (!GeDialog::InitValues())
		return false;

	if (tree)
	{
		root_of_docs.down.FreeList(false);

		DebugArray newlist;
		BuildTree(&root_of_docs, oldlist, newlist, GetActiveDocument()->GetListHead(), String("Documents"));
		oldlist.FlushAll();
		iferr (oldlist.CopyFrom(newlist))
      return false;
		oldlist.Sort();
		tree->SetRoot(&root_of_docs, &g_functable, nullptr);
		tree->Refresh();
	}

	return true;
}

Bool ActiveObjectDialog::Command(Int32 id, const BaseContainer& msg)
{
	switch (id)
	{
		case IDC_AO_LOCK_ELEMENT:
			lock = msg.GetInt32(BFM_ACTION_VALUE) != 0;
			CoreMessage(EVMSG_CHANGE, BaseContainer());
			break;
	}
	return GeDialog::Command(id, msg);
}



Bool ActiveObjectDialog::CoreMessage(Int32 id, const BaseContainer& msg)
{
	switch (id)
	{
		case EVMSG_CHANGE:
			if (CheckCoreMessage(msg) && !lock)
			{
				if (gad && GetActiveDocument())
				{
					InitValues();
				}
			}
			break;
	}
	return GeDialog::CoreMessage(id, msg);
}



class ActiveObjectDialogCommand : public CommandData
{
public:
	ActiveObjectDialog dlg;

	virtual Bool Execute(BaseDocument* doc, GeDialog* parentManager)
	{
		return dlg.Open(DLG_TYPE::ASYNC, ID_ACTIVEOBJECT, -1, -1, 500, 300);
	}

	virtual Int32 GetState(BaseDocument* doc, GeDialog* parentManager)
	{
		return CMD_ENABLED;
	}

	virtual Bool RestoreLayout(void* secret)
	{
		return dlg.RestoreLayout(ID_ACTIVEOBJECT, 0, secret);
	}
};

ActiveObjectDialogCommand* g_cmd;

static void ShowObjectProps(BaseList2D* obj)
{
	if (!g_cmd)
		return;
	if (!g_cmd->dlg.gad)
		return;
	g_cmd->dlg.gad->SetObject(obj);
}

Bool RegisterActiveObjectDlg()
{
	g_cmd = NewObjClear(ActiveObjectDialogCommand);
	return RegisterCommandPlugin(ID_ACTIVEOBJECT, GeLoadString(IDS_ACTIVEOBJECT), 0, nullptr, String("C++ SDK Active Object"), g_cmd);
}

