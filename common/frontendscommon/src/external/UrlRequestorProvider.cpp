#include "external/UrlRequestorProvider.hpp"
#include "external/AbstractUrlRequestor.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

BaseUrlRequestorProvider::BaseUrlRequestorProvider()
{
}

BaseUrlRequestorProvider::~BaseUrlRequestorProvider()
{
}

void BaseUrlRequestorProvider::register_loader()
{
    int ret = frontend_url_requestor_provide(this,
        creator, destroyer, fetch, set_opt);
    if (ret == FALSE)
    {
        LOG_ERROR(field_frontends, "Cannot load the url requestor into plugin");
    }
}

void BaseUrlRequestorProvider::unregister_loader()
{
    frontend_url_requestor_remove();
}

void* BaseUrlRequestorProvider::creator(void* provider, void* inner, 
    void (*cb)(void* cb_data, void* userdata, void* chunk_ptr, size_t chunk_size))
{
    auto p = static_cast<BaseUrlRequestorProvider*>(provider);
    auto requestor = p->create_requestor();
    if (requestor == nullptr) return nullptr;

    requestor->m_inner = inner;
    requestor->m_callback = cb;
    return requestor;
}

void BaseUrlRequestorProvider::destroyer(void *requestor)
{
    delete static_cast<AbstractUrlRequestor*>(requestor);
}

int BaseUrlRequestorProvider::set_opt(void *requestor, int opt_type, void *data)
{
    auto p = static_cast<AbstractUrlRequestor*>(requestor);
    return p->set_option(static_cast<AbstractUrlRequestor::OptType>(opt_type), data) ? TRUE : FALSE;
}

int  BaseUrlRequestorProvider::fetch(void *requestor, void *userdata, const char* url)
{
    auto p = static_cast<AbstractUrlRequestor*>(requestor);
    return p->fetch(userdata, ConstString(url)) ? TRUE : FALSE; 
}

END_NAMESPACE(Frontend)