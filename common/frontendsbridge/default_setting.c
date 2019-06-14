#include <library.h>

#define RETRASNMIT_TRIES 3
#define RETRANSMIT_TIMEOUT 2.0
#define RETRANSMIT_BASE 1.4
#define KEEPALIVE_INTERVAL 45
#define MTU_VALUE 1400

void internal_frontend_setting()
{
	lib->settings->set_int(lib->settings, "charon.retransmit_tries", RETRASNMIT_TRIES);
	lib->settings->set_double(lib->settings, "charon.retransmit_timeout", RETRANSMIT_TIMEOUT);
	lib->settings->set_double(lib->settings, "charon.retransmit_base", RETRANSMIT_BASE);
	lib->settings->set_bool(lib->settings,"charon.close_ike_on_child_failure", TRUE);
}

void frontend_plugins_settings_reload(int mtu, int keep_alive)
{
	if (mtu < 200) mtu = MTU_VALUE;
	if (keep_alive < 10) keep_alive = KEEPALIVE_INTERVAL;
	

	lib->settings->set_bool(lib->settings,
						"charon.plugins.revocation.enable_crl", TRUE);
	lib->settings->set_bool(lib->settings,
						"charon.plugins.revocation.enable_ocsp", TRUE);
	lib->settings->set_bool(lib->settings,
						"charon.rsa_pss", FALSE);

	lib->settings->set_int(lib->settings, "charon.fragment_size", mtu);
	lib->settings->set_int(lib->settings, "charon.keep_alive", keep_alive);

	// reload plugins after changing settings 
	lib->plugins->reload(lib->plugins, NULL);
}