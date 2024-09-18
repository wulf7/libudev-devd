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

#ifndef UDEV_DEVICE_H_
#define UDEV_DEVICE_H_

#include "libudev.h"
#include "udev-list.h"

/* udev_device flags */
enum {
	UD_ACTION_NONE,
	UD_ACTION_ADD,
	UD_ACTION_REMOVE,
	UD_ACTION_HOTPLUG,
};

struct udev_device *udev_device_new_common(struct udev *udev,
    const char *syspath, int action);
struct udev_list *udev_device_get_properties_list(struct udev_device *ud);
struct udev_list *udev_device_get_sysattr_list(struct udev_device *ud);
struct udev_list *udev_device_get_tags_list(struct udev_device *ud);
struct udev_list *udev_device_get_devlinks_list(struct udev_device *ud);
void udev_device_set_parent(struct udev_device *ud, struct udev_device *parent);
const char *_udev_device_get_syspath(struct udev_device *ud);
const char *_udev_device_get_sysname(struct udev_device *ud);

#endif /* UDEV_DVICE_H_ */
