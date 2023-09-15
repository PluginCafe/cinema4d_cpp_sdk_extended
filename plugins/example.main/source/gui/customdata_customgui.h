/*
	Dots CustomDataType & CustomGui Example
	(C) 2021 MAXON Computer GmbH

	Author: Sebastian Bach
	Date: 13/12/2021

	Showcases how to use custom data types in combination with custom GUIs.

  The type iCustomDataTypeDots is the actual data object that stores the custom data, where
	DotsCustomDataTypeClass is the plugin class that represents the new data type within Cinema 4D, 
	handling the underlying iCustomDataTypeDots data.

  SDKExampleCustomGUIDots is the plugin class that represents a new GUI element for that data
  type. It creates an iExampleCustomGUIDots dialog that realizes the GUI for that data type. The
  dialog includes the GeUserArea implementation DotsUserArea which handles the rendering of the
  custom data in a GUI.

  This example have been modified to be used a node inside the node editor.
*/
#ifndef CUSTOMDATA_CUSTOMGUI_H__
#define CUSTOMDATA_CUSTOMGUI_H__

#include "c4d_baselist.h"
#include "c4d_customdatatype.h"
#include "c4d_gui.h"

const maxon::Int32 id_sdk_example_customgui_dots = 1035302;	///< The ID of the custom GUI
const maxon::Int32 id_sdk_example_customdatatype_dots = 1035303; ///< The ID of the custom datatype

class DotsCustomDataTypeClass; // forward declaration

//---------------------------
/// the actual data
//---------------------------
class iCustomDataTypeDots : public iCustomDataType<iCustomDataTypeDots, id_sdk_example_customdatatype_dots>
{
	friend class DotsCustomDataTypeClass;

public:
	iCustomDataTypeDots() = default;
	explicit iCustomDataTypeDots(const maxon::BaseArray<Vector> &points);

	//---------------------------
	// The example "how to use a Classic API UI and Datatype inside the node editor" 
	// need some extra code.
	// In order to use the Classic API data type and its UI in the node editor, 
	// a copy constructor is mandatory, an assignment operator, and 
	// the DescribeIO function or the data will not be properly saved to the cinema4D's project file.
	
	iCustomDataTypeDots(const iCustomDataTypeDots& cdtd);
	
	void operator =(const iCustomDataTypeDots& b)
	{
		_points.Flush();
		_points.CopyFrom(b._points) iferr_ignore("Simply ignore copy from error"_s);
	}
		
	MAXON_OPERATOR_EQUALITY(iCustomDataTypeDots, _points);

	/// DescribeIO is mandatory otherwise, maxon Data cannot be saved.
	static maxon::Result<void> DescribeIO(const maxon::DataSerializeInterface& stream);
	//---------------------------
	
	// The accessors that will be used in several functions.
	maxon::BaseArray<Vector> GetValue() const;
	void SetValue(const maxon::BaseArray<Vector> &value);


	// an array of Vectors
	maxon::BaseArray<Vector> _points;

};
// For the Node API example, customDataTypeDot need to be registered as a Maxon Data.
MAXON_DATATYPE(iCustomDataTypeDots, "net.maxonsdk.datatype.iCustomDataTypeDots");


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

//------------
/// the iCustomGui dialog hosting the user area 
//------------
enum
{
	ID_USERAREA = 2000, ///< The ID of the user area GUI element.
};

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

//---------------------
/// custom GUI data
//---------------------
static Int32 g_stringtable[] = { id_sdk_example_customdatatype_dots };

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

//---------------------------
/// The datatype class
//---------------------------
class DotsCustomDataTypeClass : public CustomDataTypeClass
{
  INSTANCEOF(DotsCustomDataTypeClass, CustomDataTypeClass)

public:
  virtual Int32 GetId();
  virtual CustomDataType* AllocData();
  virtual void FreeData(CustomDataType* data);
  virtual Bool CopyData(const CustomDataType* src, CustomDataType* dst, AliasTrans* aliastrans);
  virtual Int32 Compare(const CustomDataType* d1, const CustomDataType* d2);
  virtual Bool WriteData(const CustomDataType* t_d, HyperFile* hf);
  virtual Bool ReadData(CustomDataType* t_d, HyperFile* hf, Int32 level);
  virtual const Char* GetResourceSym();
  virtual void GetDefaultProperties(BaseContainer& data);
};

//---------------------------
/// Register
//---------------------------
Bool RegisterCustomDatatypeCustomGUI();

#include "customdata_customgui1.hxx"
#include "customdata_customgui2.hxx"

#endif // CUSTOMDATA_CUSTOMGUI_H__
