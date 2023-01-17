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

/**
 * @file pci.h
 *
 * Functions for working with PCI devices
 */

#ifndef PCI_H
#define PCI_H

/* For size_t type */
#include <stddef.h>

/** Structure to hold PCI bus ID */
struct pci_bus_id {
    unsigned char bus;  /**< 0x00 - 0xFF */
    unsigned char slot; /**< 0x00 - 0x1F */
    unsigned char func; /**< 0 - 7 */
};

/**
 * @brief Fill PCI bus ID structure from string
 *
 * @param dest Destination #pci_bus_id struct where result will be written
 * @param buf Input string buffer of format like `0000:%00:%00.0`
 * @return 0 on success, -1 otherwise
 */
int pci_bus_id_parse(struct pci_bus_id *dest, const char *buf);

/**
 * Gets the driver name for a given Bus ID. If dest is not null and len is
 * larger than 0, the driver name will be stored in dest
 *
 * @param bus_id A #pci_bus_id struct containing a Bus ID
 * @param dest An optional buffer to store the found driver name in
 * @param len The maximum number of bytes to store in dest
 * @return The length of the driver name (which may be larger than @p len if the
 *         buffer was too small) or 0 on error
 */
size_t pci_get_driver(char *dest, const struct pci_bus_id *bus_id, size_t len);

#endif