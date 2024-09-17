#ifndef CUSTOMGUI_BUNDLE_H__
#define CUSTOMGUI_BUNDLE_H__

#include "lib_description.h"
#include "maxon/desctranslation.h"

#include "customgui_base.h"
#include "maxon/customnodegui.h"

#define BUNDLEGUI_ID 1053656

namespace maxonsdk
{

class BundleGuiData : public cinema::CustomGuiData
{
public:
	virtual cinema::Int32 GetId() override;
	virtual cinema::CDialog* Alloc(const cinema::BaseContainer& settings) override;
	virtual void Free(cinema::CDialog* dlg, void* userdata) override;
	virtual const cinema::Char* GetResourceSym() override;
	virtual cinema::CustomProperty* GetProperties() override;
	virtual cinema::Int32 GetResourceDataType(cinema::Int32*& table) override;

	static maxon::Result<void> RegisterGuiPlugin();
};

struct NodeAttribute
{
	cinema::DescID _description;
	maxon::String _name;
	cinema::Bool _hasGUI = false;
	cinema::Bool _hasAttributeManagerValue = false;
};

struct NodeData
{
	maxon::NodePath _nodePath;
	cinema::Int64 _currentDocumentAddress = 0;

	maxon::BaseArray<NodeAttribute> _attributes;
};

class BundleGui;
struct NodeDataContext
{
	maxon::AttributeManagerProxyRef				_attributeManager;
	maxon::BaseArray<cinema::BaseList2D*> _objects;
	maxon::BaseArray<cinema::DescID>			_replaceIds;
	cinema::BaseDocument*									_baseDocument = nullptr;

	static maxon::Result<NodeDataContext> Get(BundleGui& bundle);

};

enum class BUNDLELAYOUTTYPE
{
	DEBUG,
	DEFAULT,
} MAXON_ENUM_LIST(BUNDLELAYOUTTYPE);

class BundleGui : public maxon::CustomNodeGuiBase
{
	INSTANCEOF(BundleGui, maxon::CustomNodeGuiBase)


public:
	BundleGui(const cinema::BaseContainer& settings, cinema::CUSTOMGUIPLUGIN* plugin);
	virtual cinema::Bool InitValues() override;
	virtual cinema::Bool CreateLayout() override;
	virtual cinema::Bool Command(cinema::Int32 id, const cinema::BaseContainer& msg) override;
	virtual cinema::Bool CoreMessage(cinema::Int32 id, const cinema::BaseContainer& msg) override;
	virtual cinema::Int32 Message(const cinema::BaseContainer& msg, cinema::BaseContainer& result) override;

private:

	void CreateErrorText(const cinema::String& text, const maxon::AttributeManagerProxyRef& attributeManager);
	maxon::Result<void> CreateAttributesTable(NodeData&& data, const NodeDataContext& context);

	static maxon::Result<NodeData> CollectNodeData(const NodeDataContext& context);
	static maxon::Result<void> CreateDebugGUI(NodeData& data, const NodeDataContext& context, cinema::iBaseCustomGui& gui);
	static maxon::Result<void> CreateDefaultGUI(NodeData& data, const NodeDataContext& context, cinema::iBaseCustomGui& gui);

	NodeData _nodeData;
	const BUNDLELAYOUTTYPE _layoutType = BUNDLELAYOUTTYPE::DEBUG;
};


} // namespace maxonsdk

#endif // CUSTOMGUI_BUNDLE_H__
