#include <credentials/credential_manager.h>


typedef int (*CertChainValidate)(void *data, void* chunk_ptr, size_t chunk_len, int online);
typedef struct RegisterCallback_t RegisterCallback_t;

struct RegisterCallback_t
{
	void* provider;
	CertChainValidate 	validator;
};
static RegisterCallback_t s_caller;

static bool credential_manager_trust_cert(chunk_t der_format, bool online)
{
    if (s_caller.provider != NULL)
    {
        return s_caller.validator(s_caller.provider, der_format.ptr, der_format.len, online ? TRUE : FALSE) != FALSE;
    }
    return false;
}

void    frontend_cert_chain_validate_register(void *data, int (*handle)
                                                (void *data, void* chunk_ptr, size_t chunk_len, int online))
{
    s_caller.provider = data;
    s_caller.validator = handle;
    register_credential_manager_trust_cert(credential_manager_trust_cert);
}                                                

void    frontend_cert_chain_validate_unregister()
{
    memset(&s_caller, 0, sizeof(s_caller));
    register_credential_manager_trust_cert(NULL);
}