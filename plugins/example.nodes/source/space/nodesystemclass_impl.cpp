#include "nodesystemclass_impl.h"

#include "maxon/nodetemplate.h"

namespace maxonsdk
{
	
MAXON_METHOD maxon::Result<maxon::Bool> NodeSystemClassExample::SupportsImpl(const maxon::nodes::NodeTemplate& templ) const
{
	const maxon::Id& id = templ.GetId();
	if (id == maxon::Id("net.maxon.node.arithmetic") || id == maxon::Id("net.maxon.node.scale"))
	{
		return true;
	}
	return super.SupportsImpl(templ);
}

MAXON_METHOD maxon::Result<maxon::nodes::NodeSystem> NodeSystemClassExample::InstantiateImpl(const maxon::nodes::InstantiationTrace& parent, const maxon::nodes::NodeTemplate& templ, const maxon::nodes::TemplateArguments& args) const
{
	iferr_scope;
	using namespace maxon;
	using namespace maxon::nodes;
	NodeSystem sys = super.InstantiateImpl(parent, templ, args) iferr_return;
	const Id& id = templ.GetId();
	if (id == Id("net.maxon.node.arithmetic"))
	{
		MutableRoot root = sys.BeginModification(parent.GetLookupRepository()) iferr_return;
		root.SetTemplate(parent, templ, args) iferr_return;
		Tuple<Id, Data> enumerators[2];
		enumerators[0].first = Id("float");
		enumerators[0].second.Set(enumerators[0].first) iferr_return;
		enumerators[1].first = Id("int");
		enumerators[1].second.Set(enumerators[1].first) iferr_return;
		DataType enumType = DataTypeLib::GetEnumType(GetDataType<Id>(), enumerators) iferr_return;
		root.GetInputs().FindPort(Id("datatype")).SetType(enumType) iferr_return;
		sys = root.EndModification() iferr_return;
	}
	return sys;
}

	
MAXON_METHOD maxon::Result<maxon::Bool> NodeSystemClassExample::SupportsVariant(const maxon::nodes::NodeTemplate& templ, const maxon::Block<const maxon::Id>& variant) const
{
	return *variant.GetLast() != maxon::Id("net.maxon.parametrictype.col<4,float>");
}
		
MAXON_METHOD maxon::Result<maxon::Id> NodeSystemClassExample::SubstituteVariant(const maxon::nodes::NodeTemplate& templ, const maxon::Block<const maxon::Id>& variant, const maxon::Block<const maxon::Id>& options) const
{
	if (*variant.GetLast() == maxon::Id("net.maxon.parametrictype.col<4,float>"))
	{
		return maxon::Id("net.maxon.parametrictype.col<3,float>");
	}
	return {};
}
	
} // namespace maxonsdk
