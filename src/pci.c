/*
 * Copyright (c) 2022-2023, Pavel Artsishevsky
 * Copyright (c) 2011-2013, The Bumblebee Project
 * Author: Pavel Artsishevsky <polter.rnd@gmail.com>
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "pci.h"


int pci_bus_id_parse(struct pci_bus_id *dest, const char *buf) {
    unsigned int bus, func, slot;
    if (sscanf(buf, "%*x:%02x:%02x.%o", &bus, &func, &slot) != 3) {
        return -1;
    }

    /* All variables read means parsing succeeded */
    dest->bus = bus;
    dest->func = func;
    dest->slot = slot;
    return 0;
}


size_t pci_get_driver(char *dest, const struct pci_bus_id *bus_id, size_t len) {
    char path[1024], resolved_path[1024];
    ssize_t read_bytes;
    char *name;

    /* if the bus_id was invalid */
    if (!bus_id) {
        return 0;
    }

    /* the path to the driver if one is loaded */
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/0000:%02x:%02x.%o/driver",
             bus_id->bus, bus_id->slot, bus_id->func);
    read_bytes = readlink(path, resolved_path, sizeof(resolved_path) - 1);
    if (read_bytes < 0) {
        /* error, assume that the driver is not loaded */
        return 0;
    }

    /* readlink does not append a NULL according to the manpage */
    resolved_path[read_bytes] = 0;

    name = basename(resolved_path);
    /* save the name if a valid destination and buffer size was given */
    if (dest && len > 0) {
        strncpy(dest, name, len - 1);
        dest[len - 1] = 0;
    }

    return strlen(name);
}
