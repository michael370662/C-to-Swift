#include "external/FrontendsIpsecPacketForwarder.hpp"
#include "external/SocketSystem.hpp"
#include "service/TunDeviceInterface.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

FrontendsIpsecPacketForwarder::FrontendsIpsecPacketForwarder
        (TunDeviceInterface& tun, SocketSystem& socket):
m_tun_interface(tun),
m_socket_system(socket)
{
}


void FrontendsIpsecPacketForwarder::send_inbound(Net::AbstractNetBufferContainer& buffer)
{
    m_tun_interface.send_packet(*buffer);
    buffer.release();
}    

void FrontendsIpsecPacketForwarder::send_outbound(Net::AbstractNetBufferContainer& buffer, const Net::SockAddress &src, const Net::SockAddress &dst)
{
    m_socket_system.send(buffer, src, dst);
}


END_NAMESPACE(Frontend)