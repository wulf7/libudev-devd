/*
 * Copyright (c) 2021 Vladimir Kondratyev <vladimir@kondratyev.su>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#include "libudev.h"
#include "udev-utils.h"
#include "utils.h"
#ifdef ENABLE_GPL
#include "udev-list.h"
#include "utils-hwdb.h"
#endif

#ifdef ENABLE_GPL
struct udev_hwdb {
	int refcount;
	struct hwdb *hwdb;
	struct udev_list prop_list;
};

static int
udev_hwdb_get_properties_list_entry_cb(void *ctx, const char *key,
    const char *value)
{
	return (udev_list_insert(ctx, key, value) == -1 ? ENOMEM : 0);
}
#endif

LIBUDEV_EXPORT struct udev_hwdb *
udev_hwdb_new(struct udev *udev)
{
	struct udev_hwdb *uh = NULL;

	TRC("(%p)", udev);
#ifdef ENABLE_GPL
	uh = calloc(1, sizeof(struct udev_hwdb));
	if (uh != NULL) {
		uh->refcount = 1;
		uh->hwdb = libhwdb_open(HWDB_PATH);
		if (uh->hwdb == NULL) {
			free(uh);
			uh = NULL;
		} else
			udev_list_init(&uh->prop_list);
	}
#else
	UNIMPL();
#endif
	return (uh);
}

LIBUDEV_EXPORT struct udev_hwdb *
udev_hwdb_ref(struct udev_hwdb *uh)
{
	TRC("(%p)", uh);
#ifdef ENABLE_GPL
	if (uh != NULL)
		++uh->refcount;
#endif
	return (uh);
}

LIBUDEV_EXPORT struct udev_hwdb *
udev_hwdb_unref(struct udev_hwdb *uh)
{
	TRC("(%p)", uh);
#ifdef ENABLE_GPL
	if (uh != NULL && --uh->refcount == 0) {
		libhwdb_close(uh->hwdb);
		udev_list_free(&uh->prop_list);
		free(uh);
	}
#endif
	return (NULL);
}

LIBUDEV_EXPORT struct udev_list_entry *
udev_hwdb_get_properties_list_entry(struct udev_hwdb *uh, const char *modalias,
    unsigned int flags)
{
	TRC("(%p, %s, %u)", uh, modalias, flags);
#ifdef ENABLE_GPL
	udev_list_free(&uh->prop_list);
	int err = libhwdb_get_properties_list_entry(
	    udev_hwdb_get_properties_list_entry_cb, &uh->prop_list, uh->hwdb,
	    modalias);
	if (err != 0) {
		errno = err;
		return (NULL);
	}
	return (udev_list_entry_get_first(&uh->prop_list));
#else
	UNIMPL();
	errno = EINVAL;
	return (NULL);
#endif
}
