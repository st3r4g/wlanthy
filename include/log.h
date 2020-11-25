enum loglevel {
	LV_DEBUG,
	LV_INFO,
	LV_ERROR,
};

void log_set_loglevel(enum loglevel loglevel_);
void log_line(enum loglevel loglevel_, const char *fmt, ...);
void log_head(enum loglevel loglevel_);
void log_body(enum loglevel loglevel_, const char *fmt, ...);
void log_tail(enum loglevel loglevel_);
