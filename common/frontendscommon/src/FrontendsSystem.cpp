#include "FrontendsSystem.hpp"
#include "PonyTech-Frontend_include.hpp"
#include "external/HeartbeatRegularTask.hpp"

BEGIN_NAMESPACE(Frontend)

FrontendsSystem::FrontendsSystem(AbstractReportStatus& status, TunDeviceInterface &tun_interface, VpnSettingsProvider& settting_provider) :
buffer_provider(2000,32),
m_socket_engine(m_system_connector, buffer_provider),
m_system_connector(m_socket_engine, m_connection_system),
m_eap_md5_provider(settting_provider),
m_connection_system(buffer_provider, task_schedular, m_socket_engine, m_system_connector, status, settting_provider),
packet_forwarder(tun_interface, socket_system),
socket_system(buffer_provider, socket_message_forwarder),
socket_message_forwarder(buffer_provider, socket_system)
{    
    frontend_wrap_setup(TRUE);
    CrossWrap::Common::ipsec_init(task_schedular, packet_forwarder, buffer_provider);
}

FrontendsSystem::~FrontendsSystem()
{    
    frontend_wrap_setup(FALSE);
    CrossWrap::Common::ipsec_deinit();
}

void FrontendsSystem::startup() 
{
    task_schedular.start(4);
    m_socket_engine.init(1);

}
void FrontendsSystem::register_loader()
{
    m_eap_md5_provider.register_loader();
}

void FrontendsSystem::teardown() 
{
    m_eap_md5_provider.unregister_loader();
    socket_message_forwarder.set_heartbeat(SharedPtr<HeartbeatSession>());
    m_socket_engine.stop();
    m_connection_system.reset();
    task_schedular.shutdown(); 
}

END_NAMESPACE(Frontend)