#include <service/ConnectionSystem.hpp>
#include <service/SystemConnector.hpp>
#include <core/AbstractReportStatus.hpp>
#include <service/VpnSettingsProvider.hpp>

BEGIN_NAMESPACE(Frontend)

ConnectionSystem::ConnectionSystem(Net::AbstractNetBufferProvider& buffer_provider, FmWork::TaskSubmitter& submitter,
                                   VPN::Common::SocketEngine& engine, SystemConnector& connector, 
                                   AbstractReportStatus& report_status, VpnSettingsProvider &setting_provider) :
VPN::Client::ClientCore(buffer_provider, submitter),
m_engine(engine),
m_connector(connector),
m_report_status(report_status),
m_setting_provider(setting_provider)
{
}

ConnectionSystem::~ConnectionSystem()
{

}

void ConnectionSystem::create_socket(Net::AddressDefine::NetFamily family)
{
    try
    {
        AutoPtr<Net::UdpSocket> socket (Net::UdpSocket::create_instance(family));
        if (socket.is_null()) throw Exception::GeneralFailure();

        Net::IPv4Address address;
        socket->bind(address);    

        VPN::Common::SocketAdderScope adder(m_engine);
        adder.add(socket);   
    }
    catch(const Exception::Exception & ex)
    {
        m_report_status.report_status(AbstractReportStatus::ErrorStatus::es_general_error,
                                      AbstractReportStatus::ErrorCode::ec_client_socket_error);
        throw;                                      
    }
}

void ConnectionSystem::on_error_code(int err_code) 
{
    if (err_code >= ErrCode::err_server_beyond)
    {
        // TODO
        return;
    }

    int status = AbstractReportStatus::ErrorStatus::es_general_error;
    int code = AbstractReportStatus::ErrorCode::ec_none;

    switch(err_code)
    {
        case ErrCode::err_client_engine:
            code = AbstractReportStatus::ErrorCode::ec_client_socket_error;
            break;
        case ErrCode::err_server_not_reached:
            status = AbstractReportStatus::ErrorStatus::es_server_not_reached;
            break;
        case ErrCode::err_connection_broken:
            status = AbstractReportStatus::ErrorStatus::es_connection_broken;
            break;
        case ErrCode::err_version_not_match:
            status = AbstractReportStatus::ErrorStatus::es_version_not_match;
            break;
        default: break;
    }
    m_report_status.report_status(status, code);
}

void ConnectionSystem::on_send_message(Net::AbstractNetBufferContainer& container, const Net::SockAddress& addr)
{
    m_connector.send(container, addr);
}

void ConnectionSystem::on_get_auth(Common::AuthenticationProtocol::Data& data) 
{
    auto setting = m_setting_provider.settings();
    if (setting == nullptr) return;
    
    data.username = setting->username();
    String password = setting->password();

    size_t len = password.length();
    data.password_hash.resize(len);
    MemoryAction::copy(data.password_hash.ptr(), password.ptr(), len);
}

END_NAMESPACE(Frontend)