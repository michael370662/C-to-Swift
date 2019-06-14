#ifndef FRONTENDS_INCLUDE_FUNCTION_H_
#define FRONTENDS_INCLUDE_FUNCTION_H_

#include <library.h>
#include "frontends_register_function.h"


// in log_destinator.c
extern void internal_frontend_logger_create();
extern void internal_frontend_logger_destroy();
// in default_setting.c
extern void internal_frontend_setting();
// in frontends_tun_device
extern void internal_frontend_set_tun_ika_value(void *ike_sa);


#endif 
