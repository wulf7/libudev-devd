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

#ifndef UDEV_DEV_H_
#define UDEV_DEV_H_

#include "config.h"

#include "udev-utils.h"

struct udev_enumerate;

#if defined (HAVE_LINUX_INPUT_H) || defined (HAVE_DEV_EVDEV_INPUT_H)
create_node_handler_t	create_evdev_handler;
#endif
create_node_handler_t	create_keyboard_handler;
create_node_handler_t	create_mouse_handler;
create_node_handler_t	create_joystick_handler;
create_node_handler_t	create_touchpad_handler;
create_node_handler_t	create_touchscreen_handler;
create_node_handler_t	create_sysmouse_handler;
create_node_handler_t	create_kbdmux_handler;
create_node_handler_t	create_drm_handler;
#ifdef HAVE_DEV_HID_HIDRAW_H
create_node_handler_t	create_hidraw_handler;
#endif

int udev_dev_enumerate(struct udev_enumerate *ue);
int udev_dev_monitor(char *msg, char *syspath, size_t syspathlen);

#endif /* UDEV_DEV_H_ */
