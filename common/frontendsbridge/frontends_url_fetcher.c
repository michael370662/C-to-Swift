#include <utils/debug.h>
#include <library.h>
#include <daemon.h>

typedef void* (*UrlRequestorCreate)(void* provider, void* inner, void (*cb)(void*, void*, void*, size_t));
typedef void  (*UrlRequestorDestroy)(void *obj);
typedef int   (*UrlRequestorFetch)(void *obj, void* userdata, const char* uri);
typedef int   (*UrlRequestorSetOpt)(void *obj, int opt, void *data);
typedef struct RegisterCallback_t RegisterCallback_t;
typedef struct private_frontend_fetcher_t private_frontend_fetcher_t;

struct RegisterCallback_t
{
	void* provider;
	UrlRequestorDestroy	destroyer;
	UrlRequestorCreate  creator;
	UrlRequestorFetch	fetcher;
	UrlRequestorSetOpt	set_opt;
};
static RegisterCallback_t s_caller;


struct private_frontend_fetcher_t 
{
 	fetcher_t public;
 	fetcher_callback_t cb;

	void* 					external;
	UrlRequestorDestroy		destroyer;
	UrlRequestorFetch		fetch_action;
	UrlRequestorSetOpt		set_opt;
};

static void process_content(void *inner, void* userdata, void* res_ptr, size_t res_size)
{
	private_frontend_fetcher_t *this = (private_frontend_fetcher_t*) inner;
	if (this->cb) this->cb(userdata, chunk_create(res_ptr, res_size));
}

METHOD(fetcher_t, destroy, void,
	private_frontend_fetcher_t *this)
{
	if (this->external != NULL) this->destroyer(this->external);
	free(this);
}

METHOD(fetcher_t, fetch, status_t,
	private_frontend_fetcher_t *this, char *uri, void *userdata)
{
	if (this->external == NULL) 
	{
		DBG1(DBG_LIB, "failed to fetch from '%s': invalid external", uri);
		return FAILED;
	}

	int result = this->fetch_action(this->external, userdata, uri);
	return result != 0 ? SUCCESS : FAILED;
}

METHOD(fetcher_t, set_option, bool,
 	private_frontend_fetcher_t *this, fetcher_option_t option, ...)
{
	chunk_t chunk;
	char* request_pointer;
 	bool supported = FALSE;
	

 	va_list args;
 	va_start(args, option);
 	switch (option)
 	{
		case FETCH_CALLBACK:
		{
			this->cb = va_arg(args, fetcher_callback_t);
			supported = TRUE;
			break;			
		}
		case FETCH_REQUEST_DATA:
		{
			chunk = va_arg(args, chunk_t);
			if (this->external != NULL) supported = this->set_opt(this->external, 1, (void*) &chunk) != 0 ? TRUE : FALSE;
			break;
		}
		case FETCH_REQUEST_TYPE:
		{
			request_pointer = va_arg(args, char*);
			if (this->external != NULL) supported = this->set_opt(this->external, 2, (void*) request_pointer) != 0 ? TRUE : FALSE;
			break;
		}
		default: break;
	}
	va_end(args);
	return supported;
}


static fetcher_t *frontend_url_fetcher_create()
{
	private_frontend_fetcher_t *this;
	INIT(this,
		.public = {
			.fetch = _fetch,
			.set_option = _set_option,
			.destroy = _destroy,
		},
		.cb = fetcher_default_callback,
		.destroyer = s_caller.destroyer,
		.fetch_action = s_caller.fetcher,
		.set_opt = s_caller.set_opt,
		.external = NULL,
	);

	this->external = s_caller.creator == NULL ? NULL : s_caller.creator(s_caller.provider, this, process_content);
	return &this->public;
}

int    frontend_url_requestor_provide(void *provider,
                void* (*creator)(void *provider, void* inner, void (*cb)(void* inner, void* userdata, void* chunk_ptr, size_t chunk_len)),
                void  (*destroy)(void *self),
                int   (*fetch)(void *self, void* userdata, const char* uri),
                int   (*set_opt)(void *self, int opt, void *data))
{
	s_caller.provider = provider;
	s_caller.creator = creator;
	s_caller.destroyer = destroy;
	s_caller.fetcher = fetch;
	s_caller.set_opt = set_opt;

	if (provider == NULL || creator == NULL || destroy == NULL || fetch == NULL || set_opt == NULL) return FALSE;

	plugin_feature_t features[] = {
		PLUGIN_REGISTER(FETCHER, frontend_url_fetcher_create),
			PLUGIN_PROVIDE(FETCHER, "http://"),
			PLUGIN_PROVIDE(FETCHER, "https://"),
	};	
	lib->plugins->add_static_features(lib->plugins, "frontend-fetcher-modules", features,
									countof(features), TRUE, NULL, NULL);
	return TRUE;
}

void    frontend_url_requestor_remove()
{
	memset(&s_caller, 0, sizeof(s_caller));
}
