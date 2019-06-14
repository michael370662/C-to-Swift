#include "frontends_include.h"
#include <bus/bus.h>
#include <daemon.h>

extern void (*dbg)(debug_t group, level_t level, char *fmt, ...);
typedef void (*FrontendLogForwarder)(int, const char* data, size_t cnt);

typedef struct frontend_log_logger_t frontend_log_logger_t;
struct frontend_log_logger_t 
{
	logger_t logger;
	int		 level;
};


static frontend_log_logger_t *external_logger = NULL;
static FrontendLogForwarder s_log_forwarder = NULL;
static bool s_logger_accept = false;

static int convert_to(level_t lv)
{
	switch((int) lv)
	{
		case 0: 	return 1;
		case 1:		return 2;
		case 2:		return 3;
		case 3:
		case 4:		return 4; 
	}
	return 0;
}

static void log_data(debug_t group, level_t level, int thread, const char* message)
{
	char data[4096];
	size_t start;
	int lv;
	const char *next, *current;

	if (s_log_forwarder == NULL) return;

	snprintf(data, sizeof(data), "%.2d[%N] ",	thread, debug_names, group);
	start = strlen(data);
	lv = convert_to(level);

	current = message;
	while(TRUE)
	{
		next = strchr(current, '\n');
		if (next == NULL)
		{
			snprintf(data + start, sizeof(data)-start, "%s", current);
			s_log_forwarder(lv, data, strlen(data));
			break;
		}
		snprintf(data + start, sizeof(data)- start,"%.*s", (int)(next - current), current);
		s_log_forwarder(lv, data, start + (int) (next - current));
		current = next + 1;
	}
}

static void log_hooker(debug_t group, level_t level, char *fmt, ...)
{
	char buffer[4096];
	va_list arg;

	va_start(arg, fmt);
 	vsnprintf(buffer, sizeof(buffer), fmt, arg);
	log_data(group, level, 0, buffer);
	va_end(arg);
}



METHOD(logger_t, log_, void,
	frontend_log_logger_t *this, debug_t group, level_t level,
	int thread, ike_sa_t* ike_sa, const char *message)
{
	log_data(group, level, thread, message);
}


METHOD(logger_t, get_level, level_t,
	frontend_log_logger_t *this, debug_t group)
{
	return 1;
}

void internal_frontend_logger_create()
{
	frontend_log_logger_t *this;
	if (!s_logger_accept) return;

	INIT(this,
        .logger = {
            .log = _log_,
            .get_level = _get_level,
        },
	);

	external_logger = this;
	charon->bus->add_logger(charon->bus, &this->logger);
}

void internal_frontend_logger_destroy()
{
	if (s_logger_accept && external_logger != NULL)
	{
		charon->bus->remove_logger(charon->bus, &external_logger->logger);
		free(external_logger);
		external_logger = NULL;
	}
}

void frontend_register_log_destination(void (*handle)(int, const char* fmt, size_t cnt))
{
	dbg = log_hooker;
	s_log_forwarder = handle;
	s_logger_accept = true;
}
