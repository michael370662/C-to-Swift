#ifndef __PonyTech_Frontend_SocketSystem_hpp__
#define __PonyTech_Frontend_SocketSystem_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class AbstractMessageForwarder;

class SocketSystem : public VPN::Common::SocketEngine,
                    public VPN::Common::SocketActivity
{
    struct SendData
    {
        Net::SockAddress self;
        Net::SockAddress peer;
        Net::AbstractNetBuffer* buffer;
    };
    
    class UserSpace : public VPN::Common::SocketUserSpace
    {
        RawQueue<SendData> m_send_queue;
    public:
        UserSpace(){}
        ~UserSpace();

        bool get(SendData &data);
        void send(SendData& data);
    };
private:
    constexpr static int k_timeout_second = 3;
    AbstractMessageForwarder&       m_message_forwarder;  
    Net::AbstractNetBufferProvider& m_provider;

    RefPtr<UserSpace>               m_space; 
    Net::Port                       m_port;
public:
    SocketSystem(Net::AbstractNetBufferProvider &provider, AbstractMessageForwarder& forwarder);
    ~SocketSystem();

    Net::Port port() const { return m_port; }
    bool is_supported() const { return m_space.not_null() && !m_space->destroy_request(); }
    bool init_socket();

    void send(Net::AbstractNetBufferContainer& container, const Net::SockAddress& src, const Net::SockAddress& dst);

    Net::AbstractNetBufferProvider& buffer_provider() { return m_provider; }

private:
    virtual size_t on_socket_usr_space() const override { return sizeof(UserSpace);}
    virtual VPN::Common::SocketUserSpace* on_socket_space_construct(void *addr, Net::UdpSocket& socket) override;

    virtual void on_socket_close(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space) override;
    virtual bool on_socket_send(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space) override;
    virtual void on_socket_recv(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space,
                                const Net::SockAddress &self, const Net::SockAddress &peer, 
                                int ifcard, Net::AbstractNetBufferContainer& container) override;
private:
    FND_DISABLE_COPY(SocketSystem);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_SocketSystem_hpp__