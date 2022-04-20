
#ifndef _CUSTOMDATA_CUSTOMGUI_IN_NODE_H__
#define _CUSTOMDATA_CUSTOMGUI_IN_NODE_H__


#include "maxon/uiconversions.h"
#include "customdata_customgui.h"


namespace maxon
{

namespace UiConversions
{

	// We declare the id where we will register the UIconversion implementation.
	MAXON_DECLARATION(UiConversions::EntryType, UiDotData, "net.maxonsdk.ui.dotdataconversion");
} // namespace UiConversions
} // namespace maxon



#endif