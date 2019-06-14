#ifndef __PonyTech_Frontend_ConnectionSystem_hpp__
#define __PonyTech_Frontend_ConnectionSystem_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class AbstractReportStatus;
class SystemConnector;
class VpnSettingsProvider;

class ConnectionSystem : public VPN::Client::ClientCore
{
    VPN::Common::SocketEngine&  m_engine;
    SystemConnector&            m_connector;
    AbstractReportStatus&       m_report_status;
    VpnSettingsProvider&        m_setting_provider;
public:
    ConnectionSystem(Net::AbstractNetBufferProvider& buffer_provider, FmWork::TaskSubmitter& submitter, 
                     VPN::Common::SocketEngine& engine, SystemConnector& connector, 
                    AbstractReportStatus& report_status, VpnSettingsProvider &setting_provider);
    virtual ~ConnectionSystem();


    void create_socket(Net::AddressDefine::NetFamily family);


private:
    virtual void on_error_code(int err_code) override;
    virtual void on_get_auth(Common::AuthenticationProtocol::Data& data) override;
    virtual void on_send_message(Net::AbstractNetBufferContainer& container, const Net::SockAddress& addr) override;
    FND_DISABLE_COPY(ConnectionSystem);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_ConnectionSystem_hpp__