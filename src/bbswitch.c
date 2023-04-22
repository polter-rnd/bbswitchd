/*
 * Copyright (c) 2022-2023, Pavel Artsishevsky
 * Copyright (c) 2011-2013, The Bumblebee Project
 * Author: Pavel Artsishevsky <polter.rnd@gmail.com>
 * Author: Jaron Viëtor AKA "Thulinma" <jaron@vietors.com>
 * Author: Peter Lekensteyn <lekensteyn@gmail.com>
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "logger.h"
#include "bbswitch.h"
#include "module.h"
#include "pci.h"

/* Patch to bbswitch state file */
#define BBSWITCH_PATH "/proc/acpi/bbswitch"


enum switch_state bbswitch_status(struct pci_bus_id *bus_id) {
    enum switch_state ret = SWITCH_UNAVAIL;
    char bus[sizeof("0000:00:00.0")];
    char state[sizeof("OFF")];
    FILE *bbs = fopen(BBSWITCH_PATH, "r");

    if (bbs != NULL) {
        char format[32];
        snprintf(format, sizeof(format), "%%%lus %%%lus",
                sizeof(bus) - 1, sizeof(state) - 1);
        if (fscanf(bbs, format, bus, state) != -1) {
            if (bus_id && pci_bus_id_parse(bus_id, bus) != 0) {
                log_err("Unable to parse PCI bus id '%s'\n", bus);
            } else if (!strcmp(state, "OFF")) {
                ret = SWITCH_OFF;
            } else if (!strcmp(state, "ON")) {
                ret = SWITCH_ON;
            }
        } else {
            log_err("Unable to parse %s\n", BBSWITCH_PATH);
        }
        fclose(bbs);
    } else {
        log_err("Could not open %s: %s\n", BBSWITCH_PATH, strerror(errno));
    }

    return ret;
}

static void bbswitch_write(char *msg) {
    FILE *bbs = fopen(BBSWITCH_PATH, "w");
    if (bbs == 0) {
        log_err("Could not open %s: %s\n", BBSWITCH_PATH, strerror(errno));
        return;
    }
    fputs(msg, bbs);
    if (ferror(bbs)) {
        log_warn("Could not write to %s: %s\n", BBSWITCH_PATH,
                 strerror(errno));
    }
    fclose(bbs);
}

int bbswitch_is_available(void) {
    /* easy one: if the path is available, bbswitch is usable */
    if (access(BBSWITCH_PATH, F_OK | R_OK | W_OK) == 0) {
        /* file exists and read/write is allowed */
        log_debug("bbswitch has been detected.\n");
        return 1;
    }
    /* module is not loaded yet. Try to load it, checking whether the device is
     * recognized by bbswitch. Assuming that vga_switcheroo was not told to OFF
     * the device */
    if (module_load("bbswitch", "bbswitch") == 0) {
        log_debug("Successfully loaded bbswitch\n");
        /* hurrah, bbswitch could be loaded which means that the module is
         * available and that the card is supported */
        return 1;
    }
    /* nope, we can't use bbswitch */
    log_debug("bbswitch is not available, perhaps you need to insmod it?\n");
    return 0;
}

void bbswitch_on(void) {
    bbswitch_write("ON\n");
}

void bbswitch_off(void) {
    bbswitch_write("OFF\n");
}
