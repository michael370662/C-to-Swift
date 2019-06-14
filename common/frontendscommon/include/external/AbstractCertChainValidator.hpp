#ifndef __PonyTech_Frontend_AbstractCertChainValidator_hpp__
#define __PonyTech_Frontend_AbstractCertChainValidator_hpp__

#include "../PonyTech-Frontend_DEP.hpp"

BEGIN_NAMESPACE(Frontend)


class AbstractCertChainValidator
{
public:
    AbstractCertChainValidator();
    virtual ~AbstractCertChainValidator();
    
    
    virtual bool validate(const PxConstArray<byte_t>& cert, bool online) = 0;

    void register_loader();
    void unregister_loader();
private:
    static int validate_wrap(void *data, void *chunk_ptr, size_t chunk_len, int online);
    FND_DISABLE_COPY(AbstractCertChainValidator);
};



END_NAMESPACE(Frontend)

#endif // __PonyTech_Frontend_AbstractCertChainValidator_hpp__