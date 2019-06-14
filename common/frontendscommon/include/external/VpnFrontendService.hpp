#ifndef __PonyTech_Frontend_VpnFrontendService_hpp__
#define __PonyTech_Frontend_VpnFrontendService_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class VpnSettingsProvider;
class ConnectionSystem;
class SocketSystem;
class ConnectHeartbeatManager;
class SocketMessageForwarder;

class VpnFrontendService
{
    bool    m_library_inited;
    const VpnSettingsProvider&          m_provider;
    FmWork::TaskSubmitter&              m_submitter;
    ConnectionSystem&                   m_connection;

    AutoPtr<ConnectHeartbeatManager>    m_heartbeat;
public:
    VpnFrontendService(const VpnSettingsProvider& provider, ConnectionSystem& connection,
                       SocketSystem& socket_system, SocketMessageForwarder& message_forwarder,
                       FmWork::TaskSubmitter& schedular);
    ~VpnFrontendService();

    bool startup();
    void register_loader();
    void teardown();

    void reload_settings();
    bool load_plugins(); 
    void start_connect();

    ConnectHeartbeatManager* heartbeat_mgr() { return m_heartbeat.non_const_ptr(); }
    FmWork::TaskSubmitter&   submitter() { return m_submitter; }
    const VpnSettingsProvider& setting_provider() { return m_provider; }

    virtual void change_status(int status) = 0;
    virtual bool socket_bypass(int fd, int family) = 0; 
    virtual void on_sudden_disconnect() {}
private:
    Net::SockAddress is_new_request() const;
    static int sock_bypass_wrap(void *self, int fd, int family);
    FND_DISABLE_COPY(VpnFrontendService);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_VpnFrontendService_hpp__
