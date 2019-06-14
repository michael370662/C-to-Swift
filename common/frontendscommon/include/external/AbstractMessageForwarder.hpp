#ifndef __PonyTech_Frontend_AbstractMessageForwarder_hpp__
#define __PonyTech_Frontend_AbstractMessageForwarder_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)


class AbstractMessageForwarder
{
public:
    virtual void forward(Net::AbstractNetBufferContainer& container, const Net::SockAddress& self, const Net::SockAddress& peer) = 0;
};

END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_AbstractMessageForwarder_hpp__