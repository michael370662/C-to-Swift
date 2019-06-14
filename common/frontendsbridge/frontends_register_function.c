#include "frontends_register_function.h"
#include "frontends_include.h"
#include <daemon.h>
#include <library.h>
#include <../ponywrapping.inl>


#define STRONGSWAN_PLUGINS "openssl nonce eap-identity eap-md5"

#ifdef STATIC_PLUGIN_CONSTRUCTORS
plugin_t *nonce_plugin_create();
plugin_t *openssl_plugin_create();
plugin_t *eap_identity_plugin_create();
plugin_t *eap_md5_plugin_create();

static void register_plugins()
{
	plugin_constructor_register("nonce", nonce_plugin_create);
	plugin_constructor_register("openssl", openssl_plugin_create);
	plugin_constructor_register("eap-identity", eap_identity_plugin_create);
	plugin_constructor_register("eap-md5", eap_md5_plugin_create);
}

static void unregister_plugins()
{
	plugin_constructor_register("nonce", NULL);
	plugin_constructor_register("openssl", NULL);
	plugin_constructor_register("eap-identity", NULL);
	plugin_constructor_register("eap-md5", NULL);
}

#endif 


int frontend_library_init()
{
	if (!library_init(NULL, "charon"))
	{
		library_deinit();
		return FALSE;
	}

    internal_frontend_setting();

	if (!libcharon_init())
	{
        libcharon_deinit();
        library_deinit();
		return FALSE;
	}

    internal_frontend_logger_create();
    charon->load_loggers(charon);
    return TRUE;
}


void frontend_library_deinit()
{
    internal_frontend_logger_destroy();
    libcharon_deinit();
    library_deinit();
}

int frontend_charon_init_plugin()
{
#ifdef STATIC_PLUGIN_CONSTRUCTORS
	register_plugins();
#endif
	if (!charon->initialize(charon, (char*) STRONGSWAN_PLUGINS))
	{
		return FALSE;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);
    return TRUE;
}

void frontend_charon_unload_static_plugin()
{
#ifdef STATIC_PLUGIN_CONSTRUCTORS
	unregister_plugins();
#endif
}

void frontend_charon_start()
{
   	charon->start(charon); 
}

void    frontend_wrap_setup(int setup)
{
    ponytech_collection_setup(setup != FALSE);
}

uint16_t    frontend_heartbeat_port()
{
	return IKEV2_HEARTBEAT;
}


