#ifndef NativeHeader_h
#define NativeHeader_h

#include <stdbool.h>

typedef void (*ObjectStringCallback)(void *obj, void *ws, void (*f)(void *ws, const char*));

void on_lib_load();
void on_lib_unload();
void set_log_file(const char *path, bool append);
void send_log_message(const char *message, int leve);
bool initialize_charon();
void deinitialize_charon();
void initiate_service(void *settings);
void send_tun_data(const void* data, int cnt);


void register_print_log(void (*func)(const char*));
void register_swift_destroy(void (*func)(void *));

void register_vpn_setting_username(ObjectStringCallback f);
void register_vpn_setting_password(ObjectStringCallback f);
void register_vpn_setting_gateway(ObjectStringCallback f);
void register_vpn_setting_server_id(ObjectStringCallback f);

void register_vpn_service_state(void (*func)(int));

void register_tun_setting_set_addr(void (*func)(const char*, int));
void register_tun_setting_add_route(void (*func)(const char*, int));
void register_tun_setting_add_dns(void (*func)(const char*));
void register_tun_setting_establish(bool (*func)());
void register_tun_setting_send_packet(void (*func)(const void*, int));

#endif /* NativeHeader_h */
