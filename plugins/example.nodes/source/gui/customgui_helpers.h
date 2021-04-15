#ifndef CUSTOMGUI_HELPERS_H__
#define CUSTOMGUI_HELPERS_H__

#include "maxon/datadictionary.h"
#include "lib_description.h"
#include "maxon/desctranslation.h"
#include "maxon/datadescription_data.h"

namespace maxonsdk
{

// We encapsulate some static helpers for the 'glue' between new data types defined for nodes and the classical GUI descriptions.
class CustomGuiHelpers
{
public:

	static maxon::Result<maxon::BaseArray<maxon::InternedId>> GetIdArray(const maxon::DataDictionary& guiEntry, const maxon::BaseArray<maxon::InternedId>& parentIds);

	static const DescID& GetRealGroupId(const DescID& groupIdA, const maxon::DataDictionary& guiEntry, maxon::DescTranslation& translateIds);

	static maxon::Result<void> SetPDescription(Description& c4dDescription, maxon::DescTranslation& translateIds, const DescID& descId, const BaseContainer& param,
		const DescID& groupIdA, const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::BaseArray<maxon::InternedId>& parentIds);

};

} // namespace maxonsdk

#endif // CUSTOMGUI_HELPERS_H__