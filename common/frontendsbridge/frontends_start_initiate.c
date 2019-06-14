#include "frontends_register_function.h"
#include "frontends_include.h"
#include <daemon.h>
#include <library.h>


static void add_auth_cfg_pw(const char *username, const char *local_id,
							peer_cfg_t *peer_cfg)
{
	identification_t *user, *id = NULL;
	auth_cfg_t *auth;

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_EAP);

	/* in case EAP-PEAP or EAP-TTLS is used we currently accept any identity */
	auth->add(auth, AUTH_RULE_AAA_IDENTITY,
			  identification_create_from_string("%any"));


	user = identification_create_from_string((char*) username);
	auth->add(auth, AUTH_RULE_EAP_IDENTITY, user);
	if (local_id)
	{
		id = identification_create_from_string((char *) local_id);
	}
	if (!id)
	{
		id = user->clone(user);
	}
	auth->add(auth, AUTH_RULE_IDENTITY, id);
	peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);
}

uint16_t frontend_default_gateway_port() { return IKEV2_UDP_PORT; }

void frontend_execute_initiate(const char *username, const char* server,
							const char *local_id, const char* remote_id, int port,
                            void *ws, void (*update_status)(void *ws, int code))
{
	identification_t *gateway = NULL;
	ike_cfg_t *ike_cfg;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	traffic_selector_t *ts;
	ike_sa_t *ike_sa;
	auth_cfg_t *auth;
	peer_cfg_create_t peer = {
		.cert_policy = CERT_ALWAYS_SEND,
		.unique = UNIQUE_REPLACE,
		.rekey_time = 36000, /* 10h */
		.jitter_time = 600, /* 10min */
		.over_time = 600, /* 10min */
	};
	child_cfg_create_t child = {
		.lifetime = {
			.time = {
				.life = 3600, /* 1h */
				.rekey = 3000, /* 50min */
				.jitter = 300 /* 5min */
			},
		},
		.mode = MODE_TUNNEL,
		.dpd_action = ACTION_RESTART,
		.close_action = ACTION_RESTART,
	};
	if (port == 0) port = IKEV2_UDP_PORT;

	peer.keyingtries = 1;
	ike_cfg = ike_cfg_create(IKEV2, TRUE, TRUE, "0.0.0.0",
							 charon->socket->get_port_v2(charon->socket, FALSE),
							 (char *) server, port, 0);
	ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
	ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));

	peer_cfg = peer_cfg_create("frontend", ike_cfg, &peer);
	peer_cfg->add_virtual_ip(peer_cfg, host_create_any(AF_INET));
	peer_cfg->add_virtual_ip(peer_cfg, host_create_any(AF_INET6));

	add_auth_cfg_pw(username, local_id, peer_cfg);

	/* remote auth config */
	auth = auth_cfg_create();
	if (remote_id)
	{
		gateway = identification_create_from_string((char*) remote_id);
	}
	if (!gateway || gateway->get_type(gateway) == ID_ANY)
	{
		DESTROY_IF(gateway);
		gateway = identification_create_from_string((char*) server);
		/* only use this if remote ID was not configured explicitly */
		auth->add(auth, AUTH_RULE_IDENTITY_LOOSE, TRUE);
	}
	auth->add(auth, AUTH_RULE_IDENTITY, gateway);
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	peer_cfg->add_auth_cfg(peer_cfg, auth, FALSE);

	child_cfg = child_cfg_create("android", &child);
	/* create ESP proposals with and without DH groups, let responder decide
		 * if PFS is used */
	child_cfg->add_proposal(child_cfg, proposal_create_from_string(PROTO_ESP,
							"aes256gcm16-aes128gcm16-chacha20poly1305-"
							"curve25519-ecp384-ecp521-modp3072-modp4096-ecp256-modp8192"));
	child_cfg->add_proposal(child_cfg, proposal_create_from_string(PROTO_ESP,
							"aes256-aes192-aes128-sha384-sha256-sha512-sha1-"
							"curve25519-ecp384-ecp521-modp3072-modp4096-ecp256-modp2048-"
							"modp8192"));
	child_cfg->add_proposal(child_cfg, proposal_create_from_string(PROTO_ESP,
							"aes256gcm16-aes128gcm16-chacha20poly1305"));
	child_cfg->add_proposal(child_cfg, proposal_create_from_string(PROTO_ESP,
							"aes256-aes192-aes128-sha384-sha256-sha512-sha1"));
	ts = traffic_selector_create_from_cidr("0.0.0.0/0", 0, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	ts = traffic_selector_create_from_cidr("0.0.0.0/0", 0, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
	ts = traffic_selector_create_from_cidr("::/0", 0, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	ts = traffic_selector_create_from_cidr("::/0", 0, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
	peer_cfg->add_child_cfg(peer_cfg, child_cfg);

	/* get us an IKE_SA */
	ike_sa = charon->ike_sa_manager->checkout_by_config(charon->ike_sa_manager,
														peer_cfg);
	if (!ike_sa)
	{
		peer_cfg->destroy(peer_cfg);

        update_status(ws, (int) SERVICE_GENERIC_ERROR);
		return;
	}
	if (!ike_sa->get_peer_cfg(ike_sa))
	{
		ike_sa->set_peer_cfg(ike_sa, peer_cfg);
	}
	peer_cfg->destroy(peer_cfg);

	/* store the IKE_SA so we can track its progress */
	internal_frontend_set_tun_ika_value(ike_sa);

	/* get an additional reference because initiate consumes one */
	child_cfg->get_ref(child_cfg);
	if (ike_sa->initiate(ike_sa, child_cfg, 0, NULL, NULL) != SUCCESS)
	{
		DBG1(DBG_CFG, "failed to initiate tunnel");
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
			ike_sa);
		return;
	}
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
}