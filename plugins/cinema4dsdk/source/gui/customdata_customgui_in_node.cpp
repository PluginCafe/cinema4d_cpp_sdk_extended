/*
	customdata_customgui_in_node.cpp
	(C) 2021 MAXON Computer GmbH

	Date: 17/12/2021

 This example show how to support classic API UI and datatype inside the Node Editor. 
 For this, we need to register the datatype as a Maxon Data.

 The same code will be used as Classic API and Maxon APi. We need to add some functions, 
 like a copy constructor, an assignment operator and the DescribeIO function.

 To be able to create a user node with the resource editor, we need to create and register a database.
 More information can be found on our manual: 
 https://developers.maxon.net/docs/Cinema4DCPPSDK/html/page_handbook_node_implementation_register_database.html


*/


#include "maxon/apibase.h"
#include "maxon/errorbase.h"
#include "maxon/errortypes.h"
#include "maxon/base_preset_asset.h"
#include "lib_description.h"
#include "maxon/datadescription_ui.h"
#include "maxon/desctranslation.h"
#include "maxon/datadescription_data.h"
#include "maxon/nodepath.h"


#include "c4d_general.h"
#include "c4d_commanddata.h"
#include "c4d_baseplugin.h"
#include "c4d_basedocument.h"
#include "c4d_customdatatypeplugin.h"
#include "lib_description.h"


#include "customdata_customgui_in_node.h"

namespace maxon
{
	
// Implementing a UiConversionInterface for the DotData example already implemented in the sdk examples.
// This implementation allow to convert Classic API dataType to MaxonAPI data.

class UiConversionDotDataImpl : public Component<UiConversionDotDataImpl, UiConversionInterface>
{
	MAXON_COMPONENT();

public:
	
	MAXON_METHOD Result<void> QuerySupportedDataTypes(BaseArray<DataType>& dataTypes) const
	{
		iferr_scope;
		// We must define here what Maxon Data are supported by this Implementation.
		dataTypes.Append(GetDataType<::iCustomDataTypeDots>()) iferr_return;
		return OK;
	}
	
	MAXON_METHOD Result<void> CreateC4DDescription(const DataType& dataType, Description& c4dDescription, const LanguageRef& language,
		const DataDictionary& dataEntry, const DataDictionary& guiEntry, const DataDescription& mainDataDescription, const DataDescription& stringDescription, const DescID& mainId,
		const DescID& groupId, const PatchC4DDescriptionEntryDelegate& patchEntryFunc, DescTranslation& translateIds,
		const BaseArray<InternedId>& parentIds, const DescID& parentFoldId, const GetDataCallbackType& getDataCallback,
		const GetExtraDataCallbackType& getExtraDataDelegate) const
	{
		iferr_scope;

		const Int dtype = id_sdk_example_customdatatype_dots;
		DescID		descId(mainId);
		descId.PushId(DescLevel(1, dtype, 0));

		BaseContainer param = GetCustomDataTypeDefault(dtype);
		patchEntryFunc(param) iferr_return;
		// Defining the default option for the classic API gadget.
		param.SetInt32(DESC_CUSTOMGUI, id_sdk_example_customgui_dots);
		param.SetBool(DESC_SCALEH, true);
		param.SetString(DESC_NAME, "Dot Custom Data"_s);
		param.SetString(DESC_SHORT_NAME, "Dot Custom Data"_s);

		// Retrieve the default value defined inside the Resource Editor.
		iCustomDataTypeDots defaultData = dataEntry.Get(DESCRIPTION::DATA::BASE::DEFAULTVALUE.Get(), iCustomDataTypeDots());
		GeData value = GeData(id_sdk_example_customdatatype_dots, reinterpret_cast<const CustomDataType&>(defaultData));
		
		// Sometimes, we don't need to define the default value.
		param.SetData(DESC_DEFAULT, value);
		c4dDescription.SetParameter(descId, param, groupId);

		InternedId eid = guiEntry.Get(DESCRIPTION::BASE::IDENTIFIER) iferr_return;
		Bool			 isVariadic = guiEntry.Get(DESCRIPTION::DATA::BASE::ISVARIADIC, false); // #variadicPortIndicator

		BaseArray<InternedId> dataArray;
		dataArray.CopyFrom(parentIds) iferr_return;
		// If the port is variadic (multiple number of port can be created), we need extra code.
		if (!isVariadic)
		{
			NodePath::ParsePath(eid.ToBlock(), dataArray) iferr_return;
		}
		Id guiTypeId = guiEntry.Get(DESCRIPTION::UI::BASE::GUITYPEID, Id());
		Id dataTypeId = dataEntry.Get(DESCRIPTION::DATA::BASE::DATATYPE, Id());

		// We add the information in the translateIds parameter so the system can have the necessary information.
		translateIds._descIdMap.Insert(descId, { std::move(dataArray), dataTypeId, guiTypeId, Id(), dataEntry, guiEntry }) iferr_return;

		return OK;
	}

	MAXON_METHOD Result<void> ConvertToC4D(GeData& output, const DataType& dataType, const Data& data, const DescID& descIdSuffix,
		const DataDictionary& dataEntry, const DataDictionary& guiEntry, const GetExtraDataCallbackType& extraDataDelegate) const
	{
		iferr_scope;
		// Retrieve the data from the Maxon API
		iCustomDataTypeDots value = data.Get<iCustomDataTypeDots>() iferr_return;
		// Cast it so we can create a GeData from it. As the data is stored exactly the same way, we don't need more.
		output = GeData(id_sdk_example_customdatatype_dots, reinterpret_cast<const CustomDataType&>(value));

		return OK;
	}

	MAXON_METHOD Result<Tuple<Data, Bool>> ConvertToCore(const DataType& dataType, const GeData& data, const DescID& descIdSuffix,
		const DataDictionary& dataEntry, const DataDictionary& guiEntry, const Data& oldData,
		const GetExtraDataCallbackType& extraDataDelegate) const
	{
		iferr_scope;
		// Retrieve the data from the GeData. It is important to dereference the pointer.
		iCustomDataTypeDots dotData = *(static_cast<iCustomDataTypeDots*>(data.GetCustomDataType(id_sdk_example_customdatatype_dots)));
		
		// Create the Data from iCustomDataTypeDots 	
		Data convertedData;
		convertedData.Set(dotData) iferr_return;

		return Tuple<Data, Bool>(std::move(convertedData), false);
	}
};

MAXON_COMPONENT_OBJECT_REGISTER(UiConversionDotDataImpl, UiConversions::UiDotData);

static maxon::Id g_nodeCustomGUIDatabaseID = maxon::Id("net.maxonexample.databases.customgui");


// To be able to create a user node, we need to create a database and store the usernode inside it. The database need to be register in c++
// but the user node will be created inside the Ressource Editor.

static maxon::Result<void> LoadResources()
{
	iferr_scope_handler
	{
		err.CritStop();
		return err;
	};
	// Get plugin location
	const maxon::Url& binaryUrl = maxon::g_maxon.GetUrl();
	// Get plugin folder
	maxon::Url pluginDir = binaryUrl.GetDirectory();
	// Get resource folder (this folder must exist)
	const maxon::Url resourceUrl = pluginDir.Append("res"_s).Append("nodes"_s) iferr_return;
	// Register database
	maxon::DataDescriptionDefinitionDatabaseInterface::RegisterDatabaseWithUrl(g_nodeCustomGUIDatabaseID, resourceUrl) iferr_return;

	// We can define a default conversion class for a DataType.
	maxon::UiConversionInterface::AddDefaultConversion(maxon::GetDataType<iCustomDataTypeDots>(), UiConversionDotDataImpl::GetClass().GetId()) iferr_return;
	return maxon::OK;
}

static void FreeResources()
{
	iferr_scope_handler
	{
		err.CritStop();
		return;
	};
	// Unregister a database
	maxon::DataDescriptionDefinitionDatabaseInterface::UnregisterDatabase(g_nodeCustomGUIDatabaseID) iferr_return;
}
MAXON_INITIALIZATION(LoadResources, FreeResources);



} // namespace maxon



