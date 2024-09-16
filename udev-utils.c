/*
 * Copyright (c) 2015, 2021 Vladimir Kondratyev <vladimir@kondratyev.su>
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYSCTLBYNAME
#include <sys/sysctl.h>
#endif

#include <assert.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "udev-global.h"

struct devnum_scan_args {
	dev_t	devnum;
	char *	pattern;
	char *	path;
	size_t	len;
};

struct subsystem_config {
	char *subsystem;
	char *devtype;
	char *syspath;
	char *symlink; /* If syspath is symlink, path it refers to */
	int flags; /* See SCFLAG_* below. */
	create_node_handler_t *create_handler;
};

/* Flag which in indicates a device should be skipped because it's
 * already exposed through EVDEV when it's enabled. */
#define	SCFLAG_SKIP_IF_EVDEV	0x01

static const struct subsystem_config subsystems[] = {
	{
#if defined (HAVE_LINUX_INPUT_H) || defined (HAVE_DEV_EVDEV_INPUT_H)
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/input/event[0-9]*",
		.create_handler = create_evdev_handler
	}, {
#endif
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/ukbd[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_keyboard_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/atkbd[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_keyboard_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/kbdmux[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_kbdmux_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/ums[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_mouse_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/psm[0-9]*",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_mouse_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/joy[0-9]*",
		.create_handler = create_joystick_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/atp[0-9]*",
		.create_handler = create_touchpad_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/wsp[0-9]*",
		.create_handler = create_touchpad_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/uep[0-9]*",
		.create_handler = create_touchscreen_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/sysmouse",
		.flags = SCFLAG_SKIP_IF_EVDEV,
		.create_handler = create_sysmouse_handler,
	}, {
		.subsystem = "input",
		.syspath = DEV_PATH_ROOT "/vboxguest",
		.create_handler = create_mouse_handler,
	}, {
		.subsystem = "drm",
		.syspath = DEV_PATH_ROOT "/dri/card[0-9]*",
		.symlink = DEV_PATH_ROOT "/drm/[0-9]*",
		.create_handler = create_drm_handler,
	}, {
		.subsystem = "net",
		.syspath = "/net/*",
		.create_handler = create_net_handler,
	}, {
	},
#ifdef HAVE_DEV_HID_HIDRAW_H
	{
		.subsystem = "hidraw",
		.syspath = DEV_PATH_ROOT "/hidraw[0-9]*",
		.create_handler = create_hidraw_handler,
	},
#endif
};

static const struct subsystem_config *
get_subsystem_config_by_syspath(const char *path)
{
	size_t i;

	for (i = 0; i < nitems(subsystems); i++)
		if (fnmatch(subsystems[i].syspath, path, 0) == 0)
			return (&subsystems[i]);

	return (NULL);
}

static bool
kernel_has_evdev_enabled()
{
	static int enabled = -1;
#ifdef HAVE_SYSCTLBYNAME
	size_t len;
#endif

	if (enabled != -1)
		return (enabled);

#ifdef HAVE_SYSCTLBYNAME
	if (sysctlbyname("kern.features.evdev_support", &enabled, &len, NULL, 0) < 0)
		return (0);
#else
	enabled = 1;
#endif

	TRC("() EVDEV enabled: %s", enabled ? "true" : "false");
	return (enabled);
}

const char *
get_subsystem_by_syspath(const char *syspath, const char **devtype)
{
	const struct subsystem_config *sc;

	sc = get_subsystem_config_by_syspath(syspath);
	if (sc == NULL)
		return (UNKNOWN_SUBSYSTEM);
	if (sc->flags & SCFLAG_SKIP_IF_EVDEV && kernel_has_evdev_enabled()) {
		TRC("(%s) EVDEV enabled -> skipping device", syspath);
		return (UNKNOWN_SUBSYSTEM);
	}

	if (devtype != NULL)
		*devtype = sc->devtype;
	return (sc->subsystem);
}

const char *
get_sysname_by_syspath(const char *syspath)
{

	return (strbase(syspath));
}

const char *
get_devpath_by_syspath(const char *syspath)
{
	if (strncmp(syspath, DEV_PATH_ROOT "/", 5) == 0)
		return (syspath);
	else
		return (strbase(syspath));
}

const char *
get_syspath_by_devpath(const char *devpath)
{

	return (devpath);
}

static int
get_syspath_by_devnum_cb(const char *path, mode_t type, void *args)
{
	struct devnum_scan_args *sa = args;
	struct stat st;

	if (S_ISLNK(type) &&
	    fnmatch(sa->pattern, path, 0) == 0 &&
	    stat(path, &st) == 0 &&
	    st.ST_RDEV == sa->devnum) {
		strlcpy(sa->path, path, sa->len);
		return (-1);
	}
	return (0);
}

const char *
get_syspath_by_devnum(dev_t devnum)
{
	char devpath[DEV_PATH_MAX] = DEV_PATH_ROOT "/";
	char linkdir[DEV_PATH_MAX];
	struct stat st;
	struct scandir_ctx ctx;
	struct devnum_scan_args args;
	const char *linkbase;
	size_t dev_len, linkdir_len, i;

	dev_len = strlen(devpath);
	devname_r(devnum, S_IFCHR, devpath + dev_len, sizeof(devpath) - dev_len);
	/* Recheck path as devname_r returns zero-terminated garbage on error */
	if (stat(devpath, &st) != 0 || st.ST_RDEV != devnum) {
		TRC("(%d) -> failed", (int)devnum);
		return NULL;
	}
	TRC("(%d) -> %s", (int)devnum, devpath);

	/* Resolve symlink in reverse direction if necessary */
	for (i = 0; i < nitems(subsystems); i++) {
		if (subsystems[i].symlink != NULL &&
		    fnmatch(subsystems[i].symlink, devpath, 0) == 0) {
			linkbase = strbase(subsystems[i].syspath);
			assert(linkbase != NULL);
			linkdir_len = linkbase - subsystems[i].syspath;
			if (linkdir_len >= sizeof(linkdir))
				linkdir_len = sizeof(linkdir);
			strlcpy(linkdir, subsystems[i].syspath, linkdir_len + 1);
			args = (struct devnum_scan_args) {
				.devnum = devnum,
				.pattern = subsystems[i].syspath,
				.path = devpath,
				.len = sizeof(devpath),
			};
			ctx = (struct scandir_ctx) {
				.recursive = false,
				.cb = get_syspath_by_devnum_cb,
				.args = &args,
			};
			if (scandir_recursive(linkdir, sizeof(linkdir), &ctx) == -1)
				break;
		}
	}

	return (strdup(devpath));
}

void
invoke_create_handler(struct udev_device *ud)
{
	const char *path;
	const struct subsystem_config *sc;

	path = udev_device_get_syspath(ud);
	sc = get_subsystem_config_by_syspath(path);
	if (sc == NULL || sc->create_handler == NULL)
		return;
	if (sc->flags & SCFLAG_SKIP_IF_EVDEV && kernel_has_evdev_enabled()) {
		TRC("(%p) EVDEV enabled -> skipping device", ud);
		return;
	}

	sc->create_handler(ud);
}

size_t
syspathlen_wo_units(const char *path) {
	size_t len;

	len = strlen(path);
	while (len > 0) {
		if (path[len-1] < '0' || path[len-1] > '9')
			break;
		--len;
	}
	return len;
}

LIBUDEV_EXPORT int
udev_util_encode_string(const char *str, char *str_enc, size_t len)
{
#ifdef ENABLE_GPL
	return (encode_devnode_name(str, str_enc, len));
#else
	return (strlcpy(str_enc, str, len) < len ? 0 : -EINVAL);
#endif
}
