#ifndef __PonyTech_Frontend_FrontendsIpsecPacketForwarder_hpp__
#define __PonyTech_Frontend_FrontendsIpsecPacketForwarder_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class TunDeviceInterface;
class SocketSystem;

class FrontendsIpsecPacketForwarder : public CrossWrap::Common::ExternalPacketForwarder
{
    TunDeviceInterface&     m_tun_interface;
    SocketSystem&           m_socket_system;
public:
    FrontendsIpsecPacketForwarder(TunDeviceInterface& tun, SocketSystem& socket);
    virtual ~FrontendsIpsecPacketForwarder(){}
    
    virtual void send_inbound(Net::AbstractNetBufferContainer& buffer) override;    
    virtual void send_outbound(Net::AbstractNetBufferContainer& buffer, const Net::SockAddress &src, const Net::SockAddress &dst) override;
private:
    FND_DISABLE_COPY(FrontendsIpsecPacketForwarder);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_FrontendsIpsecPacketForwarder_hpp__