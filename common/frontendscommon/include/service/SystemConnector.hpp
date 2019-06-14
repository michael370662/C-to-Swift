#ifndef __PonyTech_Frontend_SystemConnector_hpp__
#define __PonyTech_Frontend_SystemConnector_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class ConnectionSystem;

class SystemConnector : public VPN::Common::SocketActivity
{
    RefPtr<VPN::Common::SocketUserSpace>    m_socket_space;
    VPN::Common::SocketEngine&              m_engine;
    ConnectionSystem&                       m_connection;
public:
    SystemConnector(VPN::Common::SocketEngine& engine, ConnectionSystem& connection);
    virtual ~SystemConnector();

    void send(Net::AbstractNetBufferContainer& container, const Net::SockAddress& peer);

private:
    virtual size_t on_socket_usr_space() const override;
    virtual VPN::Common::SocketUserSpace* on_socket_space_construct(void *addr, Net::UdpSocket& socket) override;
    virtual void on_socket_close(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space) override;
    virtual bool on_socket_send(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space) override;
    virtual void on_socket_recv(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space,
                                const Net::SockAddress &self, const Net::SockAddress &peer, 
                                int ifcard, Net::AbstractNetBufferContainer& container) override;
private:
    FND_DISABLE_COPY(SystemConnector);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_SystemConnector_hpp__