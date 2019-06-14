#include "StartConnectionTask.hpp"
#include "external/VpnFrontendService.hpp"
#include "service/VpnSettingsProvider.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

StartConnectionTask::StartConnectionTask(const VpnSettingsProvider& settings, VpnFrontendService& service):
m_settings(settings),
m_service(service)
{
}

FmWork::Task::Request StartConnectionTask::execute(FmWork::TaskSubmitter &submitter) 
{
    auto setting = m_settings.settings();
    if (setting == nullptr) 
    {
        LOG_ERROR(field_frontends, "No Vpn Setting object");
        m_service.change_status(SERVICE_GENERIC_ERROR);
        return FmWork::Task::Request::r_remove;
    }
    
    String username = setting->username();
    String gateway = setting->gateway();
    String local_id = setting->local_id();
    String remote_id = setting->remote_id();
    int port = setting->port();

    LOG_VERB(field_frontends, "Starting connection with username '{0}', local_id '{1}', "
                "remote_id '{2}', gateway '{3}', port ={4}",
             username,local_id, remote_id, gateway, port);


    frontend_execute_initiate(
        username.length() == 0 ? nullptr : username.cstring().ptr(),
        gateway.length() == 0 ? nullptr : gateway.cstring().ptr(),
        local_id.length() == 0 ? nullptr : local_id.cstring().ptr(),
        remote_id.length() == 0 ? nullptr : remote_id.cstring().ptr(),
        port, this, StartConnectionTask::update_status_wrap);

    return FmWork::Task::Request::r_remove;
}

void StartConnectionTask::update_status_wrap(void *data, int code)
{
    auto p = static_cast<StartConnectionTask*>(data);
    p->m_service.change_status(code);
}


END_NAMESPACE(Frontend)