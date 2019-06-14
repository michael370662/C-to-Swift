#ifndef __PonyTech_Frontend_StartConnectionTask_hpp__
#define __PonyTech_Frontend_StartConnectionTask_hpp__

#include <PonyTech-Frontend_DEP.hpp>

BEGIN_NAMESPACE(Frontend)

class VpnFrontendService;
class VpnSettingsProvider;

class StartConnectionTask : public FmWork::Task
{
    const VpnSettingsProvider&  m_settings;
    VpnFrontendService&         m_service;
public:
    StartConnectionTask(const VpnSettingsProvider& setting, VpnFrontendService& service);
    virtual ~StartConnectionTask(){}

    virtual Priority get_priority() const override { return Priority::p_medium; }
    virtual bool on_removed(bool shutdown) override { delete this; return true;}
    virtual Request execute(FmWork::TaskSubmitter &submitter) override;

private:
    static void update_status_wrap(void *data, int code);
    FND_DISABLE_COPY(StartConnectionTask);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_StartConnectionTask_hpp__