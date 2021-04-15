#include "nodesystem_observer.h"

namespace maxonsdk
{

MAXON_DATATYPE_LOCAL(NodeSystemChangedHandlerRef, "net.maxonexample.datatype.nodesystemchangedhandlerref");

class NodeSystemChangedTicket
{
	explicit NodeSystemChangedTicket(maxon::UInt64 nodeSystemId, maxon::UInt64 observerId);

public:

	// We enforce heap allocation for weak referencing.
	static maxon::Result<maxon::GenericData> Create(maxon::UInt64 nodeSystemId, maxon::UInt64 observerId);

	~NodeSystemChangedTicket();
private:
	maxon::UInt64  _nodeSystemId = maxon::UInt64(-1);
	maxon::UInt64 _handlerId = maxon::UInt64(-1);
};

NodeSystemChangedTicket::NodeSystemChangedTicket(maxon::UInt64 nodeSystemId, maxon::UInt64 handlerId) : _nodeSystemId(nodeSystemId), _handlerId(handlerId)
{

}

maxon::Result<maxon::GenericData> NodeSystemChangedTicket::Create(maxon::UInt64 nodeSystemId, maxon::UInt64 observerId)
{
	iferr_scope;
	maxon::StrongRef<NodeSystemChangedTicket> ticket = NewObj(NodeSystemChangedTicket, nodeSystemId, observerId) iferr_return;
	maxon::GenericData data;
	data.Set(std::move(ticket)) iferr_return;
	return data;
}

NodeSystemChangedTicket::~NodeSystemChangedTicket()
{
	NodeSystemObserverManager::RemoveChangeHandler(_nodeSystemId, _handlerId);
}

NodeSystemChangedMonitor::NodeSystemChangedMonitor(maxon::UInt64 nodeSystemId) : _nodeSystemId(nodeSystemId)
{

}

NodeSystemChangedMonitor::~NodeSystemChangedMonitor()
{
	maxon::nodes::NodeSystemManagerRef manager = _weakManager;
	if (manager != nullptr)
	{
		if (_systemChangedTicket != nullptr)
		{
			manager.ObservableTransactionCommitted().RemoveObserver(_systemChangedTicket) iferr_ignore("");
		}
		if (_systemDestroyedTicket != nullptr)
		{
			manager.ObserverDestroyed().RemoveObserver(_systemDestroyedTicket) iferr_ignore("");
		}
	}
}

maxon::Result<NodeSystemChangedMonitorRef> NodeSystemChangedMonitor::Create(maxon::UInt64 nodeSystemId, const maxon::nodes::NodesGraphModelRef& graph)
{
	iferr_scope;
	NodeSystemChangedMonitorRef monitor = NewObj(NodeSystemChangedMonitor, nodeSystemId) iferr_return;
	NodeSystemChangedMonitorWeakRef weakMonitor = monitor;

	maxon::nodes::NodeSystemManagerRef manager = graph.GetManager();

	monitor->_systemChangedTicket = manager.ObservableTransactionCommitted().AddObserver(
	[weakMonitor](const maxon::nodes::NodeSystemManagerRef& manager, const maxon::DataDictionary&) -> void
	{
		NodeSystemChangedMonitorRef monitor = weakMonitor;
		if (monitor != nullptr)
		{
			monitor->NotifySystemChanged();
		}
	}) iferr_return;

	monitor->_systemDestroyedTicket = manager.ObserverDestroyed().AddObserver(
	[weakMonitor](maxon::ObserverObjectInterface* sender) -> void
	{
		NodeSystemChangedMonitorRef monitor = weakMonitor;
		if (monitor != nullptr)
		{
			monitor->NotifySystemDestroyed();
		}
	}) iferr_return;

	monitor->_weakManager = manager;
	return monitor;
}

maxon::Result<void> NodeSystemChangedMonitor::RegisterHandler(maxon::UInt64 handlerId, NodeSystemChangedHandlerRef&& changeHandler)
{
	iferr_scope;

	NodeSystemChangedHandlerRef changeHandlerToNotify = changeHandler;

	MAXON_SCOPE // Lock
	{
		maxon::ScopedLock guard(_lock);
		_handlers.Set(handlerId, std::move(changeHandler)) iferr_return;
	} // Unlock
	

	MAXON_SCOPE // Notify immediately to process the current state.
	{
		maxon::nodes::NodeSystemManagerRef manager = _weakManager;
		if (manager != nullptr)
		{
			maxon::nodes::NodesGraphModelRef graph = manager.GetMainView();
			changeHandlerToNotify->HandleGraphChanged(graph) iferr_return;
		}
	}

	return maxon::OK;
}

void NodeSystemChangedMonitor::RemoveHandler(maxon::UInt64 handlerId)
{
	maxon::ScopedLock guard(_lock);
	_handlers.Erase(handlerId) iferr_ignore("");
}

void NodeSystemChangedMonitor::NotifySystemChanged()
{
	// We operate on safe copy of the handlers references to ensure they can be properly unregistered without risk of recursive deadlocking
	// in the case of re-entry.
	maxon::DataDictionary handlersToNotify = _handlers;

	maxon::nodes::NodeSystemManagerRef manager = _weakManager;
	if (manager != nullptr)
	{
		maxon::nodes::NodesGraphModelRef graph = manager.GetMainView();

		for (auto handlerEntry : handlersToNotify)
		{
			ifnoerr (NodeSystemChangedHandlerRef changeHandler = handlerEntry.GetValue().Get<NodeSystemChangedHandlerRef>())
			{
				iferr (changeHandler->HandleGraphChanged(graph))
				{
					// Proper error handling here requires some advanced logging logic back to the original observation requester.
					err.CritStop();
				}
			}
		}
	}
}

void NodeSystemChangedMonitor::NotifySystemDestroyed()
{
	NodeSystemObserverManager::RemoveMonitor(_nodeSystemId);
}

NodeSystemObserverManagerRef g_manager;

maxon::Result<void> NodeSystemObserverManager::Initialize()
{
	iferr_scope;
	g_manager = NewObj(NodeSystemObserverManager) iferr_return;
	return maxon::OK;
}

void NodeSystemObserverManager::Shutdown()
{
	g_manager = nullptr;
}

maxon::Result<maxon::GenericData> NodeSystemObserverManager::RegisterChangeHandler(const maxon::nodes::NodesGraphModelRef& graph, NodeSystemChangedHandlerRef&& changeHandler)
{
	iferr_scope;

	NodeSystemObserverManagerRef manager = g_manager;
	CheckState(manager != nullptr);

	maxon::GenericData registrationTicket = manager->_RegisterChangeHandler(graph, std::move(changeHandler)) iferr_return;
	return registrationTicket;
}

void NodeSystemObserverManager::RemoveChangeHandler(maxon::UInt64 nodeSystemId, maxon::UInt64 handlerId)
{
	NodeSystemObserverManagerRef manager = g_manager;
	if (manager != nullptr)
	{
		manager->_RemoveChangeHandler(nodeSystemId, handlerId);
	}
}

void NodeSystemObserverManager::RemoveMonitor(maxon::UInt64 nodeSystemId)
{
	NodeSystemObserverManagerRef manager = g_manager;
	if (manager != nullptr)
	{
		manager->_RemoveMonitor(nodeSystemId);
	}
}

maxon::Result<maxon::GenericData> NodeSystemObserverManager::_RegisterChangeHandler(const maxon::nodes::NodesGraphModelRef& graph, NodeSystemChangedHandlerRef&& changeHandler)
{
	iferr_scope;
	const maxon::UInt64 handlerId = GetHandlerId();
	const maxon::UInt64 nodeSystemId = GetNodeSystemId(graph);

	NodeSystemChangedMonitorRef monitor;
	MAXON_SCOPE // Let's reuse the existing observer if possible.
	{
		maxon::ScopedLock guard(_lock);
		auto monitorEntry = _changeMonitors.Find(nodeSystemId);

		if (monitorEntry != nullptr)
		{
			monitor = monitorEntry->GetValue();
		}
		else
		{
			monitor = NodeSystemChangedMonitor::Create(nodeSystemId, graph) iferr_return;
			_changeMonitors.Insert(nodeSystemId, monitor) iferr_return;
		}

		MAXON_SCOPE // Register the observer
		{
			if (_changeHandlers.Contains(nodeSystemId) == false)
			{
				_changeHandlers.Insert(nodeSystemId, maxon::HashSet<maxon::UInt64>()) iferr_return;
			}

			// We insert the observer id before issuing any callbacks.
			auto changeHandlersEntry = _changeHandlers.Find(nodeSystemId);
			CheckState(changeHandlersEntry != nullptr);
			maxon::HashSet<maxon::UInt64>& knownHandlers = changeHandlersEntry->GetValue();
			knownHandlers.Insert(handlerId) iferr_return;
		}
	} // Unlock.

	// Lets' keep track of the ticket.
	maxon::GenericData ticket = NodeSystemChangedTicket::Create(nodeSystemId, handlerId) iferr_return;

	monitor->RegisterHandler(handlerId, std::move(changeHandler)) iferr_return;

	return ticket;
}

void NodeSystemObserverManager::_RemoveChangeHandler(maxon::UInt64 nodeSystemId, maxon::UInt64 observerId)
{
	NodeSystemChangedMonitorRef monitorToNotify;
	NodeSystemChangedMonitorRef monitorToRelease;
	MAXON_SCOPE // Lock
	{
		maxon::ScopedLock guard(_lock);
		maxon::Bool releaseSystem = true;
		auto changeHandlersEntry = _changeHandlers.Find(nodeSystemId);
		if (changeHandlersEntry != nullptr)
		{
			maxon::HashSet<maxon::UInt64>& knownHandlers = changeHandlersEntry->GetValue();
			knownHandlers.Erase(observerId);
			if (knownHandlers.IsEmpty() == true)
			{
				_changeHandlers.Erase(changeHandlersEntry);
			}
			else
			{
				releaseSystem = false;
			}
		}

		auto monitorEntry = _changeMonitors.Find(nodeSystemId);
		{
			if (monitorEntry != nullptr)
			{
				monitorToNotify = monitorEntry->GetValue();

				if (releaseSystem == true)
				{
					monitorToRelease = monitorEntry->GetValue();
					_changeMonitors.Erase(monitorEntry);
				}
			}
		}
	} // Unlock

	if (monitorToNotify != nullptr)
	{
		monitorToNotify->RemoveHandler(observerId);
	}
}

void NodeSystemObserverManager::_RemoveMonitor(maxon::UInt64 nodeSystemId)
{
	NodeSystemChangedMonitorRef monitorToRelease;
	MAXON_SCOPE // Lock
	{ 
		maxon::ScopedLock guard(_lock);
		auto monitorEntry = _changeMonitors.Find(nodeSystemId);
		if (monitorEntry != nullptr)
		{
			monitorToRelease = monitorEntry->GetValue();
			_changeMonitors.Erase(monitorEntry);
		}
	} // Unlock
}

maxon::UInt64 NodeSystemObserverManager::GetNodeSystemId(const maxon::nodes::NodesGraphModelRef& graph)
{
	const void* nodeSystemPointer = &graph.GetNodeSystem();
	return maxon::UInt64(nodeSystemPointer);
}

maxon::UInt64 NodeSystemObserverManager::GetHandlerId()
{
	return _handlerIdFactory.SwapIncrement();
}

} // namespace maxonsdk