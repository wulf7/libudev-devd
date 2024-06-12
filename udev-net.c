/*
 * Copyright (c) 2024 Future Crew LLC
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

#include "utils.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <net/if.h>

#include <ifaddrs.h>
#include <string.h>

#include "libudev.h"

#include "udev-device.h"
#include "udev-list.h"
#include "udev-net.h"
#include "udev-utils.h"

int
udev_net_enumerate(struct scan_ctx *ctx)
{
	char syspath[IFNAMSIZ + 5] = "/net/";
	struct ifaddrs *ifap, *ifa;
	int ret = 0;

	if (getifaddrs(&ifap) != 0)
		return (-1);

	for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL ||
		    ifa->ifa_addr->sa_family != AF_LINK)
			continue;
		strlcpy(syspath + 5, ifa->ifa_name, IFNAMSIZ);
		if ((ret = (ctx->cb)(syspath, S_IFCHR, ctx->args)) < 0)
			break;
	}

	freeifaddrs(ifap);

	return (ret);
}

int
udev_net_monitor(char *msg, char *syspath, size_t syspathlen)
{
	char netpath[IFNAMSIZ + 5] = "/net/";
	const char *type, *dev_name;
	size_t type_len, dev_len;
	int action;

	if (msg[0] != DEVD_EVENT_NOTICE)
		return (UD_ACTION_NONE);

	msg++;
	if (!(match_kern_prop_value(msg, "system", "IFNET")))
		return (UD_ACTION_NONE);

	action = UD_ACTION_NONE;

	type = get_kern_prop_value(msg, "type", &type_len);
	dev_name = get_kern_prop_value(msg, "subsystem", &dev_len);
	if (type == NULL || dev_name == NULL ||
	    dev_len > (sizeof(netpath) - 5 - 1))
		return (UD_ACTION_NONE);

	if (type_len == 6 && strncmp(type, "ATTACH", type_len) == 0)
		action = UD_ACTION_ADD;
	else if (type_len == 6 && strncmp(type, "DETACH", type_len) == 0)
		action = UD_ACTION_REMOVE;
	else
		return (UD_ACTION_NONE);;

	memcpy(netpath + 5, dev_name, dev_len);
	netpath[dev_len + 5] = 0;
	strlcpy(syspath, netpath, syspathlen);

	return (action);
}

void
create_net_handler(struct udev_device *ud)
{
	char buf[80];
	const char *devpath;
	unsigned int ifindex;

	devpath = udev_device_get_devnode(ud);
	if (devpath == NULL)
		return;

	ifindex = if_nametoindex(devpath);
	if (ifindex != 0) {
		snprintf(buf, sizeof(buf), "%u", ifindex);
		udev_list_insert(udev_device_get_sysattr_list(ud), "ifindex", buf);
	}
}
