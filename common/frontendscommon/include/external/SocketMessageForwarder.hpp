#ifndef __PonyTech_Frontend_SocketMessageForwarder_hpp__
#define __PonyTech_Frontend_SocketMessageForwarder_hpp__

#include "AbstractMessageForwarder.hpp"

BEGIN_NAMESPACE(Frontend)

class SocketSystem;
class HeartbeatSession;

class SocketMessageForwarder : public AbstractMessageForwarder
{
    typedef void (*IkeProcess)(bool need_skip, void *src_sockaddr, void *dst_sockaddr,
                             void *buffer, size_t len);

    Net::AbstractNetBufferProvider &m_buffer_provider;
    SocketSystem&   m_system;
    IkeProcess      m_process;
    Net::Port       m_port;
    static SocketMessageForwarder* s_instance;
    SharedPtr<HeartbeatSession>     m_heartbeat;
public:
    SocketMessageForwarder(Net::AbstractNetBufferProvider &provider, SocketSystem &system);
    virtual ~SocketMessageForwarder();

    void set_heartbeat(const SharedPtr<HeartbeatSession>& heartbeat);
    virtual void forward(Net::AbstractNetBufferContainer& container, const Net::SockAddress& self, const Net::SockAddress& peer) override;

private:
    static void*       init_wrap(void (*handle)(bool need_skip, void *src_sockaddr, void *dst_sockaddr,
                             void *buffer, size_t len));
    static void        close_wrap(void* data);
    static uint16_t    get_port_wrap(void* data);
    static bool        get_supported_wrap(void* data);
    static void        send_wrap(void* data, void  *src_sockaddr, void *dst_sockaddr,
                          void *buffer, size_t len);
    FND_DISABLE_COPY(SocketMessageForwarder);
};

END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_AbstractMessageForwarder_hpp__