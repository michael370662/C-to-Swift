#ifndef __PonyTech_Frontend_TunDeviceInterface_hpp__
#define __PonyTech_Frontend_TunDeviceInterface_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class VpnFrontendService;

class TunDeviceInterface
{
    void*   m_dns_importor;
    VpnFrontendService& m_service;
public:
    TunDeviceInterface(VpnFrontendService &service);
    ~TunDeviceInterface();

    void register_loader();

    virtual bool add_address(const Net::SockAddress& addr) = 0;
    virtual bool add_route(const Net::SockAddress& addr, int prefix) = 0;
    virtual bool add_dns(const Net::SockAddress &addr) = 0;
    virtual void send_packet(const PxConstArray<byte_t>& content) = 0;

    virtual bool turn_on(bool &already_opened) = 0;
    virtual void post_turn_on(bool already_opened) = 0;
    virtual bool is_turn_on() const = 0;
    virtual bool turn_on_again() = 0;
    virtual void turn_off() = 0;

protected:
    void handle_received_packet(Net::AbstractNetBufferContainer& content);
private:
    static void update_status_wrap(void *data, int status);
    static int establish_wrap(void *data, int with_dns, int* already_registered);
    static void post_established_wrap(void *data, int already_registered);
    static int is_established_wrap(void *data);
    static void close_wrap(void *data);
    static int add_dns_wrap(void *data, void* sockaddr);
    static int add_address_wrap(void *data, void* sockaddr);
    static int add_route_wrap(void *data, void *sockaddr, int prefix);
    FND_DISABLE_COPY(TunDeviceInterface);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_TunDeviceInterface_hpp__