#include "maxon/datadescription_ui.h"
#include "maxon/datadescription_string.h"
#include "maxon/datadescription_nodes.h"
#include "maxon/lazylanguagestringdatadescription.h"
#include "space/customnode-customnodespace_descriptions.h"
#include "nodesystemclass_impl.h"
#include "maxon/stringresource.h"

namespace maxonsdk
{

// This example implements a node with a template parameter port.
// The user can drive that port with a String argument which is parsed as a comma-separated list of names.
// For each name a port is created dynamically. If the name starts with 'i' its type is Int,
// for 's' the type is String, otherwise Float.
class DynamicNode : public maxon::Component<DynamicNode, maxon::nodes::NodeTemplateInterface>
{
	MAXON_COMPONENT(NORMAL, maxon::nodes::NodeTemplateBaseClass);

public:
	MAXON_METHOD maxon::Result<maxon::Bool> SupportsImpl(const maxon::nodes::NodeSystemClass& cls) const
	{
		// Make sure that this node only shows up in the example node space.
		return cls.GetClass() == NodeSystemClassExample::GetClass();
	}

	MAXON_METHOD maxon::Result<maxon::nodes::NodeSystem> InstantiateImpl(const maxon::nodes::InstantiationTrace& parent, const maxon::nodes::TemplateArguments& args) const
	{
		iferr_scope;

		maxon::nodes::MutableRoot root = parent.GetNodeSystemClass().CreateNodeSystem() iferr_return;

		// Add the template parameter port.
		maxon::nodes::MutablePort codePort = root.GetInputs().AddPort(maxonexample::NODE::DYNAMICNODE::CODE) iferr_return;
		codePort.SetType<maxon::String>() iferr_return;
		codePort.SetValue<decltype(maxon::nodes::TemplateParameter)>(true) iferr_return;

		// Get the template argument for the code.
		const String& code = args.GetValueArgument<String>(codePort).GetValueOrNull();
		Int order = 0;

		// Parse code, here we just split the code into its comma-separated parts and create a port for each part.
		code.Split(","_s, true,
			[&root, &order] (const maxon::String& part) -> maxon::Result<maxon::Bool>
			{
				iferr_scope;
				if (part.IsPopulated())
				{
					maxon::Id portId;
					portId.Init(part) iferr_return;
					// Add port for part.
					maxon::nodes::MutablePort port = root.GetInputs().AddPort(portId) iferr_return;
					if (part[0] == 'i')
					{
						// Create a port of type Int.
						port.SetType<maxon::Int>() iferr_return;
						// Directly set the name.
						port.SetValue<decltype(maxon::NODE::BASE::NAME)>(part) iferr_return;
					}
					else if (part[0] == 's')
					{
						// Create a port of type String.
						port.SetType<maxon::String>() iferr_return;
						port.SetValue<decltype(maxon::NODE::BASE::NAME)>(part) iferr_return;
					}
					else
					{
						// Default case: Use Float.
						port.SetType<maxon::Float>() iferr_return;

						// This shows how to set up UI properties dynamically, in this case we configure a slider.
						maxon::DataDictionary data;
						data.Set(maxon::DESCRIPTION::DATA::BASE::DATATYPE, maxon::GetDataType<maxon::AFloat>().GetId()) iferr_return;
						port.SetValue<decltype(maxon::nodes::PortDescriptionData)>(std::move(data)) iferr_return;

						maxon::DataDictionary ui;
						ui.Set(maxon::DESCRIPTION::UI::BASE::GUITYPEID, maxon::Id("net.maxon.ui.number")) iferr_return;
						ui.Set(maxon::DESCRIPTION::UI::NET::MAXON::UI::NUMBER::SLIDER, true) iferr_return;
						ui.Set(maxon::DESCRIPTION::UI::BASE::ADDMINMAX::LIMITVALUE, true) iferr_return;
						ui.Set(maxon::DESCRIPTION::UI::BASE::ADDMINMAX::MINVALUE, maxon::Data(maxon::Float(0.0_f))) iferr_return;
						ui.Set(maxon::DESCRIPTION::UI::BASE::ADDMINMAX::MAXVALUE, maxon::Data(maxon::Float(10.0_f))) iferr_return;
						ui.Set(maxon::DESCRIPTION::UI::BASE::ADDMINMAX::STEPVALUE, maxon::Data(maxon::Float(0.1_f))) iferr_return;
						port.SetValue<decltype(maxon::nodes::PortDescriptionUi)>(std::move(ui)) iferr_return;

						// This shows how to use localization for dynamically created ports.
						maxon::LazyLanguageDictionary languageDict;

						maxon::DataDictionary english;
						english.Set(maxon::DESCRIPTION::STRING::BASE::TRANSLATEDSTRING, "Localized port name for "_s + part) iferr_return;
						languageDict.Set(maxon::LANGUAGE_ENGLISH_ID, english) iferr_return;

						port.SetValue<decltype(maxon::nodes::PortDescriptionStringLazy)>(std::move(languageDict)) iferr_return;
					}
					port.SetValue<decltype(maxon::NODE::ATTRIBUTE::ORDERINDEX)>(++order) iferr_return;
				}
				return true;
			}) iferr_return;

		root.GetOutputs().AddPort(maxonexample::NODE::DYNAMICNODE::RESULT) iferr_return;

		root.SetTemplate(parent, self, args) iferr_return;

		return root.EndModification();
	}
};

MAXON_COMPONENT_CLASS_REGISTER(DynamicNode, "net.maxonexample.class.dynamicnode");

MAXON_DECLARATION_REGISTER(maxon::nodes::BuiltinNodes, maxonexample::NODE::DYNAMICNODE::GetId())
{
	return DynamicNode::GetClass().Create();
}

} // namespace maxonsdk
