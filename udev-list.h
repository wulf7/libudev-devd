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

#ifndef UDEV_LIST_H_
#define UDEV_LIST_H_

#include "config.h"
#include "utils.h"

RB_HEAD(udev_list, udev_list_entry);

void udev_list_init(struct udev_list *ul);
int udev_list_insert(struct udev_list *ul, char const *name,
    char const *value);
int udev_list_insertf(struct udev_list *ul, char const *name,
    char const *fmt, ...);
void udev_list_free(struct udev_list *ul);
struct udev_list_entry *udev_list_entry_get_first(struct udev_list *ul);
const char *_udev_list_entry_get_name(struct udev_list_entry *ule);
const char *_udev_list_entry_get_value(struct udev_list_entry *ule);

#endif /* UDEV_LIST_H_ */
