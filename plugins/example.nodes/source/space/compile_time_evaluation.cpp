#include "nodesystemclass_impl.h"
#include "space/customnode-customnodespace_descriptions.h"
#include "maxon/nodetemplate.h"

namespace maxonsdk
{

// This example implements a color invert node which makes the color inversion at "compile time"
// (i.e., within the editor) whenever possible (i.e., when the input port has a known non-dynamic value).
// To enable compile-time evaluation, you have to implement a derivation handler to be used for the
// node instantiation. Easiest way is to make the ColorInvertNode a NodeSystemDerivationHandlerInterface itself
// (but one could use another class for a cleaner separation between NodeTemplateInterface
// and NodeSystemDerivationHandlerInterface).
class ColorInvertNode : public maxon::Component<ColorInvertNode, maxon::nodes::NodeTemplateInterface, maxon::nodes::NodeSystemDerivationHandlerInterface>
{
	// Add NodeSystemDerivationHandlerBaseClass in addition to NodeTemplateBaseClass because we implement NodeSystemDerivationHandlerInterface
	// and want to inherit default implementations for some of its methods.
	MAXON_COMPONENT(NORMAL, maxon::nodes::NodeTemplateBaseClass, maxon::nodes::NodeSystemDerivationHandlerBaseClass);

public:
	MAXON_METHOD maxon::Result<maxon::Bool> SupportsImpl(const maxon::nodes::NodeSystemClass& cls) const
	{
		// Make sure that this node only shows up in the example node space.
		return cls.GetClass() == NodeSystemClassExample::GetClass();
	}

	MAXON_METHOD maxon::Result<maxon::nodes::NodeSystem> InstantiateImpl(const maxon::nodes::InstantiationTrace& parent, const maxon::nodes::TemplateArguments& args) const
	{
		iferr_scope;

		maxon::nodes::MutableRoot root = parent.CreateNodeSystem() iferr_return;

		// Override the derivation handler for the instantiation to use self.
		root.SetDerivationHandler(self) iferr_return;

		// Create the output port.
		maxon::nodes::MutablePort output = root.GetOutputs().AddPort(maxonexample::NODE::INVERTNODE::OUT) iferr_return;
		output.SetType<maxon::Color>() iferr_return;
		// Mark the output port as dynamic. This means that its value is usually computed by shader evaluation.
		// (But we'll add an override in OverrideEffectivePortValue for the case when the input port has a constant value.)
		output.SetValue(maxon::nodes::Dynamic, true) iferr_return;

		// Create the input port.
		maxon::nodes::MutablePort input = root.GetInputs().AddPort(maxonexample::NODE::INVERTNODE::IN) iferr_return;
		input.SetType<maxon::Color>() iferr_return;

		// Add a dependency from input to output. That's necessary for the evaluation at compile time.
		input.Connect(output, maxon::Wires(maxon::Wires::DEPENDENCY)) iferr_return;

		root.SetTemplate(parent, self, args) iferr_return;

		return root.EndModification();
	}

	MAXON_METHOD maxon::Result<maxon::Bool> OverrideEffectivePortValue(const maxon::nodes::Port& port, maxon::nodes::VALUEMODE& outMode, maxon::Data& outValue) const
	{
		// This method is called whenever there are changes which might influence the effective port value of the given port.
		// Here, we want to set the effective port value of this node's output to the inverse color of this node's input
		// when the input color has a known (compile-time constant) value.

		iferr_scope;

		// Check if we are at the output port.
		if (port.GetId() != maxonexample::NODE::INVERTNODE::OUT)
		{
			return false;
		}

		// Locate the input port. Note that the access of the EffectivePortValue attribute of the input port
		// is only allowed because we made a dependency connection from the input port to the output port.
		// Without that connection the access would be illegal, and can result in errors or undefined values.
		const maxon::nodes::Port input = port.GetNode().GetInputs().FindPort(maxonexample::NODE::INVERTNODE::IN) iferr_return;
		CheckAssert(input);

		const maxon::Opt<const maxon::Color&> color = input.GetEffectivePortValue<maxon::Color>() iferr_return;
		if (!color)
		{
			// If there's no effective port value, this means that the input port has no known value.
			return false;
		}

		// Override effective port value by inverse of color.

		const maxon::Color inverse = maxon::Color(1.0) - *color;

		outMode = maxon::nodes::VALUEMODE::CONSTANT;
		outValue.Set(inverse) iferr_return;

		return true;
	}
};

MAXON_COMPONENT_CLASS_REGISTER(ColorInvertNode, "net.maxonexample.class.invertnode");

MAXON_DECLARATION_REGISTER(maxon::nodes::BuiltinNodes, maxonexample::NODE::INVERTNODE::GetId())
{
	return ColorInvertNode::GetClass().Create().SetId({objectId, maxon::Id()}, 0, maxon::AssetInterface::GetBuiltinRepository());
}

} // namespace maxonsdk
