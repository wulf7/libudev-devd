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

#ifndef UDEV_UTILS_H_
#define UDEV_UTILS_H_

struct udev_device;

#define	LIBUDEV_EXPORT	__attribute__((visibility("default")))

#define	DEV_PATH_ROOT	"/dev"
#define	DEV_PATH_MAX	80
#define	SYS_PATH_MAX	80

#define	DEVD_EVENT_ATTACH	'+'
#define	DEVD_EVENT_DETACH	'-'
#define	DEVD_EVENT_NOTICE	'!'
#define	DEVD_EVENT_UNKNOWN	'?'

#define	UNKNOWN_SUBSYSTEM	"#"
#define	UNKNOWN_DEVTYPE		"#"

typedef void (create_node_handler_t)(struct udev_device *udev_device);

const char *get_subsystem_by_syspath(const char *syspath, const char **devtype);
const char *get_sysname_by_syspath(const char *syspath);
const char *get_devpath_by_syspath(const char *syspath);
const char *get_syspath_by_devpath(const char *devpath);
const char *get_syspath_by_devnum(dev_t devnum);

void invoke_create_handler(struct udev_device *ud);
size_t syspathlen_wo_units(const char *path);

#endif /* UDEV_UTILS_H_ */
