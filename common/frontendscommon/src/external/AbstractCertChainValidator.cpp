#include "external/AbstractCertChainValidator.hpp"
#include "../PonyTech-Frontend_include.hpp"

BEGIN_NAMESPACE(Frontend)

AbstractCertChainValidator::AbstractCertChainValidator()
{
}

AbstractCertChainValidator::~AbstractCertChainValidator()
{
}

void AbstractCertChainValidator::register_loader()
{
    frontend_cert_chain_validate_register(this, &AbstractCertChainValidator::validate_wrap);
}

void AbstractCertChainValidator::unregister_loader()
{
    frontend_cert_chain_validate_unregister();
}

int AbstractCertChainValidator::validate_wrap(void *data, void *chunk_ptr, size_t chunk_len, int online)
{
    auto p = static_cast<AbstractCertChainValidator*>(data);
    auto ret =  p->validate(ConstArray<byte_t>(static_cast<byte_t*>(chunk_ptr), chunk_len), online != FALSE);
    return ret ? TRUE : FALSE;
}

END_NAMESPACE(Frontend)