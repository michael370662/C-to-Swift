#include "HeartbeatRegularTask.hpp"
#include "external/SocketSystem.hpp"
#include "external/VpnFrontendService.hpp"
#include "service/VpnSettingsProvider.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)


void HeartbeatSession::received(Net::AbstractNetBufferContainer& container)
{
    if (container.is_null() || container->count() < k_buffer_size) return;
    const byte_t *ptr = container->ptr();
    if (*reinterpret_cast<const uint32_t*>(ptr) != 0xffffffff) return;

    uint32_t value = Endian::n2h(*reinterpret_cast<const uint32_t*>(ptr+4));

    LOG_INFO(field_frontends, "Received the heartbeat with {0}, recv_id = {1}", value, recv_id);
    if (WrapAroundNumber32(recv_id) >= WrapAroundNumber32(value)) return;
    recv_id = value;
}

const Timespan HeartbeatRegularTask::m_long_wait = Timespan::from_second(30);
const Timespan HeartbeatRegularTask::m_short_wait = Timespan::from_second(5);

HeartbeatRegularTask::HeartbeatRegularTask(SocketSystem& socket_system, 
                VpnFrontendService& service, const SharedPtr<HeartbeatSession>& data):
m_socket_system(socket_system),                
m_service(service)
{
    m_data = data;
}

HeartbeatRegularTask::~HeartbeatRegularTask()
{
}

FmWork::Task::Request HeartbeatRegularTask::execute(FmWork::TaskSubmitter &submitter)
{
    if (m_data->stopped != 0) return Request::r_remove;

    LOG_INFO(field_frontends, "Getting the heartbeat testing...");
 
    m_alive = m_data->next_id == m_data->recv_id;
    if (!m_alive)
    {
        LOG_INFO(field_frontends, "Heartbeat detection is not alive");
        if (WrapAroundNumber32(m_data->next_id) > WrapAroundNumber32(m_data->recv_id) + 3)
        {
            LOG_ERROR(field_frontends, "Heartbeat enter to disconnected");
            m_service.on_sudden_disconnect();
            return Request::r_remove;
        }
    }

    do
    {
        const size_t buffer_size = HeartbeatSession::k_buffer_size;
        Net::AbstractNetBufferContainer container (m_socket_system.buffer_provider().create(buffer_size));
        if (container.is_null()) break;

        container->at_least(buffer_size, buffer_size);
        container->resize(buffer_size);

        byte_t *ptr = container->ptr();
        *reinterpret_cast<uint32_t*>(ptr) = 0xffffffff;

        m_data->next_id++;        
        *reinterpret_cast<uint32_t*>(ptr+4) = Endian::h2n(m_data->next_id.value());

        auto addr = ensure_peer();
        if (!addr.is_valid()) break;

        LOG_INFO(field_frontends, "sending the heartbeat message with {0}...", m_data->next_id);
        m_socket_system.send(container, Net::SockAddress(), addr);
    }
    while(false);

    return Request::r_schedule;
}

Net::SockAddress HeartbeatRegularTask::ensure_peer()
{
    if (m_peer.is_valid()) return m_peer;

    auto settings = m_service.setting_provider().settings();
    if (settings == nullptr) return m_peer;

    String gateway = settings->gateway();
    try
    {
        Array<Net::SockAddress> addresses;
        Net::SockAddress::parse(addresses, gateway, 1);
        if (addresses.count() == 0) return Net::SockAddress();

        m_peer = addresses[0];
        m_peer.change_port(frontend_heartbeat_port());
    }
    catch(const Exception::Exception& ex) {}
    return m_peer;
}

Timespan HeartbeatRegularTask::in_wait()
{
    return m_alive ? m_long_wait : m_short_wait;
}

END_NAMESPACE(Frontend)

