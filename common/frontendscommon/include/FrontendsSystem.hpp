#ifndef __PonyTech_Frontend_FrontendsSystem_hpp__
#define __PonyTech_Frontend_FrontendsSystem_hpp__

#include "external/FrontendsIpsecPacketForwarder.hpp"
#include "external/SocketSystem.hpp"
#include "external/SocketMessageForwarder.hpp"
#include "external/EAPMd5Provider.hpp"

#include "service/SystemConnector.hpp"
#include "service/ConnectionSystem.hpp"

BEGIN_NAMESPACE(Frontend)

class TunDeviceInterface;
class AbstractReportStatus;
class VpnSettingsProvider;

class FrontendsSystem
{
public:
    SingletonSystem::Scope  singleton_scope;
    FmWork::TaskSchedular   task_schedular;
    VPN::Common::GeneralNetBufferProvider   buffer_provider;
private:
    VPN::Common::SocketEngine  m_socket_engine;
    SystemConnector            m_system_connector;  
    EAPMd5Provider             m_eap_md5_provider;
protected:
    ConnectionSystem           m_connection_system;  
public:
    // strongswan part
    FrontendsIpsecPacketForwarder   packet_forwarder;
    SocketSystem                    socket_system;
    SocketMessageForwarder          socket_message_forwarder;

public:
    FrontendsSystem(AbstractReportStatus& status,  TunDeviceInterface& tun_interface, VpnSettingsProvider& settting_provider);
    ~FrontendsSystem();
 
    void startup();
    void register_loader();
    void teardown();

private:
    FND_DISABLE_COPY(FrontendsSystem);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_FrontendsSystem_hpp__