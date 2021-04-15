#include "customgui_helpers.h"

#include "maxon/datatype.h"
#include "maxon/datadescription_ui.h"
#include "maxon/nodepath.h"

namespace maxonsdk
{

maxon::Result<maxon::BaseArray<maxon::InternedId>> CustomGuiHelpers::GetIdArray(const maxon::DataDictionary& guiEntry, const maxon::BaseArray<maxon::InternedId>& parentIds)
{
	iferr_scope;

	maxon::InternedId eid = guiEntry.Get(maxon::DESCRIPTION::BASE::IDENTIFIER) iferr_return;
	maxon::Bool			 isVariadic = guiEntry.Get(maxon::DESCRIPTION::DATA::BASE::ISVARIADIC, false); // #variadicPortIndicator

	maxon::BaseArray<maxon::InternedId> dataArray;
	dataArray.CopyFrom(parentIds) iferr_return;
	if (!isVariadic)
	{
		maxon::NodePath::ParsePath(eid.ToBlock(), dataArray) iferr_return;
	}

	return std::move(dataArray);
}

const DescID& CustomGuiHelpers::GetRealGroupId(const DescID& groupIdA, const maxon::DataDictionary& guiEntry, maxon::DescTranslation& translateIds)
{
	maxon::InternedId assignToGroupId = guiEntry.Get(maxon::DESCRIPTION::UI::BASE::GROUPID, maxon::InternedId());
	if (assignToGroupId.IsEmpty())
		return groupIdA;

	const DescID* assignToGroupIdDescId = translateIds._groupIdMap.FindValue(assignToGroupId);
	if (!assignToGroupIdDescId)
		return groupIdA;

	return *assignToGroupIdDescId;
}

maxon::Result<void> CustomGuiHelpers::SetPDescription(Description& c4dDescription, maxon::DescTranslation& translateIds, const DescID& descId, const BaseContainer& param,
	const DescID& groupIdA, const maxon::DataDictionary& dataEntry, const maxon::DataDictionary& guiEntry, const maxon::BaseArray<maxon::InternedId>& parentIds)
{
	iferr_scope;

	const DescID& groupId = GetRealGroupId(groupIdA, guiEntry, translateIds);

	c4dDescription.SetParameter(descId, param, groupId);

	maxon::BaseArray<maxon::InternedId> dataArray = GetIdArray(guiEntry, parentIds) iferr_return;

	maxon::Id guiTypeId = guiEntry.Get(maxon::DESCRIPTION::UI::BASE::GUITYPEID, maxon::Id());
	maxon::Id dataTypeId = dataEntry.Get(maxon::DESCRIPTION::DATA::BASE::DATATYPE, maxon::Id());
	translateIds._descIdMap.Insert(descId, { std::move(dataArray), dataTypeId, guiTypeId, maxon::Id(), dataEntry, guiEntry }) iferr_return;

	return maxon::OK;
}

} // namespace maxonsdk
