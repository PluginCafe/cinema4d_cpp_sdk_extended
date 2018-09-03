#include "main.h"
#include "c4d.h"
#include "c4d_symbols.h"

//----------------------------------------------------------------------------------------
/// This example shows how to use custom datatypes in combination with custom GUIs.
///
/// iCustomDataTypeDots is the actual data object that stores the data, in this case an array of Vectors.
/// DotsCustomDataTypeClass is the plugin class that represents a new data type within Cinema 4D and handles iCustomDataTypeDots data.
/// SDKExampleCustomGUIDots is the plugin class that represents a new GUI element. It creates an iExampleCustomGUIDots dialog.
/// iExampleCustomGUIDots is a dialog that is the actual custom GUI. It includes a DotsUserArea GeUserArea.
/// DotsUserArea is a GeUserArea that displays the custom datatype and reacts to user interaction.
///
//----------------------------------------------------------------------------------------


#define ID_SDK_EXAMPLE_CUSTOMGUI_DOTS 1035302	///> The ID of the custom GUI
#define ID_SDK_EXAMPLE_CUSTOMDATATYPE_DOTS 1035303 ///> The ID of the custom datatype

class DotsCustomDataTypeClass; // forward declaration

//---------------------------
/// the actual data
//---------------------------
class iCustomDataTypeDots : public iCustomDataType<iCustomDataTypeDots>
{
	friend class DotsCustomDataTypeClass;

public:

	// an array of Vectors
	maxon::BaseArray<Vector> _points;

	iCustomDataTypeDots()
	{
		
	}
};


//---------------------------
/// The user area used to display the custom datatype
//---------------------------
class DotsUserArea : public GeUserArea
{
public:
	DotsUserArea();
	virtual ~DotsUserArea();

  virtual Bool Init();
  virtual Bool InitValues();
  virtual Bool GetMinSize(Int32& w, Int32& h);

  virtual void DrawMsg(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const BaseContainer& msg);
  virtual Bool InputEvent(const BaseContainer& msg);

	iCustomDataTypeDots* _data;

};


DotsUserArea::DotsUserArea()
{
	_data = nullptr;
}

DotsUserArea::~DotsUserArea()
{
	
}

Bool DotsUserArea::Init()
{
	return true;
}

Bool DotsUserArea::InitValues()
{
	return true;
}

Bool DotsUserArea::GetMinSize(Int32& w, Int32& h)
{
	w = 400;
	h = 300;
	return true;
}


void DotsUserArea::DrawMsg(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const BaseContainer& msg)
{
	this->OffScreenOn();

	// drawing the white background
	this->DrawSetPen(Vector(1.0));
	this->DrawRectangle(0, 0, 400, 300);

	// if the data is defined for each dot a rectangle is drawn
	if (_data)
	{
		this->DrawSetPen(Vector(0.0));

		for (Int32 i = 0; i < _data->_points.GetCount(); ++i)
		{
			const Vector vec = _data->_points[i];
			this->DrawRectangle(SAFEINT32(vec.x)-5, SAFEINT32(vec.y)-5, SAFEINT32(vec.x) + 5, SAFEINT32(vec.y) + 5);
		}
	}
}

Bool DotsUserArea::InputEvent(const BaseContainer& msg)
{
	// check the input device
	switch (msg.GetInt32(BFM_INPUT_DEVICE))
	{
		// some mouse interaction
		case	BFM_INPUT_MOUSE:																	
		{
			// get the cursor position
			Int32 mx = msg.GetInt32(BFM_INPUT_X);
			Int32 my = msg.GetInt32(BFM_INPUT_Y);
			Global2Local(&mx, &my);

			// add the new position to the data
			iferr (_data->_points.Append(Vector(mx, my, 0)))
        return false;

			// inform the parent that the data has changed
			BaseContainer m(BFM_ACTION);
			m.SetInt32(BFM_ACTION_ID, GetId());
			SendParentMessage(m);

			return true;
		}
	}
	return false;
}

//------------
/// the iCustomGui dialog hosting the user area 
//------------

#define ID_USERAREA 2000 ///< The ID of the user area GUI element.

class iExampleCustomGUIDots : public iCustomGui
{
	INSTANCEOF(iExampleCustomGUIDots, iCustomGui)

private:

	Bool _tristate;

	iCustomDataTypeDots _data;
	DotsUserArea _dotsUserArea;

public:
	
	iExampleCustomGUIDots(const BaseContainer &settings, CUSTOMGUIPLUGIN *plugin);
	virtual Bool CreateLayout();
	virtual Bool InitValues();
	virtual Bool Command(Int32 id, const BaseContainer &msg);
	virtual Bool SetData(const TriState<GeData> &tristate);
	virtual TriState<GeData> GetData();
};


iExampleCustomGUIDots::iExampleCustomGUIDots(const BaseContainer &settings, CUSTOMGUIPLUGIN *plugin) : iCustomGui(settings, plugin)
{
	_tristate = false;
};

Bool iExampleCustomGUIDots::CreateLayout()
{
	BaseContainer bc;

	GroupBegin(1000, BFH_SCALEFIT|BFV_FIT, 2, 1, String(), 0);
	{
		GroupSpace(0, 0);

		// creating the GeUserArea
		C4DGadget* userarea = this->AddUserArea(ID_USERAREA, BFH_LEFT, 400, 300);
		this->AttachUserArea(_dotsUserArea, userarea);	

		// assigning the data
		_dotsUserArea._data = &_data;
	}
	GroupEnd();

	return SUPER::CreateLayout();
};

Bool iExampleCustomGUIDots::InitValues()
{
	return SUPER::InitValues();
};

Bool iExampleCustomGUIDots::Command(Int32 id, const BaseContainer &msg)
{

	if (id == ID_USERAREA)
	{
		// a message from the user area was received
		// inform the parent that the data has changed

		BaseContainer m(msg);
		m.SetInt32(BFM_ACTION_ID, GetId());
		m.SetData(BFM_ACTION_VALUE, this->GetData().GetValue());
		SendParentMessage(m);

		// redrawing the user area
		_dotsUserArea.Redraw();

		return true;
	}

	return SUPER::Command(id, msg);
}

Bool iExampleCustomGUIDots::SetData(const TriState<GeData> &tristate)
{
	// data is set from the outside

	const iCustomDataTypeDots* const data = static_cast<const iCustomDataTypeDots*>(tristate.GetValue().GetCustomDataType(ID_SDK_EXAMPLE_CUSTOMDATATYPE_DOTS));

	if (data)
	{
		_data._points.Flush();
		iferr (_data._points.CopyFrom(data->_points))
      return false;

		_dotsUserArea.Redraw();	
	}

	return true;
};

TriState<GeData> iExampleCustomGUIDots::GetData()
{
	TriState<GeData> tri;
	tri.Add(GeData(ID_SDK_EXAMPLE_CUSTOMDATATYPE_DOTS, _data));

	return tri;
};

//---------------------
/// custom GUI data
//---------------------

static Int32 g_stringtable[] = { ID_SDK_EXAMPLE_CUSTOMDATATYPE_DOTS };

class SDKExampleCustomGUIDots : public CustomGuiData
{
public:
	virtual Int32 GetId();
  virtual CDialog* Alloc(const BaseContainer& settings);
  virtual void Free(CDialog* dlg, void* userdata);
  virtual const Char* GetResourceSym();
  virtual CustomProperty* GetProperties();
  virtual Int32 GetResourceDataType(Int32*& table);

};



Int32 SDKExampleCustomGUIDots::GetId()
{
	return ID_SDK_EXAMPLE_CUSTOMGUI_DOTS;
};


CDialog* SDKExampleCustomGUIDots::Alloc(const BaseContainer& settings)
{
	// creating the dialog that "is" the actual GUI

	iferr (iExampleCustomGUIDots* const dlg = NewObj(iExampleCustomGUIDots, settings, GetPlugin()))
		return nullptr;
  
  CDialog *cdlg = dlg->Get();

  if (!cdlg) 
		return nullptr;
    
  return cdlg;
};



void SDKExampleCustomGUIDots::Free(CDialog* dlg, void* userdata)
{
	if (!dlg || !userdata) 
		return;

  iExampleCustomGUIDots* sub = static_cast<iExampleCustomGUIDots*>(userdata);
  DeleteObj(sub);
};

const Char* SDKExampleCustomGUIDots::GetResourceSym()
{
	// the symbol used in resource files
	return "CUSTOMGUIDOTS";
};

CustomProperty* SDKExampleCustomGUIDots::GetProperties()
{
	return nullptr;
};

Int32 SDKExampleCustomGUIDots::GetResourceDataType(Int32*& table)
{
	// returns the list of datatypes this GUI can work with
	table = g_stringtable; 
	return sizeof(g_stringtable)/sizeof(Int32);
};


//---------------------------
/// The datatype class
//---------------------------

class DotsCustomDataTypeClass: public CustomDataTypeClass
{
	INSTANCEOF(DotsCustomDataTypeClass, CustomDataTypeClass)

public:
	virtual Int32 GetId()
	{
		return ID_SDK_EXAMPLE_CUSTOMDATATYPE_DOTS;
	}

	virtual CustomDataType* AllocData()
	{
		iCustomDataTypeDots* data = NewObjClear(iCustomDataTypeDots);
		return data;
	};

	virtual void FreeData(CustomDataType* data)
	{
		iCustomDataTypeDots* d =  static_cast<iCustomDataTypeDots*>(data);
		DeleteObj(d);
	}

	virtual Bool CopyData(const CustomDataType* src, CustomDataType* dst, AliasTrans* aliastrans)
	{
		// copy the data to the given target data

		const iCustomDataTypeDots* s =  static_cast<const iCustomDataTypeDots*>(src);
		iCustomDataTypeDots* d =  static_cast<iCustomDataTypeDots*>(dst);

		if (!s || !d)
			return false;

		d->_points.Flush();
		iferr (d->_points.CopyFrom(s->_points))
      return false;

		return true;
	}

	virtual Int32 Compare(const CustomDataType* d1, const CustomDataType* d2)
	{
		// compare the values of the given elements
		// this is also used to determine if a parameter has changed

		const	iCustomDataTypeDots* const s = static_cast<const	iCustomDataTypeDots*>(d1);
		const	iCustomDataTypeDots* const d = static_cast<const	iCustomDataTypeDots*>(d2);

		if (!s || !d)
			return 0;

		// this just compares the number of points
		// a better implementation would also compare the values of the points

		const maxon::Int countd1 = s->_points.GetCount();
		const maxon::Int countd2 = d->_points.GetCount();

		if (countd1 == countd2) 
			return 0;
		if (countd1 < countd2) 
			return -1;
		if (countd1 > countd2) 
			return 1;

		return 0;
	}

	virtual Bool WriteData(const CustomDataType* t_d, HyperFile* hf)
	{
		// write the data of this datatype to the given hyperfile

		const iCustomDataTypeDots* const d = static_cast<const iCustomDataTypeDots*>(t_d);

		// save number of points
		const maxon::Int length = d->_points.GetCount();
		hf->WriteInt64((Int64)length);

		// save points
		for (Int64 i = 0; i < length; ++i)
		{
			hf->WriteVector(d->_points[i]);
		}

		return true;
	}

	virtual Bool ReadData(CustomDataType* t_d, HyperFile* hf, Int32 level)
	{
		// reads the data of this datatype from the given hyperfile

		iCustomDataTypeDots* const d = static_cast<iCustomDataTypeDots*>(t_d);

		if (level > 0)
		{
			// get number of points
			Int64 length = 0;
			if (hf->ReadInt64(&length))
			{
				// read points
				for (Int64 i = 0; i < length; ++i)
				{
					Vector vec;
					if (hf->ReadVector(&vec))
					{
						iferr (d->_points.Append(vec))
              return false;
					}
				}
			}		
		}
		return true;
	}


	virtual const Char* GetResourceSym()
	{
		// this symbol can be used in resource files
		return "DOTS";
	}

	virtual void GetDefaultProperties(BaseContainer &data) 
	{
		// the default values of this datatype

		// use the custom GUI as default
		data.SetInt32(DESC_CUSTOMGUI, ID_SDK_EXAMPLE_CUSTOMGUI_DOTS);
		data.SetInt32(DESC_ANIMATE, DESC_ANIMATE_ON);
	}
};


//---------------------------
/// Register
//---------------------------
Bool RegisterCustomDatatypeCustomGUI()
{
	
	// register custom datatype
	if (!RegisterCustomDataTypePlugin(
		GeLoadString(IDS_CUSTOMDATATYPE_DOTS),
		CUSTOMDATATYPE_INFO_LOADSAVE |
		CUSTOMDATATYPE_INFO_TOGGLEDISPLAY |
		CUSTOMDATATYPE_INFO_HASSUBDESCRIPTION,
		NewObjClear(DotsCustomDataTypeClass),
		1))
		return false;

	
	// dummy library for custom GUI
	static BaseCustomGuiLib myDotCustomGUI;

	ClearMem(&myDotCustomGUI, sizeof(myDotCustomGUI));
	FillBaseCustomGui(myDotCustomGUI);

	if (!InstallLibrary(ID_SDK_EXAMPLE_CUSTOMGUI_DOTS, &myDotCustomGUI, 1000, sizeof(myDotCustomGUI))) 
		return false;

	// register custom GUI
	if (!RegisterCustomGuiPlugin(GeLoadString(IDS_CUSTOMGUI_DOTS), 0, NewObjClear(SDKExampleCustomGUIDots)))
		return false;


	return true;
}
