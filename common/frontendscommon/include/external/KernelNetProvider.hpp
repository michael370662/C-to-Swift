#ifndef __PonyTech_Frontend_KernelNetProvider_hpp__
#define __PonyTech_Frontend_KernelNetProvider_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class AbstractKernelNet;
class VpnFrontendService;

class BaseKernelNetProvider
{
    VpnFrontendService&     m_service;
public:
    BaseKernelNetProvider(VpnFrontendService &srv);
    virtual ~BaseKernelNetProvider();


    virtual AbstractKernelNet* create_net(VpnFrontendService &srv) = 0;

    void register_loader();
    void unregister_loader();
private:
    static void* creator(void* provider);
    static void  destroyer(void *self);
    static void  add_ip(void *self, void* sockaddr);
    static void  del_ip(void *self, void* sockaddr);
    static void** all_ips(void *self, size_t *cnt, void* (*allocate)(size_t), void* (*host_create)(void*sockaddr));
    static void* get_host(void* self, void *src_sockaddr, void* dst_sockaddr, void* (*host_create)(void*sockaddr));
    FND_DISABLE_COPY(BaseKernelNetProvider);
};


template<class T>
class KernelNetProvider : public BaseKernelNetProvider
{
public:
    KernelNetProvider(VpnFrontendService& src):BaseKernelNetProvider(src){}
    ~KernelNetProvider(){}

    virtual AbstractKernelNet* create_net(VpnFrontendService &srv) override { return new T(srv);}
private:
    FND_DISABLE_COPY(KernelNetProvider<T>);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_KernelNetProvider_hpp__