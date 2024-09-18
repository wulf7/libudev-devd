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

#include <sys/types.h>
#ifdef HAVE_DEVINFO_H
#include <sys/pciio.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_DEVINFO_H
#include <devinfo.h>
#endif

#include "udev-global.h"

#define _PATH_DEVPCI	"/dev/pci"

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
#ifdef HAVE_DEVINFO_H
	struct pci_conf_io pc;
	struct pci_conf conf;
	struct pci_match_conf patterns;
	const char *dbsf;
	struct udev_list *props, *attrs;
	unsigned int dom, bus, slot, func, class;
	int fd;

	dbsf = _udev_device_get_sysname(ud);
	if (sscanf(dbsf, "%x:%x:%x.%x", &dom, &bus, &slot, &func) != 4) {
		ERR("Invalid syspath dbsf value: %s", dbsf);
		return;
	}

	bzero(&pc, sizeof(struct pci_conf_io));
	pc.match_buf_len = sizeof(conf);
	pc.matches = &conf;
	pc.num_patterns = 1;
	pc.pat_buf_len = sizeof(patterns);
	pc.patterns = &patterns;

	bzero(&patterns, sizeof(patterns));
	patterns.pc_sel.pc_domain = dom;
	patterns.pc_sel.pc_bus = bus;
	patterns.pc_sel.pc_dev = slot;
	patterns.pc_sel.pc_func = func;
	patterns.flags = PCI_GETCONF_MATCH_DOMAIN |
	    PCI_GETCONF_MATCH_BUS | PCI_GETCONF_MATCH_DEV |
	    PCI_GETCONF_MATCH_FUNC;

	fd = open(_PATH_DEVPCI, O_RDONLY | O_CLOEXEC, 0);
	if (fd < 0) {
		ERR("Failed to open %s", _PATH_DEVPCI);
		return;
	}

	if (ioctl(fd, PCIOCGETCONF, &pc) == -1) {
		ERR("Failed to ioctl(PCIOCGETCONF)");
		goto out;
	}

	if (pc.status != PCI_GETCONF_LAST_DEVICE) {
		ERR("Bad ioctl(PCIOCGETCONF) status: %d", pc.status);
		goto out;
	}

	class = (conf.pc_class << 16)|(conf.pc_subclass << 8)|conf.pc_progif;

	props = udev_device_get_properties_list(ud);
	attrs = udev_device_get_sysattr_list(ud);

	udev_list_insertf(props, "PCI_CLASS", "%x", class);
	udev_list_insertf(props, "PCI_ID",
	    "%04x:%04x", conf.pc_vendor, conf.pc_device);
	udev_list_insertf(props, "PCI_SUBSYS_ID",
	    "%04x:%04x", conf.pc_subvendor, conf.pc_subdevice);
	udev_list_insertf(props, "PCI_SLOT_NAME",
	    "%04x:%02x:%02x.%01x", dom, bus, slot, func);
	udev_list_insertf(props, "ID_PATH",
	    "pci-%04x:%02x:%02x.%01x", dom, bus, slot, func);
	udev_list_insertf(props, "ID_PATH_TAG",
	    "pci-%04x_%02x_%02x_%01x", dom, bus, slot, func);

	udev_list_insertf(attrs, "class", "0x%06x", class);
	udev_list_insertf(attrs, "vendor", "0x%04x", conf.pc_vendor);
	udev_list_insertf(attrs, "device", "0x%04x", conf.pc_device);
	udev_list_insertf(attrs, "subsystem_vendor",
	   "0x%04x", conf.pc_subvendor);
	udev_list_insertf(attrs, "subsystem_device",
	   "0x%04x", conf.pc_subdevice);
	udev_list_insertf(attrs, "revision", "0x%02x", conf.pc_revid);
	udev_list_insert(attrs, "numa_node", "-1");
	udev_list_insertf(attrs, "uevent",
	    "PCI_CLASS=%x\n"
	    "PCI_ID=%04x:%04x\n"
	    "PCI_SUBSYS_ID=%04x:%04x\n"
	    "PCI_SLOT_NAME=%04x:%02x:%02x.%01x",
	    class,
	    conf.pc_vendor, conf.pc_device,
	    conf.pc_subvendor, conf.pc_subdevice,
	    dom, bus, slot, func);

out:
	close(fd);
#endif
}
