#ifndef CUSTOMGUI_STRING_H__
#define CUSTOMGUI_STRING_H__

#include "customgui_helpers.h"
#include "maxon/uiconversions.h"

namespace maxonsdk
{

class UiConversionCustomGuiString : public maxon::Component<UiConversionCustomGuiString, maxon::UiConversionInterface>
{
	MAXON_COMPONENT();

public:

	static maxon::Result<void> Initialize();

	MAXON_METHOD maxon::Result<void> QuerySupportedDataTypes(maxon::BaseArray<maxon::DataType>& dataTypes) const;

	MAXON_METHOD maxon::Result<void> CreateC4DDescription(const maxon::DataType& dataType, Description& c4dDescription, const maxon::LanguageRef& language,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::DataDescription& mainDataDescription, const maxon::DataDescription& stringDescription, const DescID& mainId,
		const DescID& groupId, const maxon::PatchC4DDescriptionEntryDelegate& patchEntryFunc, maxon::DescTranslation& translateIds,
		const maxon::BaseArray<maxon::InternedId>& parentIds, const DescID& parentFoldId, const maxon::GetDataCallbackType& getDataCallback,
		const maxon::GetExtraDataCallbackType& getExtraDataDelegate) const;

	MAXON_METHOD maxon::Result<void> ConvertToC4D(GeData& output, const maxon::DataType& dataType, const maxon::Data& data, const DescID& descIdSuffix,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::GetExtraDataCallbackType& extraDataDelegate) const;

	MAXON_METHOD maxon::Result<maxon::Tuple<maxon::Data, maxon::Bool>> ConvertToCore(const maxon::DataType& dataType, const GeData& data, const DescID& descIdSuffix,
		const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::Data& oldData,
		const maxon::GetExtraDataCallbackType& extraDataDelegate) const;

private:
};

} // namespace maxonsdk
#endif // CUSTOMGUI_STRING_H__