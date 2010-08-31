/*
 * Logging for unix service.
 *
 * Copyright (c) 2007-2009 Marko Kreen, Skype Technologies OÜ
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */ 

#include <usual/logging.h>

#include <stdio.h>
#include <stdarg.h>

#include <usual/string.h>
#include <usual/time.h>

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef WIN32
#define LOG_EMERG       0
#define LOG_ALERT       1
#define LOG_CRIT        2
#define LOG_ERR         3
#define LOG_WARNING     4
#define LOG_NOTICE      5
#define LOG_INFO        6
#define LOG_DEBUG       7

#define LOG_PID 0
#define LOG_DAEMON 0

#define openlog(a,b,c)
#define syslog win32_eventlog
#define closelog()
static void win32_eventlog(int level, const char *fmt, ...);
#endif

int cf_quiet = 0;
int cf_verbose = 0;
const char *cf_logfile = NULL;
const char *cf_syslog_ident = NULL;

/* optional function to fill prefix */
logging_prefix_fn_t logging_prefix_cb;

static FILE *log_file = NULL;
static bool syslog_started = false;

struct LevelInfo {
	const char *tag;
	int syslog_prio;
};

static const struct LevelInfo log_level_list[] = {
	{ "FATAL", LOG_CRIT },
	{ "ERROR", LOG_ERR },
	{ "WARNING", LOG_WARNING },
	{ "LOG", LOG_INFO },
	{ "DEBUG", LOG_DEBUG },
	{ "NOISE", LOG_DEBUG },
};

void reset_logging(void)
{
	if (log_file) {
		fclose(log_file);
		log_file = NULL;
	}
	if (syslog_started) {
		closelog();
		syslog_started = 0;
	}
}


static void start_syslog(void)
{
	openlog(cf_syslog_ident, LOG_PID, LOG_DAEMON);
	syslog_started = 1;
}


void log_generic(enum LogLevel level, void *ctx, const char *fmt, ...)
{
	char buf[2048];
	char ebuf[256];
	char timebuf[64];
	const struct LevelInfo *lev = &log_level_list[level];
	unsigned pid = getpid();
	va_list ap;
	int pfxlen = 0;
	int old_errno = errno;

	if (logging_prefix_cb) {
		pfxlen = logging_prefix_cb(level, ctx, buf, sizeof(buf));
		if (pfxlen < 0)
			goto done;
		if (pfxlen >= (int)sizeof(buf))
			pfxlen = sizeof(buf) - 1;
	}
	va_start(ap, fmt);
	vsnprintf(buf + pfxlen, sizeof(buf) - pfxlen, fmt, ap);
	va_end(ap);

	format_time_ms(0, timebuf, sizeof(timebuf));

	if (!log_file && cf_logfile && cf_logfile[0]) {
		log_file = fopen(cf_logfile, "a");
		if (log_file) {
			/* Got the file, disable buffering */
			setvbuf(log_file, NULL, _IONBF, 0);
		} else {
			/* Unable to open, complain and fail */
			fprintf(stderr, "%s %u %s Cannot open logfile: '%s': %s\n",
				timebuf, pid, log_level_list[0].tag,
				cf_logfile,
				strerror_r(errno, ebuf, sizeof(ebuf)));
			exit(1);
		}
	}

	if (!cf_quiet)
		fprintf(stderr, "%s %u %s %s\n", timebuf, pid, lev->tag, buf);

	if (log_file)
		fprintf(log_file, "%s %u %s %s\n", timebuf, pid, lev->tag, buf);

	if (cf_syslog_ident) {
		if (!syslog_started)
			start_syslog();
		syslog(lev->syslog_prio, "%s", buf);
	}
done:
	if (old_errno != errno)
		errno = old_errno;
}


void log_fatal(const char *file, int line, const char *func, bool show_perror, void *ctx, const char *fmt, ...)
{
	char buf[2048], ebuf[256];
	const char *estr = NULL;
	int old_errno = 0;
	va_list ap;

	if (show_perror) {
		old_errno = errno;
		estr = strerror_r(errno, ebuf, sizeof(ebuf));
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (show_perror) {
		log_generic(LG_FATAL, ctx, "@%s:%d in function %s(): %s: %s [%d]",
			     file, line, func, buf, estr, old_errno);
	} else {
		log_generic(LG_FATAL, ctx, "@%s:%d in function %s(): %s",
			     file, line, func, buf);
	}
}

#ifdef WIN32

static void win32_eventlog(int level, const char *fmt, ...)
{
	static HANDLE evtHandle = INVALID_HANDLE_VALUE;
	int elevel;
	char buf[1024];
	const char *strlist[1] = { buf };
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	switch (level) {
	case LOG_CRIT:
	case LOG_ERR:
		elevel = EVENTLOG_ERROR_TYPE;
		break;
	case LOG_WARNING:
		elevel = EVENTLOG_WARNING_TYPE;
		break;
	default:
		elevel = EVENTLOG_INFORMATION_TYPE;
	}

	if (evtHandle == INVALID_HANDLE_VALUE) {
		evtHandle = RegisterEventSource(NULL, cf_syslog_ident);
		if (evtHandle == NULL || evtHandle == INVALID_HANDLE_VALUE) {
			evtHandle = INVALID_HANDLE_VALUE;
			return;
		}
	}
	ReportEvent(evtHandle, elevel, 0, 0, NULL, 1, 0, strlist, NULL);
}

#endif

