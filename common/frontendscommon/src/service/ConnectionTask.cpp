#include "ConnectionTask.hpp"
#include "service/ConnectionSystem.hpp"

BEGIN_NAMESPACE(Frontend)

MakeConnectionTask::MakeConnectionTask(ConnectionSystem& connection, const Net::SockAddress& peer) :
m_connection_system(connection),
m_peer(peer)
{
}

FmWork::Task::Request MakeConnectionTask::execute(FmWork::TaskSubmitter &submitter) 
{
    try
    {
        m_connection_system.create_socket(Net::AddressDefine::NetFamily::nf_ipv4);
        m_connection_system.connect(m_peer);
    }
    catch(const Exception::Exception& ex)
    {
        LOG_ERROR(field_frontends, "Fail to established connect to {0}", m_peer);
    }
    return FmWork::Task::Request::r_remove;
}



END_NAMESPACE(Frontend)