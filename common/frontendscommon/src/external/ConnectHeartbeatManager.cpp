#include "ConnectHeartbeatManager.hpp"
#include "HeartbeatRegularTask.hpp"
#include "external/SocketMessageForwarder.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

ConnectHeartbeatManager::ConnectHeartbeatManager(SocketSystem& socket_system, 
                            SocketMessageForwarder& forwarder, VpnFrontendService& service) :
m_service(service),
m_socket_system(socket_system),
m_message_forwarder(forwarder)
{
}

ConnectHeartbeatManager::~ConnectHeartbeatManager()
{
    shutdown();
}

void ConnectHeartbeatManager::startup(FmWork::TaskSubmitter& submitter)
{
    shutdown();

    m_data.ref(new HeartbeatSession());
    m_message_forwarder.set_heartbeat(m_data);
    AutoPtr<FmWork::Task> task(new HeartbeatRegularTask(m_socket_system, m_service, m_data));
    submitter.add(task);
}

void ConnectHeartbeatManager::shutdown()
{
    if (m_data.is_null()) return;
    m_data->stopped = 1;
    m_data.unref();
}


END_NAMESPACE(Frontend)