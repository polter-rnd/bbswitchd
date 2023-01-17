/*
 * Copyright (c) 2022-2023, Pavel Artsishevsky
 * Copyright (c) 2011-2013, The Bumblebee Project
 * Author: Pavel Artsishevsky <polter.rnd@gmail.com>
 * Author: Joaquín Ignacio Aramendía <samsagax@gmail.com>
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
 * @file module.h
 *
 * Functions for working with kernel modules
 */

#ifndef MODULE_H
#define MODULE_H

/**
 * Initialize kernel module context
 *
 * @return 0 on success, -1 otherwise
 */
int module_ctx_init(void);

/**
 * Checks whether a kernel module is loaded
 *
 * @param driver The name of the driver (not a filename)
 * @return 1 if the module is loaded, 0 otherwise
 */
int module_is_loaded(const char *driver);

/**
 * Checks whether a kernel module is available for loading
 *
 * @param module_name The module name to be checked (filename or alias)
 * @return 1 if the module is available for loading, 0 otherwise
 */
int module_is_available(const char *module_name);

/**
 * Attempts to load a module.
 *
 * @param module_name The filename of the module to be loaded
 * @param driver The name of the driver to be loaded
 * @return 0 if the driver is successfully loaded, -1 otherwise
 */
int module_load(const char *module_name, const char *driver);

/**
 * Attempts to unload a module if loaded.
 *
 * @param driver The name of the driver (not a filename)
 * @param retries How many retries to do if module is in use
 * @param timeout Time to wait between retries, in seconds
 * @return 0 if the driver is successfully unloaded, -1 otherwise
 */
int module_unload(const char *driver, int retries, int timeout);

/**
 * Release kernel module context
 */
void module_ctx_release(void);

#endif
