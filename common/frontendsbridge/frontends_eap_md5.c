#include <libcharon/plugins/eap_md5/eap_md5.h>


typedef void (*GetUsername)(void *data, void* ws, void (*callback)(void *ws, const void* username, size_t cnt));
typedef void (*GetPassword)(void *data, void *ws, void (*callback)(void *ws, const void* password, size_t cnt));

typedef struct eap_md5_workspace_t eap_md5_workspace_t;
typedef struct RegisterCallback_t RegisterCallback_t;

struct eap_md5_workspace_t 
{
    identification_t* me;
    shared_key_t* key;
    bool exact_match;
};

struct RegisterCallback_t
{
	void* provider;
	GetUsername 	get_username;
    GetPassword     get_password;
};
static RegisterCallback_t s_caller;

static void verify_username(void* ws, const void *username, size_t count)
{
    eap_md5_workspace_t* curr = (eap_md5_workspace_t*) ws;
    identification_t*id = identification_create_from_data(chunk_create((void*)username, count)); 
    curr->exact_match = id->matches(id, curr->me) == ID_MATCH_PERFECT;
    id->destroy(id);
}    

static void construct_shared_key(void *ws, const void* password, size_t count)
{
    eap_md5_workspace_t* curr = (eap_md5_workspace_t*) ws;
    chunk_t secret = chunk_create((void*) password, count);
	curr->key = shared_key_create(SHARED_EAP, chunk_clone(secret));
}

static shared_key_t* eap_md5_get_shared_key_wrap(identification_t *me)
{
    if (s_caller.provider == NULL) return NULL;
    eap_md5_workspace_t ws;
    ws.me = me;
    ws.key = NULL;
    ws.exact_match = false;

    s_caller.get_username(s_caller.provider, &ws, &verify_username);
    if (!ws.exact_match) return NULL;
    s_caller.get_password(s_caller.provider, &ws, &construct_shared_key);
    return ws.key;
}


void    frontend_eap_md5_register(void *data, GetUsername get_username, GetPassword get_password)
{
    s_caller.provider = data;
    s_caller.get_username = get_username;
    s_caller.get_password = get_password;
    register_eap_md5_get_shared_key_wrap(eap_md5_get_shared_key_wrap);
}                                                

void    frontend_eap_md5_unregister()
{
    memset(&s_caller, 0, sizeof(s_caller));
    register_credential_manager_trust_cert(NULL);
}