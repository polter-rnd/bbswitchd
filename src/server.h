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
 * @file server.h
 *
 * Functions for serving client connections
 */

#ifndef SERVER_H
#define SERVER_H

/* For size_t type */
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>


/** Server connection structure. Passed to ::server_request_handler_t */
struct server_conn {
    struct sockaddr_un from; /**< UNIX socket address of recipient */
    socklen_t fromlen;       /**< Length of `from` */
    int fd;                  /**< Server file descriptor */
};

/** Server callback function type */
typedef void (*server_request_handler_t)(struct server_conn *, const char *);

/**
 * Setup signal handler to stop gracefully
 *
 * Handles `SIGINT`, `SIGQUIT`, `SIGTERM`
 */
void server_setup_sighandler(void);

/**
 * Run the server loop
 *
 * @param fd Already provisioned file descriptor, used for socket activation.
 *           Pass `-1` if want to create socket manually.
 * @param sockpath Path to UNIX socket (if `ready_fd` is `-1`)
 * @param buf_len Command buffer length
 * @param request_handler Client request handler callback
 * @return 0 on success, -1 otherwise
 */
int server_listen(int fd, const char *sockpath, int buf_len,
                  server_request_handler_t request_handler);

/**
 * Reply to recipient with message
 *
 * @param conn Server connection structure, see ::server_conn
 * @param buf Buffer with response message
 * @param len Response buffer length
 * @return 0 on success, -1 otherwise
 */
int server_reply(struct server_conn *conn, const char *buf, int len);

#endif
