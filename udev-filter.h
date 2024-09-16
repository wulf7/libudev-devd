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

#ifndef UDEV_FILTER_H_
#define UDEV_FILTER_H_

#include <sys/types.h>
#include <sys/queue.h>
#include <stdbool.h>

enum {
	UDEV_FILTER_TYPE_SUBSYSTEM,
	UDEV_FILTER_TYPE_SYSNAME,
	UDEV_FILTER_TYPE_PROPERTY,
	UDEV_FILTER_TYPE_TAG,
	UDEV_FILTER_TYPE_SYSATTR,
	UDEV_FILTER_TYPE_CNT,
};
STAILQ_HEAD(udev_filter_head, udev_filter_entry);

void udev_filter_init(struct udev_filter_head *ufh);
bool udev_filter_match_subsystem(struct udev_filter_head *ufh,
    const char *subsystem);
bool udev_filter_match(struct udev *udev, struct udev_filter_head *ufh,
    const char *syspath);
int udev_filter_add(struct udev_filter_head *ufh, int type, int neg,
    const char *expr, const char *value);
void udev_filter_free(struct udev_filter_head *ufh);

#endif /* UDEV_FILTER_H_ */
