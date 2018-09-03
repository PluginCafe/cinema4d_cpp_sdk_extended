#include "c4d.h"
#include "main.h"
#include "odescription.h"
#include "c4d_symbols.h"

//----------------------------------------------------------------------------------------
/// This ObjectData example shows how to create parameter descriptions dynamically and how to access parameters.
//----------------------------------------------------------------------------------------

#include "customgui_filename.h"
#include "customgui_texbox.h"
#include "customgui_inexclude.h"
#include "customgui_priority.h"
#include "customgui_datetime.h"

#define ID_QUICKTABSRADIO_GADGET 200000281  // this define is currently missing in the SDK

enum DESCRIPTIONELEMENTS
{
	BUTTON = 0,
	LONG,
	LONG_CYCLE,
	LONG_QUICKTAB,
	LONG_SLIDER,
	REAL,
	REAL_SLIDER,
	VECTOR,
	MATRIX,
	STRING,
	STRING_MULTILINE,
	STATIC_TEXT,
	FILENAME,
	BOOL,
	LINK,
	SHADERLINK,
	TIME,
	COLOR_PARAMETER,

	GRADIENT = 50,
	SPLINE,
	BITMAPBUTTON,
	INCLUDE_EXCLUDE,
	PRIORITY,
	COLORPROFILE,
	DATETIME,

	GROUP = 100,
	SEPERATOR = 150,

	DUMMY
};


class ObjectDynamicDescription: public ObjectData
{
	INSTANCEOF(ObjectDynamicDescription, ObjectData);

public:

	virtual Bool Init(GeListNode *node);
	virtual Bool GetDEnabling(GeListNode* node, const DescID& id, const GeData& t_data, DESCFLAGS_ENABLE flags, const BaseContainer* itemdesc);
	virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags);
	virtual Bool 	Message (GeListNode *node, Int32 type, void *data);

	static NodeData* Alloc() { return NewObjClear(ObjectDynamicDescription); }

private:

	void FillSelection(Description* const description);
	void CreateDynamicDescriptions(Description* const description);
	void CreateDynamicElement(Description* const description, const Int32 selection);

	void SetParameter(GeListNode* node);
	void GetParameter(GeListNode* node);
};

Bool ObjectDynamicDescription::Init(GeListNode *node)
{
	BaseContainer* bc = ((BaseList2D*)node)->GetDataInstance();

	// set the default value for these custom or complex datatypes

	bc->SetData(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LINK, GeData(DTYPE_BASELISTLINK, DEFAULTVALUE));
	bc->SetData(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SPLINE, GeData(CUSTOMDATATYPE_SPLINE, DEFAULTVALUE));
	bc->SetData(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::GRADIENT, GeData(CUSTOMDATATYPE_GRADIENT, DEFAULTVALUE));
	bc->SetData(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE, GeData(CUSTOMDATATYPE_INEXCLUDE_LIST, DEFAULTVALUE));
	bc->SetData(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::PRIORITY, GeData(CUSTOMGUI_PRIORITY_DATA, DEFAULTVALUE));
	bc->SetData(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::COLORPROFILE, GeData(CUSTOMDATATYPE_COLORPROFILE, DEFAULTVALUE));

	return true;
}


Bool ObjectDynamicDescription::GetDEnabling(GeListNode* node, const DescID& id, const GeData& t_data, DESCFLAGS_ENABLE flags, const BaseContainer* itemdesc)
{
	if (id[0].id == ID_SET_PARAMETER || id[0].id == ID_GET_PARAMETER)
	{
		BaseContainer* data = ((BaseObject*)node)->GetDataInstance();
		const Int32 selection = data->GetInt32(ID_SELECT_DESCRIPTION);

		switch (selection)
		{
			case DESCRIPTIONELEMENTS::BUTTON:
			case DESCRIPTIONELEMENTS::STATIC_TEXT:
			case DESCRIPTIONELEMENTS::BITMAPBUTTON:
			case DESCRIPTIONELEMENTS::GROUP:
			case DESCRIPTIONELEMENTS::SEPERATOR:
			{
				return false;
			}
		}
	}
	return true;
}

Bool ObjectDynamicDescription::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
	if (!description->LoadDescription(node->GetType()))
		return false;

	// fills the combo box defined in the resource file
	this->FillSelection(description);

	// create dynamic element
	this->CreateDynamicDescriptions(description);

	flags |= DESCFLAGS_DESC::LOADED;

	return SUPER::GetDDescription(node, description, flags);
}

Bool ObjectDynamicDescription::Message(GeListNode *node, Int32 type, void *data)
{
	switch (type)
	{
		case MSG_DESCRIPTION_COMMAND:
		{
			DescriptionCommand* dc = (DescriptionCommand*) data;

			const Int32 id = dc->_descId[0].id;

			switch (id)
			{
				case ID_SET_PARAMETER:
				{
					this->SetParameter(node);
					return true;
					break;
				}
				case ID_GET_PARAMETER:
				{
					this->GetParameter(node);
					return true;
					break;
				}
			}
			break;
		}
	}

	return SUPER::Message(node, type, data);
}

void ObjectDynamicDescription::FillSelection(Description* const description)
{
	const DescID* singleid = description->GetSingleDescID();
	const DescID cid = DescLevel(ID_SELECT_DESCRIPTION, DTYPE_LONG, 0);

	if (!singleid || cid.IsPartOf(*singleid, nullptr))
	{
		AutoAlloc<AtomArray> arr;
		BaseContainer* selectionParameter = description->GetParameterI(DescLevel(ID_SELECT_DESCRIPTION, DTYPE_LONG, 0), arr);

		if (selectionParameter != nullptr)
		{
			// fill the "cycle" of the combo box
			BaseContainer* items = selectionParameter->GetContainerInstance(DESC_CYCLE);

			if (items != nullptr)
			{
				items->SetString(DESCRIPTIONELEMENTS::BUTTON, String("Button"));
				items->SetString(DESCRIPTIONELEMENTS::LONG, String("Long"));
				items->SetString(DESCRIPTIONELEMENTS::LONG_CYCLE, String("Long Cycle"));
				items->SetString(DESCRIPTIONELEMENTS::LONG_QUICKTAB, String("Long Quicktab"));
				items->SetString(DESCRIPTIONELEMENTS::LONG_SLIDER, String("Long Slider"));
				items->SetString(DESCRIPTIONELEMENTS::REAL, String("Real"));
				items->SetString(DESCRIPTIONELEMENTS::REAL_SLIDER, String("Real Slider"));
				items->SetString(DESCRIPTIONELEMENTS::VECTOR, String("Vector"));
				items->SetString(DESCRIPTIONELEMENTS::MATRIX, String("Matrix"));
				items->SetString(DESCRIPTIONELEMENTS::STRING, String("String"));
				items->SetString(DESCRIPTIONELEMENTS::STRING_MULTILINE, String("String Multiline"));
				items->SetString(DESCRIPTIONELEMENTS::STATIC_TEXT, String("Static Text"));
				items->SetString(DESCRIPTIONELEMENTS::FILENAME, String("Filename"));
				items->SetString(DESCRIPTIONELEMENTS::BOOL, String("Bool"));
				items->SetString(DESCRIPTIONELEMENTS::LINK, String("Link"));
				items->SetString(DESCRIPTIONELEMENTS::SHADERLINK, String("Shaderlink"));
				items->SetString(DESCRIPTIONELEMENTS::TIME, String("Time"));
				items->SetString(DESCRIPTIONELEMENTS::COLOR_PARAMETER, String("Color"));

				items->SetString(-1, String(""));

				items->SetString(DESCRIPTIONELEMENTS::GRADIENT, String("Gradient"));
				items->SetString(DESCRIPTIONELEMENTS::SPLINE, String("Spline"));
				items->SetString(DESCRIPTIONELEMENTS::BITMAPBUTTON, String("Bitmap Button"));
				items->SetString(DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE, String("Include Exclude"));
				items->SetString(DESCRIPTIONELEMENTS::PRIORITY, String("Priority"));
				items->SetString(DESCRIPTIONELEMENTS::COLORPROFILE, String("Colorprofile"));
				items->SetString(DESCRIPTIONELEMENTS::DATETIME, String("DateTime"));

				items->SetString(-2, String(""));

				items->SetString(DESCRIPTIONELEMENTS::GROUP, String("Group"));
				items->SetString(DESCRIPTIONELEMENTS::SEPERATOR, String("Seperator"));
			}
		}
	}
}


void ObjectDynamicDescription::CreateDynamicDescriptions(Description* const description)
{
	// get current selection

	GeData data;
	this->Get()->GetParameter(DescID(ID_SELECT_DESCRIPTION), data, DESCFLAGS_GET::NONE);
	const Int32 selection = data.GetInt32();

	// create dynamic element
	this->CreateDynamicElement(description, selection);
}


MAXON_WARN_MUTE_FUNCTION_LENGTH void ObjectDynamicDescription::CreateDynamicElement(Description* const description, const Int32 selection)
{
	const DescID* singleid = description->GetSingleDescID();

	switch (selection)
	{
		// DTYPE_BUTTON / CUSTOMGUI_BUTTON
		case DESCRIPTIONELEMENTS::BUTTON:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::BUTTON;
			const DescID cid = DescLevel(ID, DTYPE_BUTTON, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_BUTTON);

				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_BUTTON);
				bc.SetString(DESC_NAME, "Button"_s);
				bc.SetInt32(DESC_ANIMATE, DESC_ANIMATE_OFF);
				bc.SetInt32(DESC_SCALEH, 1);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_LONG
		case DESCRIPTIONELEMENTS::LONG:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LONG;
			const DescID cid = DescLevel(ID, DTYPE_LONG, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_LONG);

				bc.SetString(DESC_NAME, "Long"_s);
				bc.SetInt32(DESC_SCALEH, 1);
				bc.SetInt32(DESC_MIN, 0);
				bc.SetInt32(DESC_MAX, 1000);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_LONG / CUSTOMGUI_CYCLE
		case DESCRIPTIONELEMENTS::LONG_CYCLE:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LONG_CYCLE;
			const DescID cid = DescLevel(ID, DTYPE_LONG, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_LONG);

				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_CYCLE);
				bc.SetString(DESC_NAME, "Long Cycle"_s);
				bc.SetInt32(DESC_SCALEH, 1);

				// cycle elements
				BaseContainer items;
				items.SetString(0, String("Item 0"));
				items.SetString(1, String("Item 1"));
				items.SetString(2, String("Item 2"));

				bc.SetContainer(DESC_CYCLE, items);

				// icons
				BaseContainer icons;
				icons.SetInt32(0, IDM_COPY);
				icons.SetInt32(1, IDM_CUT);
				icons.SetInt32(2, IDM_DELETE);

				bc.SetContainer(DESC_CYCLEICONS, icons);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_LONG / QUICKTABSRADIO_GADGET
		case DESCRIPTIONELEMENTS::LONG_QUICKTAB:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LONG_QUICKTAB;
			const DescID cid = DescLevel(ID, DTYPE_LONG, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_LONG);

				bc.SetInt32(DESC_CUSTOMGUI, ID_QUICKTABSRADIO_GADGET);
				bc.SetString(DESC_NAME, "Long Quicktab"_s);
				bc.SetInt32(DESC_SCALEH, 1);

				// quicktab elements
				BaseContainer items;
				items.SetString(0, String("Tab 0"));
				items.SetString(1, String("Tab 1"));
				items.SetString(2, String("Tab 2"));

				bc.SetContainer(DESC_CYCLE, items);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_LONG / CUSTOMGUI_LONGSLIDER
		case DESCRIPTIONELEMENTS::LONG_SLIDER:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LONG_SLIDER;
			const DescID cid = DescLevel(ID, DTYPE_LONG, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_LONG);

				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_LONGSLIDER);
				bc.SetString(DESC_NAME, "Long Slider"_s);
				bc.SetInt32(DESC_SCALEH, 1);
				bc.SetInt32(DESC_MIN, 0);
				bc.SetInt32(DESC_MAX, 1000);
				bc.SetInt32(DESC_STEP, 1);

				// slider settings
				bc.SetInt32(DESC_MINSLIDER, 250);
				bc.SetInt32(DESC_MAXSLIDER, 750);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_REAL
		case DESCRIPTIONELEMENTS::REAL:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::REAL;
			const DescID cid = DescLevel(ID, DTYPE_REAL, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_REAL);

				bc.SetString(DESC_NAME, "Real"_s);
				bc.SetInt32(DESC_SCALEH, 1);
				bc.SetInt32(DESC_MIN, 0);
				bc.SetInt32(DESC_MAX, 1000);
				bc.SetInt32(DESC_STEP, 1);
				bc.SetInt32(DESC_UNIT, DESC_UNIT_METER);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_REAL / CUSTOMGUI_REALSLIDER
		case DESCRIPTIONELEMENTS::REAL_SLIDER:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::REAL_SLIDER;
			const DescID cid = DescLevel(ID, DTYPE_REAL, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_REAL);

				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_REALSLIDER);
				bc.SetString(DESC_NAME, "Real Slider"_s);
				bc.SetInt32(DESC_SCALEH, 1);
				bc.SetInt32(DESC_MIN, 0);
				bc.SetInt32(DESC_MAX, 1000);
				bc.SetInt32(DESC_STEP, 1);
				bc.SetInt32(DESC_UNIT, DESC_UNIT_METER);

				// slider settings
				bc.SetFloat(DESC_MINSLIDER, 250);
				bc.SetFloat(DESC_MAXSLIDER, 750);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_VECTOR / CUSTOMGUI_VECTOR
		case DESCRIPTIONELEMENTS::VECTOR:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::VECTOR;
			const DescID cid = DescLevel(ID, DTYPE_VECTOR, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_VECTOR);

				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_VECTOR);
				bc.SetString(DESC_NAME, "Vector"_s);
				bc.SetInt32(DESC_SCALEH, 1);
				bc.SetInt32(DESC_UNIT, DESC_UNIT_METER);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_MATRIX
		case DESCRIPTIONELEMENTS::MATRIX:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::MATRIX;
			const DescID cid = DescLevel(ID, DTYPE_MATRIX, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_MATRIX);
				bc.SetString(DESC_NAME, "Matrix"_s);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_STRING
		case DESCRIPTIONELEMENTS::STRING:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::STRING;
			const DescID cid = DescLevel(ID, DTYPE_STRING, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_STRING);
				bc.SetString(DESC_NAME, "String"_s);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_STRING / CUSTOMGUI_STRINGMULTI
		case DESCRIPTIONELEMENTS::STRING_MULTILINE:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::STRING_MULTILINE;
			const DescID cid = DescLevel(ID, DTYPE_STRING, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_STRING);

				bc.SetString(DESC_NAME, "Multiline"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_STRINGMULTI);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_STATICTEXT / CUSTOMGUI_STATICTEXT
		case DESCRIPTIONELEMENTS::STATIC_TEXT:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::STATIC_TEXT;
			const DescID cid = DescLevel(ID, DTYPE_STATICTEXT, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_STATICTEXT);

				bc.SetString(DESC_NAME, "Static Text"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_STATICTEXT);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_FILENAME / CUSTOMGUI_FILENAME
		case DESCRIPTIONELEMENTS::FILENAME:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::FILENAME;
			const DescID cid = DescLevel(ID, DTYPE_FILENAME, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_FILENAME);

				bc.SetString(DESC_NAME, "Filename"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_FILENAME);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_BOOL
		case DESCRIPTIONELEMENTS::BOOL:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::BOOL;
			const DescID cid = DescLevel(ID, DTYPE_BOOL, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_BOOL);

				bc.SetString(DESC_NAME, "Boole"_s);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_BASELISTLINK / CUSTOMGUI_LINKBOX
		case DESCRIPTIONELEMENTS::LINK:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LINK;
			const DescID cid = DescLevel(ID, DTYPE_BASELISTLINK, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_BASELISTLINK);

				bc.SetString(DESC_NAME, "Link"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_LINKBOX);

				BaseContainer ac;
				ac.SetInt32(Obase, 1);
				bc.SetContainer(DESC_ACCEPT, ac);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_BASELISTLINK / CUSTOMGUI_TEXBOX
		case DESCRIPTIONELEMENTS::SHADERLINK:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SHADERLINK;
			const DescID cid = DescLevel(ID, DTYPE_BASELISTLINK, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_BASELISTLINK);

				bc.SetString(DESC_NAME, "Shader"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_TEXBOX);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_TIME
		case DESCRIPTIONELEMENTS::TIME:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::TIME;
			const DescID cid = DescLevel(ID, DTYPE_TIME, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_TIME);

				bc.SetString(DESC_NAME, "Time"_s);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_COLOR
		case DESCRIPTIONELEMENTS::COLOR_PARAMETER:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::COLOR_PARAMETER;
			const DescID cid = DescLevel(ID, DTYPE_COLOR, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_COLOR);

				bc.SetString(DESC_NAME, "Color"_s);
				bc.SetData(DESC_GUIOPEN, true);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// CUSTOMDATATYPE_GRADIENT / CUSTOMGUI_GRADIENT
		case DESCRIPTIONELEMENTS::GRADIENT:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::GRADIENT;
			const DescID cid = DescLevel(ID, CUSTOMDATATYPE_GRADIENT, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_GRADIENT);

				bc.SetString(DESC_NAME, "Gradient"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_GRADIENT);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// CUSTOMDATATYPE_SPLINE / CUSTOMGUI_SPLINE
		case DESCRIPTIONELEMENTS::SPLINE:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SPLINE;
			const DescID cid = DescLevel(ID, CUSTOMDATATYPE_SPLINE, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_SPLINE);

				bc.SetString(DESC_NAME, "Spline"_s);
				bc.SetBool(SPLINECONTROL_GRID_H, false);
				bc.SetBool(SPLINECONTROL_GRID_V, true);
				bc.SetBool(SPLINECONTROL_VALUE_EDIT_H, true);
				bc.SetBool(SPLINECONTROL_VALUE_EDIT_V, true);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_SPLINE);
				bc.SetFloat(SPLINECONTROL_X_MIN, 0);
				bc.SetFloat(SPLINECONTROL_X_MAX, 100);
				bc.SetFloat(SPLINECONTROL_Y_MIN, 0);
				bc.SetFloat(SPLINECONTROL_Y_MAX, 100);


				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// CUSTOMDATATYPE_BITMAPBUTTON / CUSTOMGUI_BITMAPBUTTON
		case DESCRIPTIONELEMENTS::BITMAPBUTTON:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::BITMAPBUTTON;
			const DescID cid = DescLevel(ID, CUSTOMDATATYPE_BITMAPBUTTON, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_BITMAPBUTTON);

				bc.SetString(DESC_NAME, "Button"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_BITMAPBUTTON);
				bc.SetBool(BITMAPBUTTON_BUTTON, true);
				bc.SetInt32(BITMAPBUTTON_BORDER, BORDER_BLACK);
				bc.SetInt32(BITMAPBUTTON_ICONID1, RESOURCEIMAGE_BROWSER_PLAY);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// CUSTOMDATATYPE_INEXCLUDE_LIST / CUSTOMGUI_INEXCLUDE_LIST
		case DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE;
			const DescID cid = DescLevel(ID, CUSTOMDATATYPE_INEXCLUDE_LIST, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_INEXCLUDE_LIST);

				bc.SetString(DESC_NAME, "In/Exclude"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_INEXCLUDE_LIST);

				BaseContainer accept;
				accept.InsData(Obase, String());

				bc.SetData(DESC_ACCEPT, accept);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// CUSTOMGUI_PRIORITY_DATA / CUSTOMGUI_PRIORITY
		case DESCRIPTIONELEMENTS::PRIORITY:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::PRIORITY;
			const DescID cid = DescLevel(ID, CUSTOMGUI_PRIORITY_DATA, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(CUSTOMGUI_PRIORITY_DATA);

				bc.SetString(DESC_NAME, "Priority"_s);
				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_PRIORITY);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// CUSTOMDATATYPE_COLORPROFILE
		case DESCRIPTIONELEMENTS::COLORPROFILE:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::COLORPROFILE;
			const DescID cid = DescLevel(ID, CUSTOMDATATYPE_COLORPROFILE, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(CUSTOMDATATYPE_COLORPROFILE);

				bc.SetString(DESC_NAME, "Colorprofile"_s);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DATETIME_DATA
		case DESCRIPTIONELEMENTS::DATETIME:
		{
			const Int32 ID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::DATETIME;
			const DescID cid = DescLevel(ID, DATETIME_DATA, 0);

			if (!singleid || cid.IsPartOf(*singleid, nullptr))
			{
				BaseContainer bc = GetCustomDataTypeDefault(DATETIME_DATA);

				bc.SetString(DESC_NAME, "DateTime"_s);

				description->SetParameter(cid, bc, DescLevel(ID_GROUP_DYNAMIC));
			}
			break;
		}

		// DTYPE_SEPARATOR / CUSTOMGUI_SEPARATOR
		case DESCRIPTIONELEMENTS::SEPERATOR:
		{
			const Int32 seperatorID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SEPERATOR;

			// create two dummy string elements so that the seperator appears

			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_STRING);
				bc.SetString(DESC_NAME, "Text"_s);

				description->SetParameter(DescLevel(seperatorID, DTYPE_STRING, 0), bc, ID_GROUP_DYNAMIC);
			}

			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_SEPARATOR);

				bc.SetInt32(DESC_CUSTOMGUI, CUSTOMGUI_SEPARATOR);
				bc.SetBool(DESC_SEPARATORLINE, true);

				description->SetParameter(DescLevel(seperatorID+1, DTYPE_SEPARATOR, 0), bc, ID_GROUP_DYNAMIC);
			}

			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_STRING);
				bc.SetString(DESC_NAME, "Text"_s);

				description->SetParameter(DescLevel(seperatorID+2, DTYPE_STRING, 0), bc, ID_GROUP_DYNAMIC);
			}
			break;
		}

		// DTYPE_GROUP
		case DESCRIPTIONELEMENTS::GROUP:
		{
			const Int32 groupID = ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::GROUP;

			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_GROUP);

				bc.SetString(DESC_NAME, "Group"_s);
				bc.SetInt32(DESC_COLUMNS, 2);
				bc.SetInt32(DESC_DEFAULT, 1);
				bc.SetInt32(DESC_SCALEH, 1);

				description->SetParameter(DescLevel(groupID, DTYPE_GROUP, 0), bc, ID_GROUP_DYNAMIC);
			}

			// dummy content

			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_STRING);
				bc.SetString(DESC_NAME, "Text 1"_s);
				bc.SetInt32(DESC_SCALEH, 1);

				description->SetParameter(DescLevel(groupID+1, DTYPE_STRING, 0), bc, groupID);
			}

			{
				BaseContainer bc = GetCustomDataTypeDefault(DTYPE_STRING);
				bc.SetString(DESC_NAME, "Text 2"_s);
				bc.SetInt32(DESC_SCALEH, 1);

				description->SetParameter(DescLevel(groupID+2, DTYPE_STRING, 0), bc, groupID);
			}
			break;
		}
	}
}

void ObjectDynamicDescription::SetParameter(GeListNode* node)
{
	if (node == nullptr)
		return;

	// get the document of this node
	BaseDocument* doc = node->GetDocument();

	if (doc == nullptr)
		return;

	// get the current selection
	GeData data;
	node->GetParameter(DescID(ID_SELECT_DESCRIPTION), data, DESCFLAGS_GET::NONE);

	const Int32 selection = data.GetInt32();

	switch (selection)
	{
		// DTYPE_LONG
		case DESCRIPTIONELEMENTS::LONG:
		case DESCRIPTIONELEMENTS::LONG_SLIDER:
		{
			const Int32 value = 123;

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_LONG
		case DESCRIPTIONELEMENTS::LONG_CYCLE:
		case DESCRIPTIONELEMENTS::LONG_QUICKTAB:
		{
			const Int32 value = 1;

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_REAL
		case DESCRIPTIONELEMENTS::REAL:
		case DESCRIPTIONELEMENTS::REAL_SLIDER:
		{
			const Float32 value = 123.45f;

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_VECTOR
		case DESCRIPTIONELEMENTS::VECTOR:
		{
			Vector value = Vector(1, 1, 1);

			if (node->IsInstanceOf(Obase))
			{
				BaseObject* object = static_cast<BaseObject*>(node);

				if (object != nullptr)
					value = object->GetMg().off;
			}

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::VECTOR), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_MATRIX
		case DESCRIPTIONELEMENTS::MATRIX:
		{
			Matrix value;

			if (node->IsInstanceOf(Obase))
			{
				BaseObject* object = static_cast<BaseObject*>(node);

				if (object != nullptr)
					value = object->GetMg();
			}

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::MATRIX), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_STRING
		case DESCRIPTIONELEMENTS::STRING:
		case DESCRIPTIONELEMENTS::STRING_MULTILINE:
		{
			String value = "Some string";

			if (node->IsInstanceOf(Tbaselist2d))
			{
				BaseList2D* entity = static_cast<BaseList2D*>(node);

				if (entity != nullptr)
					value = entity->GetName();
			}
			if (selection == DESCRIPTIONELEMENTS::STRING_MULTILINE)
			{
				// Use newline (\n) to add more lines to a multi-line string element.
				// Note: In a string resource file the pipe symbol (|) is used instead.
				value += "\n2nd line: Some more string.";
			}

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_FILENAME
		case DESCRIPTIONELEMENTS::FILENAME:
		{
			Filename filename;

			if (filename.FileSelect(FILESELECTTYPE::ANYTHING, FILESELECT::LOAD, "Select a C4D file"_s))
			{
				// set parameter
				node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::FILENAME), filename, DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_BOOL
		case DESCRIPTIONELEMENTS::BOOL:
		{
			const Bool value = true;

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::BOOL), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_BASELISTLINK
		case DESCRIPTIONELEMENTS::LINK:
		{
			node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LINK), data, DESCFLAGS_GET::NONE);

			if (data.GetType() == DTYPE_BASELISTLINK)
			{
				BaseLink* link = data.GetBaseLink();

				if (link != nullptr)
				{
					link->SetLink(doc->GetActiveObject());

					// set parameter
					node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::LINK), data, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// DTYPE_BASELISTLINK
		case DESCRIPTIONELEMENTS::SHADERLINK:
		{
			AutoAlloc<BaseLink> link;

			if (link != nullptr && node->IsInstanceOf(Tbaselist2d))
			{
				BaseShader* shader = BaseShader::Alloc(Xbrick);

				if (shader != nullptr)
				{
					BaseList2D* entity = static_cast<BaseList2D*>(node);

					link->SetLink(shader);

					// the entity takes ownership
					entity->InsertShader(shader);

					// set parameter
					node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SHADERLINK), GeData(link), DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// DTYPE_TIME
		case DESCRIPTIONELEMENTS::TIME:
		{
			BaseTime value;

			// get current document time
			value = doc->GetTime();

			// set parameter
			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::TIME), value, DESCFLAGS_SET::NONE);
			break;
		}

		// DTYPE_COLOR
		case DESCRIPTIONELEMENTS::COLOR_PARAMETER:
		{
			const Vector value = Vector(0, 0, 1);

			// set parameter
			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::COLOR_PARAMETER), value, DESCFLAGS_SET::NONE);
			break;
		}

		// CUSTOMDATATYPE_GRADIENT / CUSTOMGUI_GRADIENT
		case DESCRIPTIONELEMENTS::GRADIENT:
		{
			GeData value(CUSTOMDATATYPE_GRADIENT, DEFAULTVALUE);
			Gradient* gradient = (Gradient*)value.GetCustomDataType(CUSTOMDATATYPE_GRADIENT);

			if (gradient != nullptr)
			{
				{
					GradientKnot knot;
					knot.pos = 0.1;
					knot.col = Vector(1, 0, 0);
					gradient->InsertKnot(knot);
				}

				{
					GradientKnot knot;
					knot.pos = 0.9;
					knot.col = Vector(0, 1, 0);
					gradient->InsertKnot(knot);
				}
			}

			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::GRADIENT), value, DESCFLAGS_SET::NONE);
			break;
		}

		// CUSTOMDATATYPE_SPLINE
		case DESCRIPTIONELEMENTS::SPLINE:
		{
			node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SPLINE), data, DESCFLAGS_GET::NONE);

			if (data.GetType() == CUSTOMDATATYPE_SPLINE)
			{
				SplineData* splineData = (SplineData*)data.GetCustomDataType(CUSTOMDATATYPE_SPLINE);

				if (splineData != nullptr)
				{

					splineData->SetRange(0.0, 100.0, 0.1, 0.0, 100, 0.0);

					splineData->DeleteAllPoints();
					splineData->InsertKnot(0, 0);
					splineData->InsertKnot(100, 100);

					// set parameter
					node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SPLINE), data, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// CUSTOMDATATYPE_INEXCLUDE_LIST
		case DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE:
		{
			node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE), data, DESCFLAGS_GET::NONE);

			if (data.GetType() == CUSTOMDATATYPE_INEXCLUDE_LIST)
			{
				InExcludeData* ieData = (InExcludeData*)data.GetCustomDataType(CUSTOMDATATYPE_INEXCLUDE_LIST);

				if (ieData != nullptr)
				{
					// add the active object to the list
					ieData->InsertObject(doc->GetActiveObject(), 0);

					// set parameter
					node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE), data, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// CUSTOMGUI_PRIORITY_DATA / CUSTOMGUI_PRIORITY
		case DESCRIPTIONELEMENTS::PRIORITY:
		{
			GeData priorityData = GeData(CUSTOMGUI_PRIORITY_DATA, DEFAULTVALUE);
			PriorityData *value = (PriorityData*)priorityData.GetCustomDataType(CUSTOMGUI_PRIORITY_DATA);

			if (value != nullptr)
			{
				value->SetPriorityValue(PRIORITYVALUE_MODE, CYCLE_EXPRESSION);
				value->SetPriorityValue(PRIORITYVALUE_PRIORITY, 1);
			}

			// set parameter
			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::PRIORITY), priorityData, DESCFLAGS_SET::NONE);
			break;
		}

		// CUSTOMDATATYPE_COLORPROFILE
		case DESCRIPTIONELEMENTS::COLORPROFILE:
		{
			GeData colorProfileData = GeData(CUSTOMDATATYPE_COLORPROFILE, DEFAULTVALUE);

			ColorProfile* value = (ColorProfile*)colorProfileData.GetCustomDataType(CUSTOMDATATYPE_COLORPROFILE);

			if (value != nullptr)
			{
				value->SetMonitorProfileMode(true);
			}

			// set parameter
			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::COLORPROFILE), colorProfileData, DESCFLAGS_SET::NONE);
			break;
		}

		// DATETIME_DATA
		case DESCRIPTIONELEMENTS::DATETIME:
		{
			GeData dateTimeData = GeData(DATETIME_DATA, DEFAULTVALUE);

			DateTimeData* value = (DateTimeData*)dateTimeData.GetCustomDataType(DATETIME_DATA);

			if (value != nullptr)
			{
				DateTime time;
				GetDateTimeNow(time);

				value->SetDateTime(time);
			}

			// set parameter
			node->SetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::DATETIME), dateTimeData, DESCFLAGS_SET::NONE);
			break;
		}
	}
}

void ObjectDynamicDescription::GetParameter(GeListNode* node)
{
	iferr_scope_handler
	{
		return;
	};

	if (node == nullptr)
		return;

	// get the document of this node
	BaseDocument* doc = node->GetDocument();

	if (doc == nullptr)
		return;

	// get the current selection
	GeData data;
	node->GetParameter(DescID(ID_SELECT_DESCRIPTION), data, DESCFLAGS_GET::NONE);

	const Int32 selection = data.GetInt32();

	// print the value of the currently displayed dynamic parameter to the ID_RESULT_TEXT parameter
	switch (selection)
	{
		// DTYPE_LONG
		case DESCRIPTIONELEMENTS::LONG:
		case DESCRIPTIONELEMENTS::LONG_CYCLE:
		case DESCRIPTIONELEMENTS::LONG_QUICKTAB:
		case DESCRIPTIONELEMENTS::LONG_SLIDER:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_LONG)
			{
				const Int32 value = data.GetInt32();

				node->SetParameter(DescID(ID_RESULT_TEXT), String::IntToString(value), DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_REAL
		case DESCRIPTIONELEMENTS::REAL:
		case DESCRIPTIONELEMENTS::REAL_SLIDER:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_REAL)
			{
				const Float value = data.GetFloat();

				node->SetParameter(DescID(ID_RESULT_TEXT), String::FloatToString(value), DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_VECTOR / CUSTOMGUI_VECTOR
		case DESCRIPTIONELEMENTS::VECTOR:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::VECTOR), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_VECTOR)
			{
				const Vector value = data.GetVector();

				node->SetParameter(DescID(ID_RESULT_TEXT), String::VectorToString(value), DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_MATRIX
		case DESCRIPTIONELEMENTS::MATRIX:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::MATRIX), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_MATRIX)
			{
				const Matrix value = data.GetMatrix();

				String result;

				result += String::VectorToString(value.sqmat.v1) + "\n";
				result += String::VectorToString(value.sqmat.v2) + "\n";
				result += String::VectorToString(value.sqmat.v3) + "\n";
				result += String::VectorToString(value.off) + "\n";

				node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_STRING
		case DESCRIPTIONELEMENTS::STRING:
		case DESCRIPTIONELEMENTS::STRING_MULTILINE:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_STRING)
			{
				const String result = data.GetString();
				node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_FILENAME
		case DESCRIPTIONELEMENTS::FILENAME:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::FILENAME), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_FILENAME)
			{
				const Filename filename = data.GetFilename();
				const String result = filename.GetString();

				node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_BOOL
		case DESCRIPTIONELEMENTS::BOOL:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::BOOL), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_LONG)
			{
				const Bool value = data.GetBool();
				String result;

				if (value)
					result = "Checked";
				else
					result = "Not Checked";

				node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_BASELISTLINK
		case DESCRIPTIONELEMENTS::LINK:
		case DESCRIPTIONELEMENTS::SHADERLINK:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + selection), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_ALIASLINK)
			{
				BaseList2D* linkedEntity = data.GetLink(doc);

				if (linkedEntity != nullptr)
				{
					const String result = linkedEntity->GetName();

					node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// DTYPE_TIME
		case DESCRIPTIONELEMENTS::TIME:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::TIME), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_TIME)
			{
				const BaseTime time = data.GetTime();
				const Int32 frame = time.GetFrame(doc->GetFps());

				const String result = String::IntToString(frame);

				node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
			}
			break;
		}

		// DTYPE_COLOR
		case DESCRIPTIONELEMENTS::COLOR_PARAMETER:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::COLOR_PARAMETER), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DA_VECTOR)
			{
				const Vector value = data.GetVector();

				node->SetParameter(DescID(ID_RESULT_TEXT), String::VectorToString(value), DESCFLAGS_SET::NONE);
			}
			break;
		}

		// CUSTOMDATATYPE_GRADIENT / CUSTOMGUI_GRADIENT
		case DESCRIPTIONELEMENTS::GRADIENT:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::GRADIENT), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == CUSTOMDATATYPE_GRADIENT)
			{
				Gradient* gradientData = (Gradient*)data.GetCustomDataType(CUSTOMDATATYPE_GRADIENT);

				if (gradientData != nullptr)
				{
					String result;

					// init gradient
					InitRenderStruct irs(doc);
					gradientData->InitRender(irs) iferr_return;

					for (Int32 i = 0; i < 10; i++)
					{
							const Vector color = gradientData->CalcGradientPixel(Float(i) / 10.0);

							result += String::VectorToString(color) + "\n";
					}

					gradientData->FreeRender();

					node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// CUSTOMDATATYPE_SPLINE
		case DESCRIPTIONELEMENTS::SPLINE:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::SPLINE), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == CUSTOMDATATYPE_SPLINE)
			{
				SplineData* splineData = (SplineData*)data.GetCustomDataType(CUSTOMDATATYPE_SPLINE);

				if (splineData != nullptr)
				{
					String result;

					// get range
					Float minX, maxX, dummy;
					splineData->GetRange(&minX, &maxX, &dummy, &dummy, &dummy, &dummy);

					const Float stepSize = (maxX - minX) / 10.0;

					// sample the curve
					for (Float i = minX; i <= maxX; i = i + stepSize)
					{
						const Vector point = splineData->GetPoint(i);

						result += String::FloatToString(point.y) + "\n";
					}

					node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// CUSTOMDATATYPE_INEXCLUDE_LIST
		case DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::INCLUDE_EXCLUDE), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == CUSTOMDATATYPE_INEXCLUDE_LIST)
			{
				InExcludeData* inexData = (InExcludeData*)data.GetCustomDataType(CUSTOMDATATYPE_INEXCLUDE_LIST);

				if (inexData != nullptr)
				{
					String result;

					// loop through objects
					const Int32 count = inexData->GetObjectCount();

					for (Int32 i = 0; i < count; ++i)
					{
						BaseList2D* baseList = inexData->ObjectFromIndex(doc, i);

						// add object name to output
						if (baseList != nullptr)
							result += baseList->GetName()+"\n";
					}

					node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// CUSTOMGUI_PRIORITY_DATA
		case DESCRIPTIONELEMENTS::PRIORITY:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::PRIORITY), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == CUSTOMGUI_PRIORITY_DATA)
			{
				PriorityData *value = (PriorityData*)data.GetCustomDataType(CUSTOMGUI_PRIORITY_DATA);

				if (value != nullptr)
				{
					GeData priorityData = value->GetPriorityValue(PRIORITYVALUE_MODE);

					const String result = String::IntToString(priorityData.GetInt32());

					node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// CUSTOMDATATYPE_COLORPROFILE
		case DESCRIPTIONELEMENTS::COLORPROFILE:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::COLORPROFILE), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == CUSTOMDATATYPE_COLORPROFILE)
			{
				ColorProfile* value = (ColorProfile*)data.GetCustomDataType(CUSTOMDATATYPE_COLORPROFILE);

				if (value != nullptr)
				{
					const String result = value->GetInfo(ColorProfile::COLORPROFILEINFO_DESCRIPTION);
					node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}

		// DATETIME_DATA
		case DESCRIPTIONELEMENTS::DATETIME:
		{
			const Bool success = node->GetParameter(DescID(ID_DYNAMIC_ELEMENT + DESCRIPTIONELEMENTS::DATETIME), data, DESCFLAGS_GET::NONE);

			// check type
			if (success && data.GetType() == DATETIME_DATA)
			{
				DateTimeData* value = (DateTimeData*)data.GetCustomDataType(DATETIME_DATA);

				if (value != nullptr)
				{
					DateTime time = value->GetDateTime();

					String result;
					result += String::IntToString(time.year) + ":";
					result += String::IntToString(time.month) + ":";
					result += String::IntToString(time.day) + " - ";
					result += String::IntToString(time.hour) + ":";
					result += String::IntToString(time.minute) + ":";
					result += String::IntToString(time.second);

					node->SetParameter(DescID(ID_RESULT_TEXT), result, DESCFLAGS_SET::NONE);
				}
			}
			break;
		}
	}
}

#define ID_SDK_OBJECTDATA_DESCRIPTION 1035371

Bool RegisterObjectDynamicDescription()
{
	return RegisterObjectPlugin(ID_SDK_OBJECTDATA_DESCRIPTION, GeLoadString(IDS_OBJECTDATA_DESCRIPTION), 0, ObjectDynamicDescription::Alloc, "Odescription"_s, nullptr, 0);
};
