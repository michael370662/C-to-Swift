#include <daemon.h>
#include <library.h>
#include <kernel/kernel_net.h>

typedef void* 	(*KernelNetCreate)(void *provider);
typedef void 	(*KernelNetDestroy)(void *self);
typedef void  	(*KernelNetAddIp)(void *self, void* sockaddr);
typedef void  	(*KernelNetDelIp)(void *self, void *sockaddr);
typedef void** 	(*KernalNetAllIps)(void *self, size_t* size,
                        void* (*allocate)(size_t size), 
                        void* (*create_host)(void *sockaddr));
typedef void*	(*KernalNetGetHost)(void *self, void *srcsockaddr, void* dstsockaddr,
						void* (*create_host)(void *sockaddr));

typedef struct private_frontends_kernel_net_t private_frontends_kernel_net_t;
typedef struct RegisterCallback_t RegisterCallback_t;
typedef struct private_loop_t private_loop_t;

struct RegisterCallback_t
{
	void* provider;
    KernelNetCreate     creator;
    KernelNetDestroy    destroyer;
    KernelNetAddIp      add_ip;
    KernelNetDelIp      del_ip;
	KernalNetAllIps		all_ips;
	KernalNetGetHost	get_host;
};

static RegisterCallback_t s_caller;

struct private_frontends_kernel_net_t {
	kernel_net_t        public;
    void*               external;
    KernelNetDestroy    destroyer;
    KernelNetAddIp      add_ip;
    KernelNetDelIp      del_ip;
	KernalNetAllIps		all_ips;
	KernalNetGetHost	get_host;
};

struct private_loop_t 
{
	enumerator_t	interface;
	host_t**		items;
	size_t			cnt, curr;
};

METHOD(enumerator_t, enumerate_destroy, void,
	private_loop_t *this)
{
	size_t i;
	for(i=0; i<this->cnt; i++) this->items[i]->destroy(this->items[i]);
	free(this->items);
	free(this);
}

METHOD(enumerator_t, enumerate_current, bool,
	private_loop_t *this, va_list args)
{
	void **item;
	VA_ARGS_VGET(args, item);

	if (this->curr >= this->cnt)
	{
		return FALSE;
	}
	if (item)
	{
		*item = this->items[this->curr];
	}
	this->curr++;
	return TRUE;
}

static void* wrap_host_sockaddr(void *sockaddr)
{
	return host_create_from_sockaddr((sockaddr_t*) sockaddr);
}

METHOD(kernel_net_t, get_source_addr, host_t*,
	private_frontends_kernel_net_t *this, host_t *dest, host_t *src)
{
	return this->get_host(this->external, 
				src == NULL ? NULL :src->get_sockaddr(src), 
				dest == NULL ? NULL : dest->get_sockaddr(dest), 
				wrap_host_sockaddr);
}

METHOD(kernel_net_t, get_nexthop, host_t*,
	private_frontends_kernel_net_t *this, host_t *dest, int prefix, host_t *src,
	char **iface)
{
	return NULL;
}

METHOD(kernel_net_t, get_interface, bool,
	private_frontends_kernel_net_t *this, host_t *host, char **name)
{
	if (name)
	{	// the actual name does not matter in our case 
		*name = strdup("ponytech");
	}
	return TRUE;
}

METHOD(kernel_net_t, create_address_enumerator, enumerator_t*,
	private_frontends_kernel_net_t *this, kernel_address_type_t which)
{
	// return virtual IPs if requested, nothing otherwise 
	if (which & ADDR_TYPE_VIRTUAL)
	{
		size_t max_cnt;
		private_loop_t* enumerator;
		host_t** item = (host_t**) this->all_ips(this->external, &max_cnt, 
				malloc, wrap_host_sockaddr);
		INIT(enumerator,
			.interface = {
				.enumerate = enumerator_enumerate_default,
				.venumerate = _enumerate_current,
				.destroy = _enumerate_destroy,
			},
			.curr = 0,
			.cnt = max_cnt,
			.items = item,
		);
		return &enumerator->interface;
	}
	return enumerator_create_empty();
}

METHOD(kernel_net_t, add_ip, status_t,
	private_frontends_kernel_net_t *this, host_t *virtual_ip, int prefix, char *iface)
{
	this->add_ip(this->external, virtual_ip->get_sockaddr(virtual_ip));
	return SUCCESS;
}

METHOD(kernel_net_t, del_ip, status_t,
	private_frontends_kernel_net_t *this, host_t *virtual_ip, int prefix, bool wait)
{
	this->del_ip(this->external, virtual_ip->get_sockaddr(virtual_ip));
	return SUCCESS;
}

METHOD(kernel_net_t, add_route, status_t,
	private_frontends_kernel_net_t *this, chunk_t dst_net, uint8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_net_t, del_route, status_t,
	private_frontends_kernel_net_t *this, chunk_t dst_net, uint8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_net_t, destroy, void,
	private_frontends_kernel_net_t *this)
{
    if (this->external != NULL) this->destroyer(this->external);
	free(this);
}

static kernel_net_t *frontend_kernel_net_create()
{
	private_frontends_kernel_net_t *this;

	INIT(this,
		.public = {
			.get_source_addr = _get_source_addr,
			.get_nexthop = _get_nexthop,
			.get_interface = _get_interface,
			.create_address_enumerator = _create_address_enumerator,
			.add_ip = _add_ip,
			.del_ip = _del_ip,
			.add_route = _add_route,
			.del_route = _del_route,
			.destroy = _destroy,
		},
        .destroyer = s_caller.destroyer,
        .add_ip = s_caller.add_ip,
        .del_ip = s_caller.del_ip,
		.all_ips = s_caller.all_ips,
		.get_host = s_caller.get_host,
	);

    this->external = s_caller.creator(s_caller.provider);
	return &this->public;
}

int    frontend_kernel_net_provide(void *provider,
                void* (*creator)(void *provider),
                void  (*destroy)(void *self),
                void  (*add_ip)(void *self, void* sockaddr),
                void  (*del_ip)(void *self, void* sockaddr),
				void** (*all_ips)(void *self, size_t* size,
                        void* (*allocate)(size_t size), 
                        void* (*create_host)(void *sockaddr)),
				void* (*get_host)(void *self, void *srcsockaddr, void* dstsockaddr,
						void* (*create_host)(void *sockaddr))
				)
{
	s_caller.provider = provider;
	s_caller.creator = creator;
	s_caller.destroyer = destroy;
	s_caller.add_ip = add_ip;
	s_caller.del_ip = del_ip;
	s_caller.all_ips = all_ips;
	s_caller.get_host = get_host;

	if (provider == NULL || creator == NULL || destroy == NULL || 
		add_ip == NULL || del_ip == NULL || all_ips == NULL ||
		get_host == NULL) return FALSE;

	plugin_feature_t features[] = {
		PLUGIN_CALLBACK(kernel_net_register, frontend_kernel_net_create),
			PLUGIN_PROVIDE(CUSTOM, "kernel-net"),
	};	
	lib->plugins->add_static_features(lib->plugins, "frontend-kernel_net-modules", features,
									countof(features), TRUE, NULL, NULL);
	return TRUE;
}

void    frontend_kernel_net_remove()
{
	memset(&s_caller, 0, sizeof(s_caller));
}

// TODO

// #define ROAM_DELAY 100
// #define ROAM_DELAY_RECHECK 1000

// static job_requeue_t roam_event()
// {
// 	/* this will fail if no connection is up */
// 	vpn_system_bypass_socket(-1, 0);
// 	charon->kernel->roam(charon->kernel, TRUE);
// 	return JOB_REQUEUE_NONE;
// }

// /**
//  * Listen for connectivity change events and queue a roam event
//  */
// static void connectivity_cb(void *data,	int  disconnected)
// {
//     private_android_net_t *this = (private_android_net_t*) data;
// 	timeval_t now;
// 	job_t *job;

// 	time_monotonic(&now);
// 	this->mutex->lock(this->mutex);
// 	this->connected = disconnected == 0;
// 	if (!timercmp(&now, &this->next_roam, >))
// 	{
// 		this->mutex->unlock(this->mutex);
// 		return;
// 	}
// 	timeval_add_ms(&now, ROAM_DELAY);
// 	this->next_roam = now;
// 	this->mutex->unlock(this->mutex);

// 	job = (job_t*)callback_job_create((callback_job_cb_t)roam_event, NULL,
// 									   NULL, NULL);
// 	lib->scheduler->schedule_job_ms(lib->scheduler, job, ROAM_DELAY);
// }

