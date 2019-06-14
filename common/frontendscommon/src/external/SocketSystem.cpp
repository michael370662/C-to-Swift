#include "external/SocketSystem.hpp"
#include "external/SocketMessageForwarder.hpp"

BEGIN_NAMESPACE(Frontend)


SocketSystem::UserSpace::~UserSpace()
{
    auto scope = LockScope::normal(m_lock);
    while(m_send_queue.count())
    {
        auto data = m_send_queue.dequeue();
        data.buffer->recycle();
    }
}

void SocketSystem::UserSpace::send(SendData& data)
{
    {   auto scope = LockScope::normal(m_lock);
        m_send_queue.enqueue(data);
    }
    trigger_worker();
}

bool SocketSystem::UserSpace::get(SendData &data)
{
    auto scope = LockScope::normal(m_lock);
    if (m_send_queue.count() == 0) return false;
    data = m_send_queue.dequeue();
    return true;
}



SocketSystem::SocketSystem(Net::AbstractNetBufferProvider &provider, AbstractMessageForwarder& forwarder):
SocketEngine(*this, provider),
m_message_forwarder(forwarder),
m_provider(provider)
{    
    set_interval(Timespan::from_second(k_timeout_second));
}

SocketSystem::~SocketSystem()
{
}

bool SocketSystem::init_socket()
{
    try
    {
        AutoPtr<Net::UdpSocket> socket (Net::UdpSocket::create_instance(Net::AddressDefine::NetFamily::nf_ipv4));
        if (socket.is_null()) return false;

        Net::IPv4Address address;
        socket->bind(address);    
        Net::SockAddress addr;
        socket->get_self_addr(addr);
        m_port = addr.port();

        VPN::Common::SocketAdderScope scope(*this);
        m_space.set(static_cast<UserSpace*>(scope.add(socket)));
    }
    catch(const Exception::Exception& ex)
    {
        LOG_ERROR(field_frontends, "Failed to initiate socket with port {0}", m_port);
        return false;
    }
    return m_space.not_null();
}

void SocketSystem::send(Net::AbstractNetBufferContainer& container,const Net::SockAddress& self, const Net::SockAddress& peer)
{
    if (container.is_null())
    {
        LOG_ERROR(field_frontends, "Nil buffer to be send");
        return;
    }

    VPN::Common::SocketIterator itor(*this);    // gain read lock for engine

    LOG_TRACE(field_frontends, "send buffer with size {0}, {1} -> {2} ", container->count(), self, peer);
    Net::Port port = self.port();
    if ((port && m_port != port) || m_space.is_null())
    {
        LOG_ERROR(field_frontends, "Fail to send message {0}, {1}", self, m_port);
        return;
    }

    SendData data;
    data.self = self;
    data.peer = peer;
    data.buffer = container.deref();
    m_space->send(data);
}

VPN::Common::SocketUserSpace* SocketSystem::on_socket_space_construct(void *addr, Net::UdpSocket& socket) 
{
    new (addr) UserSpace();
    return static_cast<UserSpace*>(addr);
}

void SocketSystem::on_socket_close(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space)
{
    m_space.set_null();
}

bool SocketSystem::on_socket_send(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space) 
{
    auto &concrete = static_cast<UserSpace&>(usr_space);
    SendData data;
    bool has = concrete.get(data);      if (!has) return false;
    
    Net::AbstractNetBufferContainer container (data.buffer);
    socket.send(data.self, -1, data.peer, container->non_const_ptr(), container->count());
    return true;
}

void SocketSystem::on_socket_recv(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space, 
                                const Net::SockAddress &self, const Net::SockAddress &peer, 
                                int ifcard, Net::AbstractNetBufferContainer& container) 
{
    Net::SockAddress self_data(self);
    self_data.change_port(m_port);
    LOG_TRACE(field_frontends, "Receive buffer size {0}, {1} -> {2} ", container->count(), peer, self_data);

    return m_message_forwarder.forward(container, self, peer);     
}                                

END_NAMESPACE(Frontend)
