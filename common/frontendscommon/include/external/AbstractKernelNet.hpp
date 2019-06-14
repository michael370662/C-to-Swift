#ifndef __PonyTech_Frontend_AbstractKernelNet_hpp__
#define __PonyTech_Frontend_AbstractKernelNet_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class VpnFrontendService;

class AbstractKernelNet
{
    VpnFrontendService& m_service;
    Mutex   m_lock;
    Array<Net::SockAddress> m_address;
    int m_socket;
public:
    AbstractKernelNet(VpnFrontendService& srv);
    virtual ~AbstractKernelNet();
    
    VpnFrontendService& service() { return m_service; }
    
    virtual Net::SockAddress get_host(const Net::SockAddress& src, const Net::SockAddress &dst);
protected:
    void add_address(const Net::SockAddress &src);
    bool remove_address(const Net::SockAddress &src);
private:
    static void on_flush_sockets(void *owner) {}
    friend class BaseKernelNetProvider;
    FND_DISABLE_COPY(AbstractKernelNet);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_AbstractKernelNet_hpp__