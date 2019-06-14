#include "service/SystemConnector.hpp"
#include "service/ConnectionSystem.hpp"


BEGIN_NAMESPACE(Frontend)
BEGIN_NAMESPACE()

struct SendData
{
    Net::SockAddress peer;
    Net::AbstractNetBuffer* buffer;
};

class UserSpace : public VPN::Common::SocketUserSpace
{
    RawQueue<SendData> m_send_queue;
public:
    Net::SockAddress   address;
    int                ifcard;
public:
    UserSpace(){ ifcard = -1; }
    virtual ~UserSpace();

    bool get(SendData &data);
    void send(SendData& data);
};

UserSpace::~UserSpace() 
{
    auto scope = LockScope::normal(m_lock);
    while(m_send_queue.count())
    {
        auto data = m_send_queue.dequeue();
        data.buffer->recycle();
    }
}

void UserSpace::send(SendData& data)
{
    {   auto scope = LockScope::normal(m_lock);
        m_send_queue.enqueue(data);
    }
    trigger_worker();
}

bool UserSpace::get(SendData &data)
{
    auto scope = LockScope::normal(m_lock);
    if (m_send_queue.count() == 0) return false;
    data = m_send_queue.dequeue();
    return true;
}

END_NAMESPACE()

SystemConnector::SystemConnector(VPN::Common::SocketEngine& engine, ConnectionSystem& connection) :
m_engine(engine),
m_connection(connection)
{    
}

SystemConnector::~SystemConnector()
{    
}

void SystemConnector::send(Net::AbstractNetBufferContainer& container, const Net::SockAddress& peer)
{
    if (container.is_null() || m_socket_space.is_null())
    {
        LOG_ERROR(field_frontends, "Nil buffer or Nil socket space and it cannot send");
        return;
    }

    VPN::Common::SocketIterator itor(m_engine);    // gain read lock for socket engine
    FND_UNUSED(itor);

    SendData data;
    data.peer = peer;
    data.buffer = container.deref();
    m_socket_space.downcast_data<UserSpace>().send(data);
}

size_t SystemConnector::on_socket_usr_space() const 
{
    return sizeof(UserSpace);
}

VPN::Common::SocketUserSpace* SystemConnector::on_socket_space_construct(void *addr, Net::UdpSocket& socket)
{
    new (addr) UserSpace();    
    auto p = static_cast<UserSpace*>(addr);
    m_socket_space.set(p);
    return p;
}

void SystemConnector::on_socket_close(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space)
{    
    m_socket_space.set_null();
}

bool SystemConnector::on_socket_send(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space)
{
    auto &concrete = static_cast<UserSpace&>(usr_space);
    SendData data;
    bool has = concrete.get(data);      if (!has) return false;
    
    Net::AbstractNetBufferContainer container (data.buffer);
    socket.send(concrete.address, concrete.ifcard, data.peer, container->non_const_ptr(), container->count());
    return true;
}


void SystemConnector::on_socket_recv(Net::UdpSocket& socket, VPN::Common::SocketUserSpace& usr_space,
                                const Net::SockAddress &self, const Net::SockAddress &peer, 
                                int ifcard, Net::AbstractNetBufferContainer& container)
{
    // TODO need to assign self ?? how to do ip roaming (currently not assign back to user space)
    m_connection.recv_message(container);
}


END_NAMESPACE(Frontend)
