#include "external/VpnFrontendService.hpp"
#include "service/VpnSettingsProvider.hpp"
#include "StartConnectionTask.hpp"
#include "ConnectHeartbeatManager.hpp"
#include "../service/ConnectionTask.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

VpnFrontendService::VpnFrontendService(const VpnSettingsProvider& provider, ConnectionSystem& connection,
                                   SocketSystem& socket_system, SocketMessageForwarder& message_forwarder, 
                                   FmWork::TaskSubmitter& submitter) :
m_provider(provider),
m_submitter(submitter),
m_connection(connection)
{    
    m_library_inited = false;
    m_heartbeat.ref(new ConnectHeartbeatManager(socket_system, message_forwarder, *this));
}

VpnFrontendService::~VpnFrontendService()
{    
}

bool VpnFrontendService::startup()
{
    if (!m_library_inited)
    { 
        if (frontend_library_init() == FALSE) return false;
        m_library_inited = true;
    }
    return true;
}

void VpnFrontendService::teardown()
{
    if (m_library_inited) 
    {
        frontend_library_deinit(); 
        m_library_inited = false;
    }
    frontend_kernel_ipsec_remove();
    CrossWrap::Common::ipsec_teardown();
    m_heartbeat->shutdown();   
    
    frontend_charon_unload_static_plugin();
}

void VpnFrontendService::register_loader()
{
    int ret = frontend_kernel_ipsec_provide(this, sock_bypass_wrap);
    if (ret == FALSE)
    {
        LOG_ERROR(field_frontends, "Fails to register kernel for frontends into plugin");
    }
}

void VpnFrontendService::reload_settings()
{
    auto settings = m_provider.settings();
    if (settings == nullptr) 
    {
        LOG_ERROR(field_frontends, "No Vpn Setting object");
        return;
    }    
    frontend_plugins_settings_reload(settings->mtu(), settings->keep_alive());
}

bool VpnFrontendService::load_plugins()
{
    return frontend_charon_init_plugin() != FALSE;
}

Net::SockAddress VpnFrontendService::is_new_request() const
{
    auto settings = m_provider.settings();
    if (settings == nullptr) return Net::SockAddress();

    String gateway = settings->gateway();
    try
    {
        Array<Net::SockAddress> addresses;
        Net::SockAddress::parse(addresses, gateway, 1);
        if (addresses.count() == 0) return Net::SockAddress();

        Net::Port port = addresses[0].port();
        return port == 0 ? Net::SockAddress() : addresses[0];
    }
    catch(const Exception::Exception& ex) {}
    return Net::SockAddress();
}


void VpnFrontendService::start_connect()
{
    Net::SockAddress address = is_new_request();
    if (!address.is_valid())
    {
        frontend_charon_start();

        AutoPtr<FmWork::Task> task(new StartConnectionTask(m_provider, *this));
        m_submitter.add(task);
    }
    else
    {
        LOG_INFO(field_frontends, "Start the connection using new method");
        AutoPtr<FmWork::Task> task(new MakeConnectionTask(m_connection, address));
        m_submitter.add(task);
    }
}

int VpnFrontendService::sock_bypass_wrap(void* self, int fd, int family)
{
    auto p = static_cast<VpnFrontendService*>(self);
    return p->socket_bypass(fd, family) ? TRUE : FALSE;
}

END_NAMESPACE(Frontend)