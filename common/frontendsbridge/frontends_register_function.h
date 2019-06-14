#ifndef FRONTENDS_REGISTER_FUNCTION_H_
#define FRONTENDS_REGISTER_FUNCTION_H_

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//TODO
void frontend_register_log_destination(void (*handle)(int, const char* data, size_t cnt));


enum vpn_state_t
{
    SERVICE_CHILD_STATE_UP = 1,
    SERVICE_CHILD_STATE_DOWN,
    SERVICE_AUTH_ERROR,
    SERVICE_PEER_AUTH_ERROR,
    SERVICE_LOOKUP_ERROR,
    SERVICE_UNREACHABLE_ERROR,
    SERVICE_CERTIFICATE_UNAVAILABLE,
    SERVICE_GENERIC_ERROR,
};

// ---------- register --------------


// frontends_register_function.c
void        frontend_wrap_setup(int setup);
int         frontend_library_init();
void        frontend_library_deinit();
void        frontend_charon_start();
int         frontend_charon_init_plugin();
void        frontend_charon_unload_static_plugin();
uint16_t    frontend_heartbeat_port();


// frontends_dns_importor.c
void*   frontend_dns_importor_create(void* data, int (*add_dns)(void* data, void *sockaddr));
void    frontend_dns_importor_destroy(void* self);
int     frontend_dns_importor_setup(void *self);

// frontends_url_fetcher.c
int     frontend_url_requestor_provide(void *provider,
                void* (*creator)(void *provider, void* inner, void (*cb)(void* inner, void* userdata, void* chunk_ptr, size_t chunk_len)),
                void  (*destroy)(void *self),
                int   (*fetch)(void *self, void* userdata, const char* uri),
                int   (*set_opt)(void *self, int opt, void *data));
void    frontend_url_requestor_remove();

// frontends_kernel_ipsec.c
int     frontend_kernel_ipsec_provide(void *provider,
                int (*bypass)(void *provider, int fd, int family));
void    frontend_kernel_ipsec_remove();

// frontends_kernel_net.c
int     frontend_kernel_net_provide(void *provider,
                void* (*creator)(void *provider),
                void  (*destroy)(void *self),
                void  (*add_ip)(void *self, void* sockaddr),
                void  (*del_ip)(void *self, void* sockaddr),
                void** (*all_ips)(void *self, size_t* size,
                        void* (*allocate)(size_t size), 
                        void* (*create_host)(void *sockaddr)),
                void* (*get_host)(void *self, void *srcsockaddr, void* dstsockaddr,
						void* (*create_host)(void *sockaddr))
                        );
void    frontend_kernel_net_remove();


// frontends_cert_chain_validate.c
void    frontend_cert_chain_validate_register(void *data, int (*handle)
                                                (void *data, void* chunk_ptr, size_t chunk_len, int online));
void    frontend_cert_chain_validate_unregister();                                                

// frontends_eap_md5.c
void    frontend_eap_md5_register(void *data, void (*get_username)(void *data, void* ws, 
                                              void (*callback)(void *ws, const void* username, size_t count)),
                                              void (*get_password)(void *data, void *ws,
                                              void (*callback)(void *ws, const void* password, size_t count))
                                               );
void    frontend_eap_md5_unregister();                                                

// frontends_tun_device.c
int     frontend_tun_device_provide(void *data, 
                                void (*update_status)(void *data, int value),
                                int (*establish)(void *data, int with_dns, int *already_registered),
                                void (*post_established)(void *data, int already_registered),
                                int (*is_established)(void *data),
                                void (*close_tun)(void *data),
                                int (*add_address)(void *data, void* sockaddr),
                                int (*add_route)(void *data, void *sockaddr, int prefix));

// frontends_socket.c
void     frontend_socket_provide(
                                void* (*init)(void (*handle)(bool need_skip, void *src_sockaddr, void *dst_sockaddr, void *buffer, size_t len)),
                                void (*shutdown)(void* data),
                                uint16_t (*get_port)(void* data),
                                bool (*is_supported)(void* data),
                                void (*send_msg)(void* data, void  *src_sockaddr, void *dst_sockaddr, void *buffer, size_t len));
void    frontend_socket_remove();


// frontends_start_initiate.c
uint16_t frontend_default_gateway_port();
void    frontend_execute_initiate(const char *username, const char* server,
		    	    				const char *local_id, const char* remote_id, int port,
                                    void *data, void (*update_status)(void *data, int code));


// default_settings.c
void    frontend_plugins_settings_reload(int mtu, int keep_alive);

// socket_hostaddr_helper.c
int     frontend_udpsock_open();
void    frontend_udpsock_close(int fd);
int     frontend_udpsock_get_host(int fd, void* ret_addr, void* owner, void* dst_addr, 
                            void (*onflush)(void *owner), 
                            void (*set_addr)(void *ret, void *sockaddr));




#ifdef __cplusplus
}
#endif


#endif 
