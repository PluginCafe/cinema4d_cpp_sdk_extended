
#ifndef _CUSTOMDATA_CUSTOMGUI_IN_NODE_H__
#define _CUSTOMDATA_CUSTOMGUI_IN_NODE_H__



#include "customdata_customgui.h"
#include "maxon/uiconversions.h"


namespace maxon
{

namespace UiConversions
{

	// Declare the id where the UIconversion implementation will be registered.
	MAXON_DECLARATION(maxon::UiConversions::EntryType, UiDotData, "net.maxonsdk.ui.dotdataconversion");
} // namespace UiConversions
} // namespace maxon



#endif