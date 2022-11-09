#ifndef CUSTOMGUI_HYBRIDDATATYPE_H__
#define CUSTOMGUI_HYBRIDDATATYPE_H__

#include "maxon/apibase.h"
#include "maxon/interfacebase.h"
#include "maxon/lib_math.h"
#include "maxon/datatype.h"
#include "maxon/uiconversions.h"

#include "c4d_gui.h"
#include "c4d_customguidata.h"
#include "c4d_baselist.h"
#include "customgui_base.h"
#include "hybriddatatype.h"

#define HYBRIDDATATYPEGUI_ID 1053473

namespace maxonsdk
{

struct HybridDataTypeLib : public BaseCustomGuiLib
{

};

class HybridDataTypeData : public CustomGuiData
{
public:
	virtual Int32 GetId() override;
	virtual CDialog* Alloc(const BaseContainer &settings) override;
	virtual void Free(CDialog *dlg, void *userdata) override;
	virtual const Char* GetResourceSym() override;
	virtual CustomProperty* GetProperties() override;
	virtual Int32 GetResourceDataType(Int32 *&table) override;

	static maxon::Result<void> RegisterGuiPlugin();
};

class HybridDataTypeGui : public iBaseCustomGui
{
	INSTANCEOF(HybridDataTypeGui, iCustomGui)

public:
	HybridDataTypeGui(const BaseContainer &settings, CUSTOMGUIPLUGIN *plugin);

	virtual Bool CreateLayout() override;
	virtual Bool InitValues() override;
	virtual Bool Command(Int32 id, const BaseContainer &msg) override;

	virtual TriState<GeData> GetData() override;
	virtual Bool SetData(const TriState<GeData> &tristate) override;
private:
	HybridDataType _value;
	HybridDataType _defaultValue;
};

// This component maps between the 'classical' and 'new' API and allows to represent our own data type in the Attribute Manager GUI for nodes.
class HybridDataTypeGuiConversion : public maxon::Component<HybridDataTypeGuiConversion, maxon::UiConversionInterface>
{
	MAXON_COMPONENT();

public:

	static maxon::Result<void> Initialize();

	MAXON_METHOD maxon::Result<void> QuerySupportedDataTypes(maxon::BaseArray<maxon::DataType>& dataTypes) const;

	MAXON_METHOD maxon::Result<void> CreateC4DDescription(const maxon::DataType& dataType, Description& c4dDescription, const maxon::LanguageRef& language,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::DataDescription& mainDataDescription, const maxon::DataDescription& stringDescription, const DescID& mainId,
		const DescID& groupId, const maxon::PatchC4DDescriptionEntryDelegate& patchEntryFunc, maxon::DescTranslation& translateIds,
		const maxon::BaseArray<maxon::InternedId>& parentIds, const DescID& parentFoldId, const maxon::GetDataCallbackType& getDataCallback,
		const maxon::GetExtraDataCallbackType& getExtraDataDelegate, BaseDocument* doc) const;

	MAXON_METHOD maxon::Result<void> ConvertToC4D(GeData& output, const maxon::DataType& dataType, const maxon::Data& data, const DescID& descIdSuffix,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::GetExtraDataCallbackType& extraDataDelegate, BaseDocument* doc) const;

	MAXON_METHOD maxon::Result<maxon::Tuple<maxon::Data, maxon::Bool>> ConvertToCore(const maxon::DataType& dataType, const GeData& data, const DescID& descIdSuffix,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::Data& oldData,
		const maxon::GetExtraDataCallbackType& extraDataDelegate, BaseDocument* doc) const;
};

} // namespace maxonsdk
#endif // CUSTOMGUI_HYBRIDDATATYPE_H__
