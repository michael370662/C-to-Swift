#ifndef __PonyTech_Frontend_UrlRequestorProvider_hpp__
#define __PonyTech_Frontend_UrlRequestorProvider_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)

class AbstractUrlRequestor;

class BaseUrlRequestorProvider
{
public:
    BaseUrlRequestorProvider();
    virtual ~BaseUrlRequestorProvider();


    virtual AbstractUrlRequestor* create_requestor() = 0;

    void register_loader();
    void unregister_loader();
private:
    static void* creator(void* provider, void* inner,
                 void (*cb)(void* cb_data, void* userdata, void* chunk_ptr, size_t chunk_size));
    static void destroyer(void *requestor);
    static int  set_opt(void *requestor, int opt_type, void *data);
    static int  fetch(void *requestor, void *userdata, const char* url);
    FND_DISABLE_COPY(BaseUrlRequestorProvider);
};




template<class T>
class UrlRequestorProvider : public BaseUrlRequestorProvider
{
public:
    UrlRequestorProvider(){}
    ~UrlRequestorProvider(){}

    virtual AbstractUrlRequestor* create_requestor() override { return new T(); }
private:
    FND_DISABLE_COPY(UrlRequestorProvider<T>);
};

END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_UrlRequestorProvider_hpp__