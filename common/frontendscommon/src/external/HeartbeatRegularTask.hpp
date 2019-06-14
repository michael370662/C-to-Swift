#ifndef __PonyTech_Frontend_HeartbeatRegularTask_hpp__
#define __PonyTech_Frontend_HeartbeatRegularTask_hpp__

#include <PonyTech-Frontend_DEP.hpp>

BEGIN_NAMESPACE(Frontend)

class VpnFrontendService;
class SocketSystem;

class HeartbeatSession 
{
public:
     constexpr static size_t k_buffer_size = 8;

    AtomicInt   stopped;
    AtomicInt   next_id;
    AtomicInt   recv_id;
public:
    HeartbeatSession(){}
    ~HeartbeatSession(){}

    void received(Net::AbstractNetBufferContainer& container);
private:
    FND_DISABLE_COPY(HeartbeatSession);
};


class HeartbeatRegularTask : public FmWork::Task
{
    SocketSystem&               m_socket_system;
    VpnFrontendService&         m_service;
    SharedPtr<HeartbeatSession> m_data;
    Net::SockAddress            m_peer;
    bool                        m_alive;

    const static Timespan  m_long_wait;
    const static Timespan  m_short_wait;
public:
    HeartbeatRegularTask(SocketSystem& socket_system, VpnFrontendService& service, const SharedPtr<HeartbeatSession>& data);
    virtual ~HeartbeatRegularTask();

    virtual Priority get_priority() const override { return Priority::p_medium; }
    virtual bool on_removed(bool shutdown) override { delete this; return true;}
    virtual Request execute(FmWork::TaskSubmitter &submitter) override;
    virtual Timespan in_wait() override;
private:
    Net::SockAddress ensure_peer();
    FND_DISABLE_COPY(HeartbeatRegularTask);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_HeartbeatRegularTask_hpp__