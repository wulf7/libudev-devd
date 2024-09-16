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

#include "udev-global.h"

#ifdef HAVE_DEVINFO_H
static int
udev_sys_enumerate_cb(const char *path, mode_t type, void *arg)
{
	struct udev_enumerate *ue = arg;
	const char *syspath;

	if (S_ISLNK(type) || S_ISCHR(type)) {
		syspath = get_syspath_by_devpath(path);
		return (udev_enumerate_add_device(ue, syspath));
	}
	return (0);
}
#endif

int
udev_sys_enumerate(struct udev_enumerate *ue)
{
#ifdef HAVE_DEVINFO_H
	struct scan_ctx ctx = {
		.recursive = true,
		.cb = udev_sys_enumerate_cb,
		.args = ue,
	};

	return (scandev_recursive(&ctx));
#else
	return (0);
#endif
}

int
udev_sys_monitor(char *msg, char *syspath, size_t syspathlen)
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

	*(strchrnul(msg + 1, ' ')) = '\0';
	strlcpy(syspath, msg + 1, syspathlen);
#endif /* HAVE_DEVINFO_H */

	return (action);
}
