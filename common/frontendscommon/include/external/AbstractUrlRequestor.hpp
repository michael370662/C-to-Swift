#ifndef __PonyTech_Frontend_AbstractUrlRequestor_hpp__
#define __PonyTech_Frontend_AbstractUrlRequestor_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)


class AbstractUrlRequestor
{
public:   
    enum OptType 
    {
        t_data = 0,
        t_request,
    };
private:
    typedef void (*Callback)(void *cb_data, void *userdata, void* data, size_t len);
    void*       m_inner;
    Callback    m_callback;
public:
    AbstractUrlRequestor();
    virtual ~AbstractUrlRequestor();

    virtual bool set_option(OptType type, void *data) = 0;
    virtual bool fetch(void* userdata, const PxConstString& uri) = 0;
protected:
    void pass_back(const PxConstArray<byte_t>& content, void* userdata);
private:
    friend class BaseUrlRequestorProvider;
    FND_DISABLE_COPY(AbstractUrlRequestor);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_AbstractUrlRequestor_hpp__