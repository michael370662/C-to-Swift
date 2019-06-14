#include <utils/debug.h>
#include <library.h>
#include <daemon.h>
#include <ipsec_sa_mgr.h>
#include <ipsec_policy_mgr.h>

typedef int   (*IpsecByPassSocket)(void *obj, int fd, int family);
typedef struct RegisterCallback_t RegisterCallback_t;
typedef struct private_kernel_frontends_ipsec_t private_kernel_frontends_ipsec_t;


struct RegisterCallback_t
{
	void* provider;
	IpsecByPassSocket 	bypass;
};
static RegisterCallback_t s_caller;


struct private_kernel_frontends_ipsec_t
{
	kernel_ipsec_t interface;
	RegisterCallback_t caller;
};

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_kernel_frontends_ipsec_t *this, host_t *src, host_t *dst,
	uint8_t protocol, uint32_t *spi)
{
	bool success;
	IPsecMgrGetSpi ptr = pointer_ipsec_sa_mgr_get_spi();
	if (ptr == NULL) 
	{
		DBG1(DBG_ESP, "failed to get spi: no external");
		return FAILED;		
	}

	success = ptr(spi);
	if (success) DBG2(DBG_ESP, "allocated SPI %.8x", ntohl(*spi));
	return success ? SUCCESS : FAILED;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_frontends_ipsec_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_frontends_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	return internal_ipsec_sa_mgr_add_sa(id->src, id->dst, id->spi, id->proto,
					data->reqid, id->mark, data->tfc, data->lifetime,
					data->enc_alg, data->enc_key, data->int_alg, data->int_key,
					data->mode, data->ipcomp, data->cpi, data->initiator,
					data->encap, data->esn, data->inbound, data->update);
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_frontends_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	bool success;
	IPsecMgrUpdateSa ptr;

	DBG2(DBG_ESP, "updating SAD entry with SPI %.8x from %#H..%#H to %#H..%#H",
		 ntohl(id->spi), id->src, id->dst, data->new_src, data->new_dst);

	if (!data->new_encap)
	{
		DBG1(DBG_ESP, "failed to update SAD entry: can't deactivate UDP "
			 "encapsulation");
		return NOT_SUPPORTED;
	}

	ptr = pointer_ipsec_sa_mgr_update_sa();
	if (ptr != NULL)
	{
		DBG1(DBG_ESP, "failed to update SAD entry: no external");
		return FAILED;
	}
	success = ptr(id->spi, id->src->get_sockaddr(id->src), id->dst->get_sockaddr(id->dst), 
				data->new_src->get_sockaddr(data->new_src), data->new_dst->get_sockaddr(data->new_dst));
	if (!success)
	{
		DBG1(DBG_ESP, "failed to update SAD entry: not found");
		return FAILED;
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_frontends_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	uint64_t time_val;
	bool success;

	IPsecMgrQuerySa ptr = pointer_ipsec_sa_mgr_query_sa();
	if (ptr == NULL) 
	{
		DBG1(DBG_ESP, "failed to query SAD entry: no external");
		return FAILED;
	}

	success = ptr(id->spi, id->src->get_sockaddr(id->src), id->dst->get_sockaddr(id->dst), &time_val, packets, bytes);	
	if (success && time != NULL) *time = (time_t) time_val;
	return success ? SUCCESS : NOT_FOUND;
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_frontends_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	bool success;
	IPsecMgrRemoveSa ptr = pointer_ipsec_sa_mgr_remove_sa();
	if (ptr == NULL)
	{
		DBG1(DBG_ESP, "failed to delete SAD entry: no external");
		return FAILED;
	}

	success = ptr(id->spi, id->src->get_sockaddr(id->src), id->dst->get_sockaddr(id->dst));
	return success ? SUCCESS : NOT_FOUND;
}

METHOD(kernel_ipsec_t, flush_sas, status_t,
	private_kernel_frontends_ipsec_t *this)
{
	IPsecMgrClearSas ptr = pointer_ipsec_sa_mgr_clean_sas();
	if (ptr == NULL) 
	{
		DBG1(DBG_ESP, "failed to flush SAD: no external");
		return FAILED;
	}
	DBG2(DBG_ESP, "flushing SAD");

	ptr();
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_frontends_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	bool src_v4, dst_v4;
	uint8_t src_proto, dst_proto, src_netbit, dst_netbit;
	uint16_t src_from_port, src_to_port, dst_from_port, dst_to_port;
	void *src_from_addr, *src_to_addr, *dst_from_addr, *dst_to_addr;
	status_t status = SUCCESS;

	SAPolicyMgrAdd ptr = pointer_ipsec_policy_mgr_add();
	if (ptr == NULL) return FAILED;

	if (data->type == POLICY_IPSEC && id->dir != POLICY_FWD)
	{
		DBG2(DBG_ESP, "adding policy %R === %R %N", id->src_ts, id->dst_ts,
				 policy_dir_names, id->dir);


		src_v4 = id->src_ts->get_type(id->src_ts) == TS_IPV4_ADDR_RANGE;
		src_proto =	id->src_ts->get_protocol(id->src_ts);
		src_netbit = id->src_ts->get_netbits(id->src_ts);
		src_from_port =	id->src_ts->get_from_port(id->src_ts);
		src_to_port = id->src_ts->get_to_port(id->src_ts);
		src_from_addr = id->src_ts->get_from_address(id->src_ts).ptr;
		src_to_addr = id->src_ts->get_to_address(id->src_ts).ptr;

		dst_v4 = id->dst_ts->get_type(id->dst_ts) == TS_IPV4_ADDR_RANGE;
		dst_proto =	id->dst_ts->get_protocol(id->dst_ts);
		dst_netbit = id->dst_ts->get_netbits(id->dst_ts);
		dst_from_port =	id->dst_ts->get_from_port(id->dst_ts);
		dst_to_port = id->dst_ts->get_to_port(id->dst_ts);
		dst_from_addr =	id->dst_ts->get_from_address(id->dst_ts).ptr;
		dst_to_addr = id->dst_ts->get_to_address(id->dst_ts).ptr;


		status = ptr(id->dir == POLICY_IN, data->prio, data->sa->reqid, id->mark.value, id->mark.mask,
			data->src->get_sockaddr(data->src), data->dst->get_sockaddr(data->dst), 
			src_v4, dst_v4,src_proto, dst_proto, src_netbit, dst_netbit,
			src_from_port, dst_from_port, src_to_port, dst_to_port,
			src_from_addr, dst_from_addr, src_to_addr, dst_to_addr) ? SUCCESS : FAILED;
	}
	return status;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_frontends_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_frontends_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	bool src_v4, dst_v4;
	uint8_t src_proto, dst_proto, src_netbit, dst_netbit;
	uint16_t src_from_port, src_to_port, dst_from_port, dst_to_port;
	void *src_from_addr, *src_to_addr, *dst_from_addr, *dst_to_addr;
	status_t status = FAILED;

	SAPolicyMgrRemove ptr = pointer_ipsec_policy_mgr_remove();
	if (ptr != NULL && data->type == POLICY_IPSEC && id->dir != POLICY_FWD)
	{
		DBG2(DBG_ESP, "deleting policy %R === %R %N", id->src_ts, id->dst_ts,
			policy_dir_names, id->dir);


		src_v4 = id->src_ts->get_type(id->src_ts) == TS_IPV4_ADDR_RANGE;
		src_proto =	id->src_ts->get_protocol(id->src_ts);
		src_netbit = id->src_ts->get_netbits(id->src_ts);
		src_from_port =	id->src_ts->get_from_port(id->src_ts);
		src_to_port = id->src_ts->get_to_port(id->src_ts);
		src_from_addr = id->src_ts->get_from_address(id->src_ts).ptr;
		src_to_addr = id->src_ts->get_to_address(id->src_ts).ptr;

		dst_v4 = id->dst_ts->get_type(id->dst_ts) == TS_IPV4_ADDR_RANGE;
		dst_proto =	id->dst_ts->get_protocol(id->dst_ts);
		dst_netbit = id->dst_ts->get_netbits(id->dst_ts);
		dst_from_port =	id->dst_ts->get_from_port(id->dst_ts);
		dst_to_port = id->dst_ts->get_to_port(id->dst_ts);
		dst_from_addr =	id->dst_ts->get_from_address(id->dst_ts).ptr;
		dst_to_addr = id->dst_ts->get_to_address(id->dst_ts).ptr;
	
		status =  ptr(id->dir == POLICY_IN, data->prio, data->sa->reqid, id->mark.value, id->mark.mask,
			src_v4, dst_v4,src_proto, dst_proto, src_netbit, dst_netbit,
			src_from_port, dst_from_port, src_to_port, dst_to_port,
			src_from_addr, dst_from_addr, src_to_addr, dst_to_addr) ? SUCCESS : FAILED;
	}
	return status;
}

METHOD(kernel_ipsec_t, flush_policies, status_t,
	private_kernel_frontends_ipsec_t *this)
{
	SAPolicyMgrFlush ptr = pointer_ipsec_policy_mgr_flush();
	if (ptr != NULL) 
	{
		DBG2(DBG_ESP, "flushing policies");
		ptr();
	}
	return ptr == NULL ? FAILED : SUCCESS;
}

METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_kernel_frontends_ipsec_t *this, int fd, int family)
{
	return this->caller.bypass(this->caller.provider, fd, family) != FALSE;
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_kernel_frontends_ipsec_t *this, int fd, int family, uint16_t port)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_frontends_ipsec_t *this)
{
	free(this);
}

static kernel_ipsec_t *frontends_kernel_ipsec_create()
{
	private_kernel_frontends_ipsec_t *this;

	INIT(this,
		.interface = {
			.get_spi = _get_spi,
			.get_cpi = _get_cpi,
			.add_sa  = _add_sa,
			.update_sa = _update_sa,
			.query_sa = _query_sa,
			.del_sa = _del_sa,
			.flush_sas = _flush_sas,
			.add_policy = _add_policy,
			.query_policy = _query_policy,
			.del_policy = _del_policy,
			.flush_policies = _flush_policies,
			.bypass_socket = _bypass_socket,
			.enable_udp_decap = _enable_udp_decap,
			.destroy = _destroy,
		},
		.caller = s_caller,
	);
	return &this->interface;
}


int    frontend_kernel_ipsec_provide(void *provider,
                int (*bypass)(void *provider, int fd, int family))
{
	s_caller.provider = provider;
	s_caller.bypass = bypass;
	if (provider == NULL || bypass == NULL) return FALSE;

	plugin_feature_t features[] = {
		PLUGIN_CALLBACK(kernel_ipsec_register, frontends_kernel_ipsec_create),
			PLUGIN_PROVIDE(CUSTOM, "kernel-ipsec"),
	};

	lib->plugins->add_static_features(lib->plugins, "frontend-ipsec-modules", features,
									countof(features), TRUE, NULL, NULL);
	return TRUE;
}

void    frontend_kernel_ipsec_remove()
{
	memset(&s_caller, 0, sizeof(s_caller));
}
