#ifndef CUSTOMGUI_BUNDLE_H__
#define CUSTOMGUI_BUNDLE_H__

#include "lib_description.h"
#include "maxon/desctranslation.h"

#include "customgui_base.h"
#include "maxon/customnodegui.h"

#define BUNDLEGUI_ID 1053656

namespace maxonsdk
{

class BundleGuiData : public CustomGuiData
{
public:
	virtual Int32 GetId() override;
	virtual ::CDialog* Alloc(const BaseContainer& settings) override;
	virtual void Free(::CDialog* dlg, void* userdata) override;
	virtual const Char* GetResourceSym() override;
	virtual CustomProperty* GetProperties() override;
	virtual Int32 GetResourceDataType(Int32*& table) override;

	static maxon::Result<void> RegisterGuiPlugin();
};

struct NodeAttribute
{
	DescID _description;
	maxon::String _name;
	Bool _hasGUI = false;
	Bool _hasAttributeManagerValue = false;
};

struct NodeData
{
	maxon::NodePath _nodePath;
	Int64 _currentDocumentAddress = 0;

	maxon::BaseArray<NodeAttribute> _attributes;
};

class BundleGui;
struct NodeDataContext
{
	maxon::AttributeManagerProxyRef _attributeManager;
	maxon::BaseArray<BaseList2D*> _objects;
	maxon::BaseArray<DescID> _replaceIds;
	BaseDocument* _baseDocument = nullptr;

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
	BundleGui(const BaseContainer& settings, CUSTOMGUIPLUGIN* plugin);
	virtual Bool InitValues() override;
	virtual Bool CreateLayout() override;
	virtual Bool Command(Int32 id, const BaseContainer& msg) override;
	virtual Bool CoreMessage(Int32 id, const BaseContainer& msg) override;
	virtual Int32 Message(const BaseContainer& msg, BaseContainer& result) override;

private:

	void CreateErrorText(const String& text, const maxon::AttributeManagerProxyRef& attributeManager);
	maxon::Result<void> CreateAttributesTable(NodeData&& data, const NodeDataContext& context);

	static maxon::Result<NodeData> CollectNodeData(const NodeDataContext& context);
	static maxon::Result<void> CreateDebugGUI(NodeData& data, const NodeDataContext& context, iBaseCustomGui& gui);
	static maxon::Result<void> CreateDefaultGUI(NodeData& data, const NodeDataContext& context, iBaseCustomGui& gui);

	NodeData _nodeData;
	const BUNDLELAYOUTTYPE _layoutType = BUNDLELAYOUTTYPE::DEBUG;
};


} // namespace maxonsdk

#endif // CUSTOMGUI_BUNDLE_H__