#ifndef __PonyTech_Frontend_EAPMd5Provider_hpp__
#define __PonyTech_Frontend_EAPMd5Provider_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class VpnSettingsProvider;

class EAPMd5Provider
{
    const VpnSettingsProvider&    m_provider;
public:
    EAPMd5Provider(const VpnSettingsProvider& provider);
    ~EAPMd5Provider();
    
 
    void register_loader();
    void unregister_loader();
private:
    static void get_username_wrap(void *data, void *ws, void (*handle)(void *ws, const void *username, size_t cnt));
    static void get_password_wrap(void *data, void *ws, void (*handle)(void *ws, const void *password, size_t cnt));
    FND_DISABLE_COPY(EAPMd5Provider);
};


END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_EAPMd5Provider_hpp__