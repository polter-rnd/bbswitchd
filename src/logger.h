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

/**
 * @file logger.h
 *
 * Logging functions for bbswitch daemon
 */

#ifndef LOGGER_H
#define LOGGER_H

/** Verbosity of logging */
enum log_verbosity {
    VERBOSITY_NONE,  /**< No messages will be shown */
    VERBOSITY_ERR,   /**< Error messages only */
    VERBOSITY_WARN,  /**< Warnings and everything more important */
    VERBOSITY_INFO,  /**< Informational messages and everything more important */
    VERBOSITY_DEBUG, /**< Debugging messages and everything more important */
    VERBOSITY_ALL    /**< All messages will be shown */
};

/** Logging engine */
enum log_type {
    LOGGER_SYSLOG, /**< Redirect messages to syslog */
    LOGGER_FILE,   /**< Write log to file (create if not exist) */
    LOGGER_STDERR  /**< Write log to STDERR */
};


/**
 * Set log verbosity level.
 * Default is #VERBOSITY_INFO
 * @param verbosity Verbosity level
 */
void log_set_verbosity(enum log_verbosity verbosity);

/**
 * Initialize logger
 * @param type Logger type: #LOGGER_SYSLOG, #LOGGER_FILE or #LOGGER_STDERR
 * @param name Log name for #LOGGER_SYSLOG or filename for #LOGGER_FILE.
               Not used for #LOGGER_STDERR.
 * @return 0 on success, -1 otherwise
 */
int log_open(enum log_type type, const char *name);

/**
 * Log error message (shown on >= #VERBOSITY_ERR)
 * @param msg_format Format string to be printed
 * @param ... Arguments for format specification
 */
void log_err(char *msg_format, ...);

/**
 * Log warning message (shown on >= #VERBOSITY_WARN)
 * @param msg_format Format string to be printed
 * @param ... Arguments for format specification
 */
void log_warn(char *msg_format, ...);

/**
 * Log informational message (shown on >= #VERBOSITY_INFO)
 * @param msg_format Format string to be printed
 * @param ... Arguments for format specification
 */
void log_info(char *msg_format, ...);

/**
 * Log informational message (shown on >= #VERBOSITY_DEBUG)
 * @param msg_format Format string to be printed
 * @param ... Arguments for format specification
 */
void log_debug(char *msg_format, ...);

/** 
 * Release logger resources.
 */
void log_close(void);


#endif
