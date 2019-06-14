#include "external/EAPMd5Provider.hpp"
#include "service/VpnSettingsProvider.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

EAPMd5Provider::EAPMd5Provider(const VpnSettingsProvider& provider) :
m_provider(provider)
{
}

EAPMd5Provider::~EAPMd5Provider()
{
}

void EAPMd5Provider::register_loader()
{
    frontend_eap_md5_register(this, &EAPMd5Provider::get_username_wrap, 
                                    &EAPMd5Provider::get_password_wrap);
}

void EAPMd5Provider::unregister_loader()
{
    frontend_eap_md5_unregister();
}

void EAPMd5Provider::get_username_wrap(void *data, void *ws,
                void (*handle)(void *ws, const void* username, size_t count))
{
    auto p = static_cast<EAPMd5Provider*>(data);
    auto setting = p->m_provider.settings();
    if (setting == nullptr) return;

    auto username = setting->username();
    handle(ws, username.ptr(), username.length());
}

void EAPMd5Provider::get_password_wrap(void *data, void *ws,
                void (*handle)(void *ws, const void* password, size_t count))
{
    auto p = static_cast<EAPMd5Provider*>(data);
    auto setting = p->m_provider.settings();
    if (setting == nullptr) return;

    auto password = setting->password();

    String sha_str;

    {   ManagedStringBuffer buffer(sha_str);
        Utf8OutputStream stream(buffer);

        Crypto::SHA1 sha1;
        OnstackArray<byte_t, Crypto::SHA1::k_result_size> sha_result;
        ConstArray<byte_t> content(reinterpret_cast<const byte_t*>(password.ptr()), password.length());

        sha1.update(content);
        sha1.digest(sha_result); 
        stream.write(HexByteArrayRepresentation(sha_result, HexByteArrayRepresentation::Mode::m_normal, false));
    }
    handle(ws, sha_str.ptr(), sha_str.length());
}

END_NAMESPACE(Frontend)