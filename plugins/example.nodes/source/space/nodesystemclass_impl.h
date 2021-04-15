#ifndef NODESYSTEMCLASS_IMPL_H__
#define NODESYSTEMCLASS_IMPL_H__

#include "maxon/nodes_corenodes_base.h"

namespace maxonsdk
{
	
class NodeSystemClassExample : public maxon::Component<NodeSystemClassExample, maxon::nodes::NodeSystemClassInterface>
{
	MAXON_COMPONENT(FINAL, maxon::nodes::BaseCoreNodesNodeSystemClass().GetClass());
	
public:
	MAXON_METHOD maxon::Result<maxon::Bool> SupportsImpl(const maxon::nodes::NodeTemplate& templ) const;
	
	MAXON_METHOD maxon::Result<maxon::nodes::NodeSystem> InstantiateImpl(const maxon::nodes::InstantiationTrace& parent, const maxon::nodes::NodeTemplate& templ, const maxon::nodes::TemplateArguments& args) const;
	
	MAXON_METHOD maxon::Result<maxon::Bool> SupportsVariant(const maxon::nodes::NodeTemplate& templ, const maxon::Block<const maxon::Id>& variant) const;
	
	MAXON_METHOD maxon::Result<maxon::Id> SubstituteVariant(const maxon::nodes::NodeTemplate& templ, const maxon::Block<const maxon::Id>& variant, const maxon::Block<const maxon::Id>& options) const;
};

} // namespace maxonsdk
#endif // NODESYSTEMCLASS_IMPL_H__
