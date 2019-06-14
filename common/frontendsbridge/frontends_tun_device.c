#include <daemon.h>
#include <library.h>
#include <processing/jobs/callback_job.h>

#include "frontends_register_function.h"

typedef void (*TunDeviceUpdateStatus)(void *data, int status);
typedef int  (*TunDeviceEstablish)(void *data, int with_dns, int *already_registered);
typedef void (*TunDevicePostEstablished)(void *data, int already_registered);
typedef int  (*TunDeviceIsEstablished)(void *data);
typedef void (*TunDeviceClose)(void *data);
typedef int  (*TunDeviceAddAddress)(void *data, void* sockaddr);
typedef int  (*TunDeviceAddRoute)(void *data, void *sockaddr, int prefix);


typedef struct RegisterCallback_t RegisterCallback_t;
typedef struct private_service_t private_service_t;

struct RegisterCallback_t
{
	void* provider;
	TunDeviceUpdateStatus		update_status;
	TunDeviceEstablish			establish;	
	TunDevicePostEstablished	post_established;
	TunDeviceIsEstablished		is_established;
	TunDeviceClose				close_tun;
	TunDeviceAddAddress 		add_address;
    TunDeviceAddRoute       	add_route;
};

struct private_service_t 
{
	listener_t listener;
	ike_sa_t *ike_sa;
	RegisterCallback_t	callback;
};

static private_service_t* s_current = NULL;


static bool add_route(RegisterCallback_t *callback, host_t *net, uint8_t prefix)
{
	/* if route is 0.0.0.0/0, split it into two routes 0.0.0.0/1 and
	 * 128.0.0.0/1 because otherwise it would conflict with the current default
	 * route.  likewise for IPv6 with ::/0. */
	if (net->is_anyaddr(net) && prefix == 0)
	{
		bool success;

		success = add_route(callback, net, 1);
		if (net->get_family(net) == AF_INET)
		{
			net = host_create_from_string("128.0.0.0", 0);
		}
		else
		{
			net = host_create_from_string("8000::", 0);
		}
		success = success && add_route(callback, net, 1);
		net->destroy(net);
		return success;
	}

	int result = callback->add_route(callback->provider, net->get_sockaddr(net), prefix);
	if (result == FALSE)
	{
		DBG1(DBG_LIB, "builder: failed to add route");
		return false;		
	}
	return true;
}

static bool add_routes(RegisterCallback_t *callback, child_sa_t *child_sa)
{
	traffic_selector_t *src_ts, *dst_ts;
	enumerator_t *enumerator;
	bool success = TRUE;

	enumerator = child_sa->create_policy_enumerator(child_sa);
	while (success && enumerator->enumerate(enumerator, &src_ts, &dst_ts))
	{
		host_t *net;
		uint8_t prefix;

		dst_ts->to_subnet(dst_ts, &net, &prefix);
		success = add_route(callback, net, prefix);
		net->destroy(net);
	}
	enumerator->destroy(enumerator);
	return success;
}

static bool add_addresses(RegisterCallback_t *callback, ike_sa_t *ike_sa)
{
	host_t *vip;
    bool vip_found = false;

	enumerator_t *enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, TRUE);
	while (enumerator->enumerate(enumerator, &vip))
	{
		if (!vip->is_anyaddr(vip))
		{
			if (callback->add_address(callback->provider, vip->get_sockaddr(vip)) == FALSE)
			{	
				DBG1(DBG_LIB, "builder: failed to add address");
				break;
			}
			vip_found = true;
		}
	}
	enumerator->destroy(enumerator);
    return vip_found;
}

static int setup_tun_device(RegisterCallback_t *callback, ike_sa_t* ike_sa, child_sa_t* child_sa)						
{
    if (callback->provider == NULL) 
    {
        DBG1(DBG_DMN, "failed to setup the tun deivce: Invalid external");
        return FALSE;
    }

	DBG1(DBG_DMN, "setting up TUN device for CHILD_SA %s{%u}",
		 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa));

    if (!add_addresses(callback, ike_sa))
    {
		DBG1(DBG_DMN, "setting up TUN device failed, no virtual IP found");
        return FALSE;
    }

    if (!add_routes(callback, child_sa)) 
    {
        DBG1(DBG_DMN, "setting up TUN device failed, add routes failed");
        return FALSE;
    }

	int already_registered = FALSE;
    int ret = callback->establish(callback->provider, true, &already_registered);
    if (ret == FALSE)
    {
        DBG1(DBG_DMN, "setting up TUN device failed, fail to extablist connection");
        return FALSE;
    }

	DBG1(DBG_DMN, "successfully created TUN device");
	callback->post_established(callback->provider, already_registered);
	return TRUE;
}

static int setup_tun_device_without_dns(RegisterCallback_t *callback)
{
	if (callback->provider == NULL) 
	{
        DBG1(DBG_DMN, "failed to setup the tun deivce: Invalid external");
        return FALSE;
    }
	DBG1(DBG_DMN, "setting up TUN device without DNS");

	int ret = callback->establish(callback->provider, false, NULL);
	if (ret == FALSE)
	{     
		DBG1(DBG_DMN, "setting up TUN device failed, fail to extablist connection");
        return FALSE;
	}
	DBG1(DBG_DMN, "successfully created TUN device without DNS");
	return TRUE;
}

static void close_tun_device(RegisterCallback_t *callback)
{
	if (callback->provider == NULL) 
    {
        DBG1(DBG_DMN, "failed to clost the tun deivce: Invalid external");
		return;
    }
	callback->close_tun(callback->provider);
}

CALLBACK(terminate, job_requeue_t,
	uint32_t *id)
{
	charon->controller->terminate_ike(charon->controller, *id, FALSE,
									  controller_cb_empty, NULL, 0);
	return JOB_REQUEUE_NONE;
}

CALLBACK(reestablish, job_requeue_t,
	uint32_t *id)
{
	ike_sa_t *ike_sa;

	ike_sa = charon->ike_sa_manager->checkout_by_id(charon->ike_sa_manager, *id);
	if (ike_sa)
	{
		if (ike_sa->reauth(ike_sa) == DESTROY_ME)
		{
			charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
														ike_sa);
		}
		else
		{
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		}
	}
	return JOB_REQUEUE_NONE;
}

METHOD(listener_t, ike_updown, bool,
	private_service_t *this, ike_sa_t *ike_sa, bool up)
{
	/* this callback is only registered during initiation, so if the IKE_SA
	 * goes down we assume some kind of authentication error, more specific
	 * errors are caught in the alert() handler */
	if (this->ike_sa == ike_sa && !up)
	{
	    this->callback.update_status(this->callback.provider, (int) SERVICE_AUTH_ERROR);
		return FALSE;
	}
	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_service_t *this, ike_sa_t *old, ike_sa_t *new)
{
	if (this->ike_sa == old)
	{
		this->ike_sa = new;
	}
	return TRUE;
}

METHOD(listener_t, ike_reestablish_post_redirect, bool,
	private_service_t *this, ike_sa_t *old, ike_sa_t *new,
	bool initiated)
{
	if (this->ike_sa == old && initiated)
	{	/* if we get redirected during IKE_AUTH we just migrate to the new SA,
		 * we don't have a TUN device yet, so reinstalling it without DNS would
		 * fail (and using the DNS proxy is not required anyway) */
		this->ike_sa = new;
	}
	return TRUE;
}

METHOD(listener_t, ike_reestablish_pre, bool,
	private_service_t *this, ike_sa_t *old, ike_sa_t *new)
{
	if (this->ike_sa != old) return TRUE;

	// if DNS servers are installed that are only reachable through the VPN
	// the DNS proxy doesn't help, so uninstall DNS servers
	if (setup_tun_device_without_dns(&this->callback) == FALSE)
	{
		this->callback.update_status(this->callback.provider, (int) SERVICE_GENERIC_ERROR);
	}
	return TRUE;
}

METHOD(listener_t, ike_reestablish_post, bool,
	private_service_t *this, ike_sa_t *old, ike_sa_t *new, bool initiated)
{
	if (this->ike_sa == old && initiated)
	{
		this->ike_sa = new;
		// re-register hook to detect initiation failures 
		this->listener.ike_updown = _ike_updown;
		// if the IKE_SA got deleted by the responder we get the child_down()
		// event on the old IKE_SA after this hook has been called, so they
		// get ignored and thus we trigger the event here 
		this->callback.update_status(this->callback.provider, (int) SERVICE_CHILD_STATE_DOWN);
	}
	return TRUE;
}

METHOD(listener_t, child_updown, bool,
	private_service_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa, bool up)
{
	if (this->ike_sa != ike_sa) return TRUE;

	if (!up)
	{
		this->callback.update_status(this->callback.provider, (int) SERVICE_CHILD_STATE_DOWN);
		return TRUE;
	}

	// disable the hooks registered to catch initiation failures 
	this->listener.ike_updown = NULL;
	// enable hooks to handle reauthentications 
	this->listener.ike_reestablish_pre = _ike_reestablish_pre;
	this->listener.ike_reestablish_post = _ike_reestablish_post;
	// CHILD_SA is up so we can disable the DNS proxy we enabled to
	// reestablish the SA
	if (setup_tun_device(&this->callback, ike_sa, child_sa) == FALSE)
	{
		this->callback.update_status(this->callback.provider, (int) SERVICE_GENERIC_ERROR);
		return FALSE;
	}
	this->callback.update_status(this->callback.provider, (int) SERVICE_CHILD_STATE_UP);
	return TRUE;
}

METHOD(listener_t, alert, bool,
	private_service_t *this, ike_sa_t *ike_sa, alert_t alert, va_list args)
{
	if (this->ike_sa != ike_sa) return TRUE;

	bool stay_registered = TRUE;
	switch (alert)
	{
		case ALERT_PEER_ADDR_FAILED:
			this->callback.update_status(this->callback.provider, (int) SERVICE_LOOKUP_ERROR);
			return FALSE;
		case ALERT_PEER_AUTH_FAILED:		
			this->callback.update_status(this->callback.provider, (int) SERVICE_PEER_AUTH_ERROR);
			return FALSE;
		case ALERT_KEEP_ON_CHILD_SA_FAILURE:
		{
			uint32_t *id = malloc_thing(uint32_t);

			/* because close_ike_on_child_failure is set this is only
				* triggered when CHILD_SA rekeying failed. reestablish it in
				* the hope that the initial setup works again. */
			*id = ike_sa->get_unique_id(ike_sa);
			lib->processor->queue_job(lib->processor,
				(job_t*)callback_job_create_with_prio(
					(callback_job_cb_t)reestablish, id, free,
					(callback_job_cancel_t)return_false, JOB_PRIO_HIGH));
			break;
		}
		case ALERT_PEER_INIT_UNREACHABLE:
			if (this->callback.is_established(this->callback.provider) == FALSE)
			{
				uint32_t *id = malloc_thing(uint32_t);

				// always fail if we are not able to initiate the IKE_SA initially 
				this->callback.update_status(this->callback.provider, (int) SERVICE_UNREACHABLE_ERROR);
				// terminate the IKE_SA so no further keying tries are attempted 
				*id = ike_sa->get_unique_id(ike_sa);
				lib->processor->queue_job(lib->processor,
					(job_t*)callback_job_create_with_prio(
						(callback_job_cb_t)terminate, id, free,
						(callback_job_cancel_t)return_false, JOB_PRIO_HIGH));
				stay_registered = FALSE;
			}
			else
			{
				peer_cfg_t *peer_cfg;
				uint32_t tries, try;

				// when reestablishing and if keyingtries is not %forever
				// the IKE_SA is destroyed after the set number of tries, so notify the GUI 
				peer_cfg = ike_sa->get_peer_cfg(ike_sa);
				tries = peer_cfg->get_keyingtries(peer_cfg);
				try = va_arg(args, uint32_t);
				if (tries != 0 && try == tries-1)
				{
					this->callback.update_status(this->callback.provider, (int) SERVICE_UNREACHABLE_ERROR);
					stay_registered = FALSE;
				}
			}
			break;
		default:
			break;
	}
	return stay_registered;
}

static bool service_register(plugin_t *plugin, plugin_feature_t *feature,
								   bool reg, void *data)
{
	private_service_t *this = (private_service_t*) data;
	if (reg)	
	{
		charon->bus->add_listener(charon->bus, &this->listener);
	}
	else
	{
		DBG1(DBG_DMN, "unload service");
		if (s_current != NULL)
		{
			s_current = NULL;;
			DBG1(DBG_DMN, "close service");
			charon->bus->remove_listener(charon->bus, &this->listener);
			close_tun_device(&this->callback);
			free(this);
		}
	}
	return TRUE;
}

int    frontend_tun_device_provide(void *data, 
									TunDeviceUpdateStatus update_status, TunDeviceEstablish establish, 
									TunDevicePostEstablished post_established, TunDeviceIsEstablished is_established,
									TunDeviceClose close_tun, TunDeviceAddAddress add_address,
                                    TunDeviceAddRoute add_route)
{
	if (data == NULL || update_status == NULL || establish == NULL ||
		post_established == NULL || is_established == NULL || close_tun == NULL ||
		add_address == NULL || add_route == NULL) 
		return FALSE;


	private_service_t *this;
	INIT(this,
		.listener = {
			.ike_rekey = _ike_rekey,
			.ike_reestablish_post = _ike_reestablish_post_redirect,
			.ike_updown = _ike_updown,
			.child_updown = _child_updown,
			.alert = _alert,
		},
		.callback =  {
			.provider = data,
			.update_status = update_status,
			.establish = establish,
			.post_established = post_established,
			.is_established = is_established,
			.close_tun = close_tun,
    		.add_address = add_address,
    		.add_route = add_route,
		},
		.ike_sa = NULL,
	);
	if (s_current) free(s_current);
	s_current = this;

	plugin_feature_t features[] = {
		PLUGIN_CALLBACK(service_register, this),
			PLUGIN_PROVIDE(CUSTOM, "frontend-service"),
				PLUGIN_DEPENDS(CUSTOM, "libcharon"),
				PLUGIN_DEPENDS(CERT_DECODE, CERT_X509_CRL),
	};

	lib->plugins->add_static_features(lib->plugins, "frontend-service-modules", features,
									countof(features), TRUE, NULL, NULL);
	return TRUE;
}                                                

void internal_frontend_set_tun_ika_value(void* ike_sa)
{
	if (s_current == NULL) return;
	s_current->ike_sa = (ike_sa_t*) ike_sa;
}