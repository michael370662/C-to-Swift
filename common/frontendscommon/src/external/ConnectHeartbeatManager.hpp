#ifndef __PonyTech_Frontend_ConnectHeartbeatManager_hpp__
#define __PonyTech_Frontend_ConnectHeartbeatManager_hpp__

#include <PonyTech-Frontend_DEP.hpp>


BEGIN_NAMESPACE(Frontend)

class VpnFrontendService;
class HeartbeatSession;
class SocketSystem;
class SocketMessageForwarder;

class ConnectHeartbeatManager
{
    VpnFrontendService&           m_service;
    SocketSystem&                 m_socket_system;
    SocketMessageForwarder&       m_message_forwarder;
    SharedPtr<HeartbeatSession>   m_data;
public:
    ConnectHeartbeatManager(SocketSystem& socket_system, SocketMessageForwarder& forwarder, 
                            VpnFrontendService& service);
    ~ConnectHeartbeatManager();

    void startup(FmWork::TaskSubmitter& submitter);
    void shutdown();

private:
    FND_DISABLE_COPY(ConnectHeartbeatManager);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_ConnectHeartbeatManager_hpp__
