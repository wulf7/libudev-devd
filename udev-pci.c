/*
 * Copyright (c) 2024 Future Crew LLC
 *
 * Author: Vladimir Kondratyev <vladimir@kondratyev.su>
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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_DEVINFO_H
#include <devinfo.h>
#endif

#include "udev-global.h"

#ifdef HAVE_DEVINFO_H
static bool
devd2udev_dbsf(const char *msg, char *syspath, size_t syspathlen)
{
	const char *dbsf;
	unsigned int dom, bus, slot, func;

	dbsf = get_kern_prop_value(msg, "dbsf", NULL);
	if (dbsf == NULL)
		return (false);
	if (sscanf(dbsf, "pci%u:%u:%u:%u", &dom, &bus, &slot, &func) != 4) {
		ERR("Invalid dbsf value: %s", dbsf);
		return (false);
	}

	if (snprintf(syspath, syspathlen,
	    "%04x:%02x:%02x.%01x", dom, bus, slot, func) >= syspathlen) {
		ERR("dbsf %s overflowed %zu butes", dbsf, syspathlen);
		return (false);
	}

	return (true);
}

static int
udev_pci_enumerate_cb(struct devinfo_dev *dev, void *arg)
{
	char syspath[DEV_PATH_MAX] = "/pci/";
	struct udev_enumerate *ue = arg;

	if (!devd2udev_dbsf(dev->dd_location, syspath + 5, sizeof(syspath) - 5))
		return (0);

	return (udev_enumerate_add_device(ue, syspath));
}
#endif

int
udev_pci_enumerate(struct udev_enumerate *ue)
{
#ifdef HAVE_DEVINFO_H
	struct scandev_ctx ctx = {
		.cb = udev_pci_enumerate_cb,
		.args = ue,
	};

	return (scandev_recursive(&ctx));
#else
	return (0);
#endif
}

int
udev_pci_monitor(char *msg, char *syspath, size_t syspathlen)
{
        int action  = UD_ACTION_NONE;

#ifdef HAVE_DEVINFO_H
	switch (msg[0]) {
	case DEVD_EVENT_ATTACH:
		action = UD_ACTION_ADD;
		break;
	case DEVD_EVENT_DETACH:
		action = UD_ACTION_REMOVE;
		break;
	default:
		return (UD_ACTION_NONE);
	}

	if (syspathlen <= 5)
		return (UD_ACTION_NONE);
	if (!devd2udev_dbsf(msg + 1, syspath + 5, syspathlen - 5))
		return (UD_ACTION_NONE);
	memcpy(syspath, "/pci/", 5);
#endif /* HAVE_DEVINFO_H */

	return (action);
}

void
create_pci_handler(struct udev_device *ud)
{
}
