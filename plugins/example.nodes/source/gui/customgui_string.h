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

private:
};

} // namespace maxonsdk
#endif // CUSTOMGUI_STRING_H__
