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

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "logger.h"
#include "server.h"


/* Flag to be set by signal handler below */
static int S_server_stopped = 0;

static void handle_signal(int signum, siginfo_t *siginfo, void *ignore) {
    (void) siginfo;
    (void) ignore;

    switch (signum) {
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
        log_info("Received signal %d (%s), stopping server.\n", signum, strsignal(signum));
        S_server_stopped = 1;
        break;
    default:
        break;
    }
}

void server_setup_sighandler(void) {
    struct sigaction sa = {0};
    /* Set exit handler function */
    sa.sa_sigaction = handle_signal;
    sigaction(SIGINT,  &sa, 0);
    sigaction(SIGQUIT, &sa, 0);
    sigaction(SIGTERM, &sa, 0);
}

int server_listen(int fd, const char *sockpath, int buf_len,
                  server_request_handler_t request_handler) {
    struct server_conn conn;
    char buf[buf_len];
    ssize_t len;

    /* If fd if not initialized yet, create and bind socket ourselves.
     * Othwewise just use existing one, e.g. in case of xinetd or systemd-socket. */
    if (fd == -1) {
        struct sockaddr_un addr = { 0 };

        if ((conn.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
            log_err("Error creating socket, errno %d: %s\n", errno, strerror(errno));
            return -1;
        }

        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, sockpath);
        unlink(sockpath);
        if (bind(conn.fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            log_err("Error binding socket %s, errno %d: %s\n",
                    sockpath, errno, strerror(errno));
            close(conn.fd);
            return -1;
        }
    } else {
        conn.fd = fd;
    }

    /* Serve until stopped or zero bytes received (EOF) */
    S_server_stopped = 0;
    do {
        conn.fromlen = sizeof(conn.from);
        len = recvfrom(conn.fd, buf, sizeof(buf), 0, (struct sockaddr *) &conn.from, &conn.fromlen);
        if (len > 0) {
            buf[len - 1] = '\0';
            request_handler(&conn, buf);
        } else if (len < 0) {
            if (errno == EINTR) {
                /* Interrupted by signal, continue serving if we were not stopped */
                continue;
            } else {
                log_err("Error serving socket, errno %d: %s\n", errno, strerror(errno));
                break;
            }
        }
    } while (!S_server_stopped);

    if (conn.fd != fd)
        close(conn.fd);

    return (S_server_stopped || len == 0) ? 0 : -1;
}

int server_reply(struct server_conn *conn, const char *buf, int len) {
    if (sendto(conn->fd, buf, len, 0, (struct sockaddr *) &conn->from, conn->fromlen) < 0) {
        log_err("Error replying to client, errno %d: %s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}
