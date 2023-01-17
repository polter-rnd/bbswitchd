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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include "logger.h"
#include "module.h"
#include "bbswitch.h"
#include "pci.h"
#include "server.h"

#define DAEMON_NAME   "bbswitchd"
#define NVIDIA_DRIVER "nvidia"
#define NVIDIA_MODULE "nvidia-drm"
#define SERVER_SOCK   "/var/run/bbswitchd.sock"


/* Default logging parameters */
static int   S_log_type = LOGGER_STDERR;
static char *S_log_name = DAEMON_NAME;

/* Default socket file descriptor and path*/
static int   S_sockfd = -1;
static char *S_sockpath = NULL;

/* Default NVIDIA kernel module */
static char *S_module = NVIDIA_MODULE;

/* Timeout for unloading modules */
static const int S_unload_retries = 5;
static const int S_unload_retry_timeout = 1;


/**
 * Print out usage information.
 */
static void usage(void) {
	fprintf(stderr, "Usage: bbswitchd [ -v ] [ -l log ] [ -m module ] [ -f fd ] [ -s sock ]\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -v,--verbose  Display debug messages. Takes no arguments.\n");
	fprintf(stderr, "  -l,--log      Logging engine.\n");
    fprintf(stderr, "                Valid arguments: syslog, stderr, file=/path/to/logfile\n");
    fprintf(stderr, "                Default: stderr\n");
	fprintf(stderr, "  -m,--module   Kernel module to load/unload after GPU power state switch.\n");
    fprintf(stderr, "                Default: nvidia-drm\n");
	fprintf(stderr, "  -f,--fd       Ready to use file descriptor to listen on.\n");
    fprintf(stderr, "                Use when starting by inetd or systemd socket activation.\n");
    fprintf(stderr, "                Do not use together with --sock option.\n");
	fprintf(stderr, "  -s,--sock     Path to UNIX socket that will be created.\n");
    fprintf(stderr, "                Do not use together with --fd option.\n");
	fprintf(stderr, "  -h,--help     Display this message.\n");
}

/**
 * Parse command-line arguments.
 */
static int parse_arguments(int argc, char *argv[]) {
    int c = 0, option_index = 0;
    static struct option long_options[] = {
        { "verbose", no_argument,       0, 'v' },
        { "log",     required_argument, 0, 'l' },
        { "module",  required_argument, 0, 'm' },
        { "fd",      required_argument, 0, 'f' },
        { "sock",    required_argument, 0, 's' },
        { "help",    required_argument, 0, 'h' },
        { 0, 0, 0, 0 }
    };

    while ((c = getopt_long(argc, argv, "vhl:m:", long_options, &option_index)) != -1) {
        switch (c) {
        case 'v':
            log_set_verbosity(VERBOSITY_DEBUG);
            break;

        case 'l':
            if (!strcmp(optarg, "syslog")) {
                S_log_type = LOGGER_SYSLOG;
            } else if (!strcmp(optarg, "stderr")) {
                S_log_type = LOGGER_STDERR;
            } else if (!strncmp(optarg, "file=", sizeof("file=") - 1)) {
                S_log_type = LOGGER_FILE;
                S_log_name = optarg + sizeof("file=") - 1;
            } else {
                fprintf(stderr, "Unknown log type '%s'\n", optarg);
                return -1;
            }
            break;

        case 'm':
            S_module = optarg;
            break;

        case 'f':
            if (S_sockpath != NULL) {
                fprintf(stderr, "Unable to use both '--fd' and '--sock' options\n");
                return -1;
            }
            S_sockfd = atol(optarg);
            break;

        case 's':
            if (S_sockfd != -1) {
                fprintf(stderr, "Unable to use both '--sock' and '--fd' options\n");
                return -1;
            }
            S_sockpath = optarg;
            break;

        case 'h':
        default:
            usage();
            return -1;
        }
    }

    /* If nothing set, use default sock path */
    if (S_sockpath == NULL && S_sockfd == -1) {
        S_sockpath = SERVER_SOCK;
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        usage();
        return -1;
    }

    return 0;
}

/**
 * Load the kernel module, powering on the card beforehand
 */
static int switch_and_load(const char **errmsg)
{
    struct pci_bus_id bus_id;
    char driver[sizeof(NVIDIA_DRIVER)];

    bbswitch_on();
    if (bbswitch_status(&bus_id) == SWITCH_ON) {
        log_err("Discrete graphics card enabled\n");
    } else {
        *errmsg = "Could not enable discrete graphics card";
        log_err("%s\n", *errmsg);
        return -1;
    }

    if (pci_get_driver(driver, &bus_id, sizeof(driver))) {
        /* if the loaded driver does not equal the driver from config, unload it */
        if (strcasecmp(NVIDIA_DRIVER, driver)) {
            if (!module_unload(driver, S_unload_retries, S_unload_retry_timeout)) {
                /* driver failed to unload, aborting */
                return -1;
            }
        }
    }

    if (module_load(S_module, NVIDIA_DRIVER) != 0) {
        *errmsg = "Could not load GPU driver";
        log_err("%s\n", *errmsg);
        return -1;
    }

    return 0;
}

/**
 * Unload the kernel module and power down the card
 */
static int switch_and_unload(const char **errmsg)
{
    struct pci_bus_id bus_id;
    char driver[sizeof(NVIDIA_DRIVER)];

    if (bbswitch_status(&bus_id) != SWITCH_ON) {
        log_info("Discrete graphics card is already disabled\n");
        return 0;
    }

    if (pci_get_driver(driver, &bus_id, sizeof(driver))) {
        /* If driver is loaded, unload it */
        if (module_unload(driver, S_unload_retries, S_unload_retry_timeout) != 0) {
            *errmsg = "Could not unload GPU driver";
            log_err("%s\n", *errmsg);
            return -1;
        }
    }

    bbswitch_off();
    if (bbswitch_status(NULL) == SWITCH_OFF) {
        log_info("Discrete graphics card disabled\n");
    } else {
        *errmsg = "Could not disable discrete graphics card";
        log_err("%s\n", *errmsg);
        return -1;
    }

    return 0;
}

/**
 * Handle client commands
 */
void request_handler(struct server_conn *conn, const char *command) {
    const char *errmsg = NULL;
    if (!strcmp(command, "on")) {
        switch_and_load(&errmsg);
    } else if (!strcmp(command, "off")) {
        switch_and_unload(&errmsg);
    } else {
        errmsg = "Ignoring unknown command";
        log_warn("%s '%s'\n", errmsg, command);
    }

    if (errmsg == NULL) {
        /* Empty response means success */
        server_reply(conn, "", 1);
    } else {
        /* Otherwise send error message */
        server_reply(conn, errmsg, strlen(errmsg) + 1);
    }
}


int main(int argc, char *argv[]) {
    if (parse_arguments(argc, argv) != 0) {
        return EXIT_FAILURE;
    }

    if (log_open(S_log_type, S_log_name) != 0) {
        fprintf(stderr, "Failed to open log (errno %d): %s\n",
                errno, strerror(errno));
        return EXIT_FAILURE;
    }

    if (module_ctx_init() != 0) {
        log_err("Failed to initialize kmod context (errno %d): %s\n",
                errno, strerror(errno));
    } else {
        if (bbswitch_is_available()) {
            server_setup_sighandler();
            /* 4 is maximum command length since we accept only `on` or `off`. */
            if (server_listen(S_sockfd, S_sockpath, 4, request_handler) != 0) {
                log_err("Aborting server.\n");
            } else {
                log_close();
                return EXIT_SUCCESS;
            }
        } else {
            log_err("No switching method available, aborting\n");
        }
        module_ctx_release();
    }

    log_close();
    return EXIT_FAILURE;
}