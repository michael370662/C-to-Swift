#ifndef __PonyTech_Frontend_ConnectionTask_hpp__
#define __PonyTech_Frontend_ConnectionTask_hpp__

#include <PonyTech-Frontend_DEP.hpp>

BEGIN_NAMESPACE(Frontend)

class ConnectionSystem;

class MakeConnectionTask : public FmWork::Task
{
    ConnectionSystem&           m_connection_system;
    Net::SockAddress            m_peer;
public:
    MakeConnectionTask(ConnectionSystem& connection, const Net::SockAddress& peer);
    virtual ~MakeConnectionTask(){}


    virtual Priority get_priority() const override { return Priority::p_medium; }
    virtual bool on_removed(bool shutdown) override { delete this; return true;}
    virtual Request execute(FmWork::TaskSubmitter &submitter) override;

private:
    FND_DISABLE_COPY(MakeConnectionTask);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_ConnectionTask_hpp__