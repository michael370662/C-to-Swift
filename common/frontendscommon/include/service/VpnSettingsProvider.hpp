#ifndef __PonyTech_Frontend_VpnSettingsProvider_hpp__
#define __PonyTech_Frontend_VpnSettingsProvider_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class AbstractVpnSettings
{
public:
    AbstractVpnSettings(){}
    virtual ~AbstractVpnSettings(){}

    virtual String username() const = 0;
    virtual String password() const = 0;
    virtual String gateway() const = 0;
    virtual String remote_id() const = 0;
    virtual String local_id() const = 0;
    virtual int mtu() const = 0;
    virtual int port() const = 0;
    virtual int keep_alive() const = 0;
    virtual bool ignore_ca_trust() const = 0;
private:
    FND_DISABLE_COPY(AbstractVpnSettings);
};


class VpnSettingsProvider
{
    AutoPtr<AbstractVpnSettings> m_settings;
public:
    VpnSettingsProvider();
    virtual ~VpnSettingsProvider();

    const AbstractVpnSettings* settings() const { return m_settings.ptr(); }
    void set(AutoPtr<AbstractVpnSettings> &data);
    void set_null();
private:
    FND_DISABLE_COPY(VpnSettingsProvider);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_VpnSettingsProvider_hpp__