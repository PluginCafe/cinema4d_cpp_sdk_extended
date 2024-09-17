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

struct HybridDataTypeLib : public cinema::BaseCustomGuiLib
{

};

class HybridDataTypeData : public cinema::CustomGuiData
{
public:
	virtual cinema::Int32 GetId() override;
	virtual cinema::CDialog* Alloc(const cinema::BaseContainer &settings) override;
	virtual void Free(cinema::CDialog *dlg, void *userdata) override;
	virtual const cinema::Char* GetResourceSym() override;
	virtual cinema::CustomProperty* GetProperties() override;
	virtual cinema::Int32 GetResourceDataType(cinema::Int32 *&table) override;

	static maxon::Result<void> RegisterGuiPlugin();
};

class HybridDataTypeGui : public cinema::iBaseCustomGui
{
	INSTANCEOF(HybridDataTypeGui, cinema::iCustomGui)

public:
	HybridDataTypeGui(const cinema::BaseContainer &settings, cinema::CUSTOMGUIPLUGIN *plugin);

	virtual cinema::Bool CreateLayout() override;
	virtual cinema::Bool InitValues() override;
	virtual cinema::Bool Command(cinema::Int32 id, const cinema::BaseContainer &msg) override;

	virtual cinema::TriState<cinema::GeData> GetData() override;
	virtual cinema::Bool SetData(const cinema::TriState<cinema::GeData> &tristate) override;
private:
	HybridDataType _value;
	HybridDataType _defaultValue;
};

// This component maps between the Cinema and Maxon API and allows to represent our own data type in the Attribute Manager GUI for nodes.
class HybridDataTypeGuiConversion : public maxon::Component<HybridDataTypeGuiConversion, maxon::UiConversionInterface>
{
	MAXON_COMPONENT();

public:

	static maxon::Result<void> Initialize();

	MAXON_METHOD maxon::Result<void> QuerySupportedDataTypes(maxon::BaseArray<maxon::DataType>& dataTypes) const;

	MAXON_METHOD maxon::Result<void> CreateC4DDescription(const maxon::DataType& dataType, cinema::Description& c4dDescription, const maxon::LanguageRef& language,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::DataDescription& mainDataDescription, const maxon::DataDescription& stringDescription, const cinema::DescID& mainId,
		const cinema::DescID& groupId, const maxon::PatchC4DDescriptionEntryDelegate& patchEntryFunc, maxon::DescTranslation& translateIds,
		const maxon::BaseArray<maxon::InternedId>& parentIds, const cinema::DescID& parentFoldId, const maxon::GetDataCallbackType& getDataCallback,
		const maxon::GetExtraDataCallbackType& getExtraDataDelegate, const cinema::BaseDocument* doc) const;

	MAXON_METHOD maxon::Result<void> ConvertToC4D(cinema::GeData& output, const maxon::DataType& dataType, const maxon::Data& data, const cinema::DescID& descIdSuffix,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::GetExtraDataCallbackType& extraDataDelegate, const cinema::BaseDocument* doc) const;

	MAXON_METHOD maxon::Result<maxon::Tuple<maxon::Data, maxon::Bool>> ConvertToCore(const maxon::DataType& dataType, const cinema::GeData& data, const cinema::DescID& descIdSuffix,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::Data& oldData,
		const maxon::GetExtraDataCallbackType& extraDataDelegate, const cinema::BaseDocument* doc) const;
};

} // namespace maxonsdk
#endif // CUSTOMGUI_HYBRIDDATATYPE_H__
