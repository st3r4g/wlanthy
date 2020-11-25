#include "log.h"

#include <stdio.h>
#include <stdarg.h>

static enum loglevel loglevel = LV_ERROR;

static const char *loglevelstr[] = {
	"dbg",
	"inf",
	"ERR",
};

void log_set_loglevel(enum loglevel loglevel_) {
	loglevel = loglevel_;
}

void log_line(enum loglevel loglevel_, const char *fmt, ...) {
	if (loglevel_ < loglevel)
		return;
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "[wlanthy:%s] ", loglevelstr[loglevel_]);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void log_head(enum loglevel loglevel_) {
	if (loglevel_ < loglevel)
		return;
	fprintf(stderr, "[wlanthy:%s] ", loglevelstr[loglevel_]);
}
void log_body(enum loglevel loglevel_, const char *fmt, ...) {
	if (loglevel_ < loglevel)
		return;
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
void log_tail(enum loglevel loglevel_) {
	if (loglevel_ < loglevel)
		return;
	fprintf(stderr, "\n");
}
