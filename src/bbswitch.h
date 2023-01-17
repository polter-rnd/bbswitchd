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

/**
 * @file bbswitch.h
 *
 * Functions for manipulating bbswitch status
 */

#ifndef BBSWITCH_H
#define BBSWITCH_H

#include <stddef.h>

#include "pci.h"


/** State of dedicated video card */
enum switch_state {
    SWITCH_ON = 1,      /**< Card is turned ON */
    SWITCH_OFF = 0,     /**< Card is turned OFF */
    SWITCH_UNAVAIL = -1 /**< State is not available */
};

/**
 * Reports the status of bbswitch
 *
 * @param bus_id Where to write bus_id of bbswtich-able card or `NULL` if not needed
 * @return #SWITCH_OFF if card is off, #SWITCH_ON if card is on
 *         and #SWITCH_UNAVAIL if bbswitch not available
 */
enum switch_state bbswitch_status(struct pci_bus_id *bus_id);

/**
 * Whether bbswitch is available for use
 *
 * @return 1 if available for use for PM, 0 otherwise
 */
int bbswitch_is_available(void);

/**
 * Turns card on if not already on
 */
void bbswitch_on(void);

/**
 * Turns card off if not already off
 */
void bbswitch_off(void);

#endif
