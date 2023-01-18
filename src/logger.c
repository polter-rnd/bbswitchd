/*
 * Copyright (c) 2022-2023, Pavel Artsishevsky
 * Author: Pavel Artsishevsky <polter.rnd@gmail.com>
 *
 * This file is part of bbswitchd.
 *
 * bbswitchd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bbswitchd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bbswitchd. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>

#include "logger.h"


/* Verbosity level */
static enum log_verbosity S_verbosity = VERBOSITY_INFO;
static enum log_type S_type = LOGGER_STDERR;
static FILE *S_file = NULL;


void log_set_verbosity(enum log_verbosity verbosity) {
    S_verbosity = verbosity;
}

int log_open(enum log_type type, const char *name) {
    /*  Open Logggin mechanism based on configuration */
    S_type = type;
    switch (type) {
    case LOGGER_SYSLOG:
        openlog(name, LOG_PID, LOG_DAEMON);
        return 0;

    case LOGGER_FILE:
        S_file = fopen(name, "a");
        if (!S_file)
          return -1;
        break;

    case LOGGER_STDERR:
        S_file = stderr;
        break;
    }

    return 0;
}

static void log_vprint(int level, char *msg_format, va_list args) {
    struct timespec tp;
    const char *level_name;

    switch (S_type) {
    case LOGGER_SYSLOG:
        vsyslog(level, msg_format, args);
        break;

    case LOGGER_FILE:
    case LOGGER_STDERR:
        switch (level) {
        case LOG_ERR:
            level_name = "ERROR";
            break;
        case LOG_DEBUG:
            level_name = "DEBUG";
            break;
        case LOG_WARNING:
            level_name = " WARN";
            break;
        default:
            level_name = " INFO";
        }

        clock_gettime(CLOCK_MONOTONIC, &tp);

        fprintf(S_file, "[%5llu.%06lu] %s ",
                (unsigned long long) tp.tv_sec,
                (unsigned long) tp.tv_nsec / 1000,
                level_name);
        vfprintf(S_file, msg_format, args);
        fflush(S_file);

        break;
    }
}

void log_err(char *msg_format, ...) {
    va_list args;
    if (S_verbosity < VERBOSITY_ERR)
        return;
    va_start(args, msg_format);
    log_vprint(LOG_ERR, msg_format, args);
    va_end(args);
}

void log_warn(char *msg_format, ...) {
    va_list args;
    if (S_verbosity < VERBOSITY_WARN)
        return;
    va_start(args, msg_format);
    log_vprint(LOG_WARNING, msg_format, args);
    va_end(args);
}

void log_info(char *msg_format, ...) {
    va_list args;
    if (S_verbosity < VERBOSITY_INFO)
        return;
    va_start(args, msg_format);
    log_vprint(LOG_INFO, msg_format, args);
    va_end(args);
}

void log_debug(char *msg_format, ...) {
    va_list args;
    if (S_verbosity < VERBOSITY_DEBUG)
        return;
    va_start(args, msg_format);
    log_vprint(LOG_DEBUG, msg_format, args);
    va_end(args);
}

void log_close(void) {
    switch (S_type) {
    case LOGGER_SYSLOG:
        closelog();
        break;
    case LOGGER_FILE:
        fclose(S_file);
        break;
    default:
        break;
    }
}
