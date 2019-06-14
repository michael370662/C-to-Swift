#include "external/SocketMessageForwarder.hpp"
#include "external/SocketSystem.hpp"
#include "HeartbeatRegularTask.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

SocketMessageForwarder* SocketMessageForwarder::s_instance = nullptr;

SocketMessageForwarder::SocketMessageForwarder(Net::AbstractNetBufferProvider &provider, SocketSystem &system) :
m_buffer_provider(provider),
m_system(system)
{
    s_instance = this;
    m_port = frontend_default_gateway_port();
    frontend_socket_provide(
        &SocketMessageForwarder::init_wrap,
        &SocketMessageForwarder::close_wrap,
        &SocketMessageForwarder::get_port_wrap,
        &SocketMessageForwarder::get_supported_wrap,
        &SocketMessageForwarder::send_wrap
    );
}

SocketMessageForwarder::~SocketMessageForwarder()
{
    frontend_socket_remove();
}

void SocketMessageForwarder::set_heartbeat(const SharedPtr<HeartbeatSession>& heartbeat)
{
    m_heartbeat = heartbeat;
}

void SocketMessageForwarder::forward(Net::AbstractNetBufferContainer& container, const Net::SockAddress& self, const Net::SockAddress& peer) 
{
    if (m_heartbeat.not_null() && peer.port() == frontend_heartbeat_port())
    {
        m_heartbeat->received(container);
        return;
    }
    
    if (container->count() == 1 && container->ptr()[0] == 0xff) return;
    if (container->count() < 4)
    {
        LOG_WARN(field_frontends, "Receved packet is too short");
        return;
    }

    bool need_skip = false;
    uint32_t value = 0;
    if (self.port() != m_port && peer.port() != m_port)
    {
        need_skip = true;
        if (*reinterpret_cast<uint32_t*>(container->ptr()) != value)
        {
            CrossWrap::Common::ipsec_add_inbound(container, self);
            return;
        }
    }

    if (m_process)
    {
        m_process(need_skip, peer.non_const_ptr(), self.non_const_ptr(), container->ptr(), container->count());
    }
}

void* SocketMessageForwarder::init_wrap(void (*handle)(bool need_skip, 
            void *src_sockaddr, void *dst_sockaddr, void *buffer, size_t len))
{
    if (s_instance == nullptr) return nullptr;
    bool success = s_instance->m_system.init_socket();     if (!success) return nullptr;   
    s_instance->m_process = handle;
    s_instance->m_system.init(1);
    return s_instance;
}

void SocketMessageForwarder::close_wrap(void* data)
{
    auto self = static_cast<SocketMessageForwarder*>(data);
    if (self != nullptr) self->m_system.stop();
}
    
uint16_t SocketMessageForwarder::get_port_wrap(void* data)
{
    auto self = static_cast<SocketMessageForwarder*>(data);
    return self == nullptr ? 0 : self->m_system.port();
}

bool SocketMessageForwarder::get_supported_wrap(void* data)
{
    auto self = static_cast<SocketMessageForwarder*>(data);
    return self == nullptr ? false : self->m_system.is_supported();
}

void SocketMessageForwarder::send_wrap(void* data, void  *src_sockaddr, void *dst_sockaddr,
                                        void *buffer, size_t len)
{
    auto self = static_cast<SocketMessageForwarder*>(data);
    if (self == nullptr) return;

    Net::SockAddress src, dst;
    src.set_sockaddr(src_sockaddr);
    dst.set_sockaddr(dst_sockaddr);
    Net::AbstractNetBufferContainer container( self->m_buffer_provider.create(len) );
    if (container.is_null()) return;

    container->at_least(len, len);
    container->resize(len);
    MemoryAction::copy(container->non_const_ptr(), buffer, len);    
    self->m_system.send(container, src, dst);
}

END_NAMESPACE(Frontend)
