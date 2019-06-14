#include "service/TunDeviceInterface.hpp"
#include "external/VpnFrontendService.hpp"
#include "../external/ConnectHeartbeatManager.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

TunDeviceInterface::TunDeviceInterface(VpnFrontendService& srv):
m_service(srv)
{
    m_dns_importor = frontend_dns_importor_create(this, add_dns_wrap);
}

TunDeviceInterface::~TunDeviceInterface()
{
    if (m_dns_importor != nullptr) frontend_dns_importor_destroy(m_dns_importor);
}

void TunDeviceInterface::register_loader()
{
    if (m_dns_importor != nullptr)
    {
        int ret = frontend_dns_importor_setup(m_dns_importor);
        if (ret == FALSE)
        {
            LOG_ERROR(field_frontends, "Cannot load the dns importor into plugin");
        }
    }

    int ret = frontend_tun_device_provide(this, 
                                        &TunDeviceInterface::update_status_wrap,
                                        &TunDeviceInterface::establish_wrap,
                                        &TunDeviceInterface::post_established_wrap,
                                        &TunDeviceInterface::is_established_wrap,
                                        &TunDeviceInterface::close_wrap,
                                        &TunDeviceInterface::add_address_wrap,
                                        &TunDeviceInterface::add_route_wrap);

    if (ret == FALSE)
    {
        LOG_ERROR(field_frontends, "Cannot load the service into plugin");
    }
}

void TunDeviceInterface::handle_received_packet(Net::AbstractNetBufferContainer& buffer)
{
    CrossWrap::Common::ipsec_add_outbound(buffer);
}

void TunDeviceInterface::update_status_wrap(void *data, int status)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    self->m_service.change_status(status);
}

int TunDeviceInterface::add_dns_wrap(void *data, void* sockaddr)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    Net::SockAddress src;
    src.set_sockaddr(sockaddr);
    return self->add_dns(src) ? TRUE: FALSE;
}

int TunDeviceInterface::add_address_wrap(void *data, void* sockaddr)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    Net::SockAddress src;
    src.set_sockaddr(sockaddr);
    return self->add_address(src) ? TRUE: FALSE;  
}

int TunDeviceInterface::add_route_wrap(void *data, void *sockaddr, int prefix)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    Net::SockAddress src;
    src.set_sockaddr(sockaddr);
    return self->add_route(src, prefix) ? TRUE: FALSE;   
}

int TunDeviceInterface::establish_wrap(void *data, int with_dns, int* already_registered)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    if (!with_dns)
    {
        return self->turn_on_again() ? TRUE : FALSE;
    }

    bool flag = false;
    bool success = self->turn_on(flag);
    if (!success) return false;

    if (already_registered != nullptr) *already_registered = flag ? TRUE : FALSE;
    return true;
}

void TunDeviceInterface::post_established_wrap(void *data, int already_registered)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    self->post_turn_on(already_registered != FALSE);  
    self->m_service.heartbeat_mgr()->startup(self->m_service.submitter());
}

int TunDeviceInterface::is_established_wrap(void *data)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    return self->is_turn_on() ? TRUE : FALSE;
}

void TunDeviceInterface::close_wrap(void *data)
{
    auto self = static_cast<TunDeviceInterface*>(data);
    self->turn_off();
    self->m_service.heartbeat_mgr()->shutdown();
}


END_NAMESPACE(Frontend)
