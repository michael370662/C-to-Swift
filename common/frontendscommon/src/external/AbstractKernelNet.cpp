#include "external/AbstractKernelNet.hpp"
#include "external/VpnFrontendService.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

AbstractKernelNet::AbstractKernelNet(VpnFrontendService& srv) :
m_service(srv)
{
    m_socket = frontend_udpsock_open();
    if (m_socket >= 0)
    {
        srv.socket_bypass(m_socket, 2);
    }
}

AbstractKernelNet::~AbstractKernelNet()
{
    frontend_udpsock_close(m_socket);
}

void AbstractKernelNet::add_address(const Net::SockAddress &src)
{
    auto scope = LockScope::normal(m_lock);
    m_address.append(src);
}

bool AbstractKernelNet::remove_address(const Net::SockAddress &src)
{
    auto scope = LockScope::normal(m_lock);
    bool found =false;
    for(size_t i=m_address.count(); i>0; i--)
    {
        if (m_address[i-1] != src) continue;
        m_address.remove_and_swap_last(i-1);
        found = true;
    }
    return found;
}

static void set_addr_value(void *ret, void* sockaddr)
{
    auto p = static_cast<Net::SockAddress*>(ret);
    p->set_sockaddr(sockaddr);
}


Net::SockAddress AbstractKernelNet::get_host
    (const Net::SockAddress& src, const Net::SockAddress &dst)
{
    if (m_socket < 0) return Net::SockAddress();
    Net::SockAddress ret_addr;

    int ret = frontend_udpsock_get_host(m_socket, &ret_addr, this, 
                dst.non_const_ptr(), on_flush_sockets, set_addr_value);
    return ret == FALSE ? Net::SockAddress() : ret_addr;
}

END_NAMESPACE(Frontend)