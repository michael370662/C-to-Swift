#include "external/KernelNetProvider.hpp"
#include "external/AbstractKernelNet.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

BaseKernelNetProvider::BaseKernelNetProvider(VpnFrontendService &srv) :
m_service(srv)
{
}

BaseKernelNetProvider::~BaseKernelNetProvider()
{
}

void BaseKernelNetProvider::register_loader()
{
    int ret = frontend_kernel_net_provide(this,
        creator, destroyer, add_ip, del_ip, all_ips, get_host);
    if (ret == FALSE)
    {
        LOG_ERROR(field_frontends, "Cannot load the kernel net into plugin");
    }
}

void BaseKernelNetProvider::unregister_loader()
{
    frontend_kernel_net_remove();
}

void* BaseKernelNetProvider::creator(void* provider)
{
    auto p = static_cast<BaseKernelNetProvider*>(provider);
    return p->create_net(p->m_service);
}

void BaseKernelNetProvider::destroyer(void *self)
{
    delete static_cast<AbstractKernelNet*>(self);
}

void BaseKernelNetProvider::add_ip(void *self, void* sockaddr)
{
    auto p = static_cast<AbstractKernelNet*>(self);
    Net::SockAddress src;
    src.set_sockaddr(sockaddr);
    LOG_INFO(field_frontends, "adding address {0} in kernel net", src);
    p->add_address(src);
}

void  BaseKernelNetProvider::del_ip(void* self, void* sockaddr)
{
    auto p = static_cast<AbstractKernelNet*>(self);
    Net::SockAddress src;
    src.set_sockaddr(sockaddr);
    LOG_INFO(field_frontends, "removing address {0} in kernel net", src);
    p->remove_address(src);
}
    
void** BaseKernelNetProvider::all_ips(void *self, size_t *cnt, 
    void* (*allocate)(size_t), void* (*host_create)(void*sockaddr))
{
    auto p = static_cast<AbstractKernelNet*>(self);
    auto scope = LockScope::normal(p->m_lock);

    *cnt = p->m_address.count();
    LOG_INFO(field_frontends, "retrieve address in kernel net, cnt = {0}", *cnt);

    void **addr = static_cast<void**>(allocate(sizeof(void*) * (*cnt)));
    for(size_t i=0; i<*cnt; i++)
    {
        addr[i] = host_create(p->m_address[i].non_const_ptr());
    }
    return addr;    
}

void* BaseKernelNetProvider::get_host(void* self, void *src_sockaddr, void* dst_sockaddr, 
                        void* (*host_create)(void*sockaddr))
{
    Net::SockAddress src, dst;
    if (src_sockaddr != nullptr) src.set_sockaddr(src_sockaddr);
    if (dst_sockaddr != nullptr) dst.set_sockaddr(dst_sockaddr);
    
    auto p = static_cast<AbstractKernelNet*>(self);
    Net::SockAddress host = p->get_host(src,dst);
    if (!host.is_valid()) return nullptr;

    return host_create(host.non_const_ptr());
}



END_NAMESPACE(Frontend)