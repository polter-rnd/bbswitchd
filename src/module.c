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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <libkmod.h>

#include "module.h"
#include "logger.h"

static struct kmod_ctx *S_kmod_ctx = NULL;


int module_ctx_init(void) {
    S_kmod_ctx = kmod_new(NULL, NULL);
    return S_kmod_ctx ? 0 : -1;
}

int module_is_loaded(const char *driver) {
    int err, state;
    struct kmod_module *mod;

    err = kmod_module_new_from_name(S_kmod_ctx, driver, &mod);
    if (err < 0) {
        log_debug("kmod_module_new_from_name(%s) failed (err: %d).\n", driver, err);
        return 0;
    }

    state = kmod_module_get_initstate(mod);
    kmod_module_unref(mod);

    return state == KMOD_MODULE_LIVE;
}

int module_load(const char *module_name, const char *driver) {
    int err = 0;
    int flags = KMOD_PROBE_IGNORE_LOADED;
    struct kmod_list *l, *list = NULL;

    if (module_is_loaded(module_name) == 0) {
        /* the module has not loaded yet, try to load it */

        log_info("Loading driver '%s' (module '%s')\n", driver, module_name);
        err = kmod_module_new_from_lookup(S_kmod_ctx, module_name, &list);

        if (err < 0) {
            log_debug("kmod_module_new_from_lookup(%s) failed (err: %d).\n",
                      module_name, err);
            return -1;
        }

        if (list == NULL) {
            log_err("Module '%s' not found.\n", module_name);
            return -1;
        }

        kmod_list_foreach(l, list) {
            struct kmod_module *mod = kmod_module_get_module(l);

            log_debug("Loading module '%s'.\n", kmod_module_get_name(mod));
            err = kmod_module_probe_insert_module(mod, flags, NULL, NULL, NULL, 0);

            if (err < 0) {
                log_debug("kmod_module_probe_insert_module(%s) failed (err: %d).\n",
                          kmod_module_get_name(mod), err);
            }

            kmod_module_unref(mod);

            if (err < 0) {
                break;
            }
        }

        kmod_module_unref_list(list);
    }

    if (err != 0) {
        log_err("Failed to load module '%s' (errno %d): %s\n",
                module_name, errno, strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Unloads module and modules that are depending on this module.
 *
 * @param mod Reference to libkmod module
 * @return 0 if the module is succesfully unloaded, -1 otherwise
 */
static int module_unload_recursive(struct kmod_module *mod, int retries, int timeout) {
    int err = 0, flags = 0, refcnt = 0;
    struct kmod_list *holders;

    holders = kmod_module_get_holders(mod);
    if (holders != NULL) {
        struct kmod_list *itr;

        kmod_list_foreach(itr, holders) {
            struct kmod_module *hm = kmod_module_get_module(itr);
            err = module_unload_recursive(hm, retries, timeout);
            kmod_module_unref(hm);

            if (err != 0) {
                break;
            }
        }
        kmod_module_unref_list(holders);
    }

    while (err == 0 && retries-- > 0) {
        refcnt = kmod_module_get_refcnt(mod);
        if (refcnt == 0) {
            log_info("Unloading module %s\n", kmod_module_get_name(mod));
            err = kmod_module_remove_module(mod, flags);
        } else if (refcnt != -ENOENT) {
            if (retries > 0) {
                log_info("Failed to unload module '%s' (ref count: %d). Retrying...\n",
                         kmod_module_get_name(mod), refcnt);
                sleep(timeout);
            } else {
                /* Stop retrying when attempt count exceeded */
                err = -1;
            }
        }
    }

    if (err != 0) {
        if (refcnt == 0) {
            if (errno == 0) {
                log_err("Failed to unload module '%s'\n",
                        kmod_module_get_name(mod));
            } else {
                log_err("Failed to unload module '%s' (errno %d): %s\n",
                        kmod_module_get_name(mod), refcnt, errno, strerror(errno));
            }
        } else {
            log_err("Failed to unload module '%s' (ref count: %d): Module still in use\n",
                    kmod_module_get_name(mod), refcnt);
        }
        return -1;
    }

    return 0;
}

int module_unload(const char *driver, int retries, int timeout) {
    int err;
    struct kmod_module *mod;
    if (module_is_loaded(driver) == 1) {
        err = kmod_module_new_from_name(S_kmod_ctx, driver, &mod);

        if (err < 0) {
            log_debug("kmod_module_new_from_name(%s) failed (err: %d).\n", driver,
                      err);
            return -1;
        }

        if (module_unload_recursive(mod, retries, timeout) < 0) {
            err = -1;
        }
        kmod_module_unref(mod);

        return err;
    }
    return 0;
}


int module_is_available(const char *module_name) {
    int err, available;
    struct kmod_list *list = NULL;

    err = kmod_module_new_from_lookup(S_kmod_ctx, module_name, &list);

    if (err < 0) {
        log_debug("kmod_module_new_from_lookup(%s) failed (err: %d).\n",
                  module_name, err);
        return 0;
    }

    available = list != NULL;

    kmod_module_unref_list(list);

    return available;
}

void module_ctx_release(void) {
    kmod_unref(S_kmod_ctx);
}
