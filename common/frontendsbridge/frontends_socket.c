#include <network/socket_manager.h>
#include <network/socket_manager_frontends.h>

 
 void     frontend_socket_provide(  FrontendsSocketInit init,
                                    FrontendsSocketClose closer, 
                                    FrontendsSocketGetPort get_port,
                                    FrontendsSocketGetSupported is_supported,
                                    FrontendsSocketSend sender)
{
    register_frontends_socket_init(init);
    register_frontends_socket_close(closer);
    register_frontends_socket_get_port(get_port);
    register_frontends_socket_get_supported(is_supported);
    register_frontends_socket_send(sender);
    register_socket_manager_create(socket_manager_create_frontends);
}                                    
         

void    frontend_socket_remove()
{
    register_frontends_socket_init(NULL);
    register_frontends_socket_close(NULL);
    register_frontends_socket_get_port(NULL);
    register_frontends_socket_get_supported(NULL);
    register_frontends_socket_send(NULL);
    register_socket_manager_create(NULL);
}
