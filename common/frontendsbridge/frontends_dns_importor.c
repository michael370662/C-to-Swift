#include <utils/debug.h>
#include <library.h>
#include <daemon.h>
#include <attributes/attribute_handler.h>

typedef struct private_frontends_dns_importor_t private_frontends_dns_importor_t;


struct private_frontends_dns_importor_t 
{
	attribute_handler_t handler;
	void *data;
	int (*add_dns)(void* data, void *sockaddr);
};

METHOD(attribute_handler_t, handle, bool,
	private_frontends_dns_importor_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	host_t *dns;
	switch (type)
	{
		case INTERNAL_IP4_DNS:
			dns = host_create_from_chunk(AF_INET, data, 0);
			break;
		case INTERNAL_IP6_DNS:
			dns = host_create_from_chunk(AF_INET6, data, 0);
			break;
		default:
			return FALSE;
	}

	if (!dns || dns->is_anyaddr(dns))
	{
		DESTROY_IF(dns);
		return FALSE;
	}
	DBG1(DBG_IKE, "installing DNS server %H", dns);

	int result = this->add_dns(this->data, dns->get_sockaddr(dns));
	dns->destroy(dns);

	if (result == 0)
	{
		DBG1(DBG_LIB, "builder: failed to add DNS server");
		return FALSE;		
	}
	return TRUE;
}

METHOD(attribute_handler_t, release, void,
	private_frontends_dns_importor_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	// DNS servers cannot be removed from an existing TUN device 
}

METHOD(enumerator_t, enumerate_dns6, bool,
	enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	*type = INTERNAL_IP6_DNS;
	*data = chunk_empty;
	this->venumerate = (void*)return_false;
	return TRUE;
}

METHOD(enumerator_t, enumerate_dns4, bool,
	enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	*type = INTERNAL_IP4_DNS;
	*data = chunk_empty;
	this->venumerate = _enumerate_dns6;
	return TRUE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t*,
	private_frontends_dns_importor_t *this, ike_sa_t *ike_sa, linked_list_t *vips)
{
	enumerator_t *enumerator;

	INIT(enumerator,
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_dns4,
			.destroy = (void*)free,
	);
	return enumerator;
}

void*   frontend_dns_importor_create(void* data, int (*add_dns_ptr)(void* data, void *sockaddr))
{
	if (add_dns_ptr == NULL) return NULL;
	private_frontends_dns_importor_t *this;

	INIT(this,
		.handler = {
			.handle = _handle,
			.release = _release,
			.create_attribute_enumerator = _create_attribute_enumerator,
		},
		.data = data,
		.add_dns = add_dns_ptr,
	);
	return this;
}

void    frontend_dns_importor_destroy(void* self)
{
	private_frontends_dns_importor_t *this = (private_frontends_dns_importor_t *) self;
	free(this);
}

static bool dns_importor_register(plugin_t *plugin, plugin_feature_t *feature,
								   bool reg, void *data)
{
	private_frontends_dns_importor_t *this = (private_frontends_dns_importor_t*) data;
	if (reg)
	{
		DBG1(DBG_LIB, "load dns importor");
		charon->attributes->add_handler(charon->attributes,	&this->handler);
	}
	else
	{
		DBG1(DBG_LIB, "unload dns importor");
		charon->attributes->remove_handler(charon->attributes, &this->handler);
	}
	return TRUE;
}

int     frontend_dns_importor_setup(void *self)
{
	plugin_feature_t features[] = {
		PLUGIN_CALLBACK(dns_importor_register, self),
			PLUGIN_PROVIDE(CUSTOM, "dns-importor"),
				PLUGIN_DEPENDS(CUSTOM, "libcharon"),
	};	
	lib->plugins->add_static_features(lib->plugins, "dns-importor-modules", features,
									countof(features), TRUE, NULL, NULL);
	return TRUE;
}
