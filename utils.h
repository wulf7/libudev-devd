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

#ifndef UTILS_H_
#define UTILS_H_

#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

#ifdef HAVE_SYS_TREE_H
#include <sys/tree.h>
#else
#include "tree.h"
#endif

/* #define	ENABLE_TRACE */
#define	LOG_LEVEL       0

/*
#ifndef	LOG_LEVEL
#define	LOG_LEVEL -1
#endif
*/
#ifdef ENABLE_TRACE
#define	TRC(msg, ...)							\
do {									\
	int saved_errno_ = errno;					\
	fprintf(stderr, "%s" msg "\n", __FUNCTION__, ##__VA_ARGS__);	\
	errno = saved_errno_;						\
} while (0)
#else
#define	TRC(msg, ...)
#endif

#define LOG(level, msg, ...) do {					\
	if (level < LOG_LEVEL) {					\
		if (level == 0 && errno != 0)				\
			fprintf(stderr, msg" %d(%s)\n", ##__VA_ARGS__,	\
			    errno, strerror(errno));			\
		else							\
			fprintf(stderr, msg"\n", ##__VA_ARGS__);	\
	}								\
} while (0)
#define	ERR(...)	LOG(0, __VA_ARGS__)
#define	DBG(...)	LOG(1, __VA_ARGS__)

#define	UNIMPL()	ERR("%s is unimplemented", __FUNCTION__)

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

/*
 * On Linuxolator st.st_rdev returned by stat() contains faked Linux device
 * major/minor numbers while st.ino still contains FreeBSD native numbers.
 * Choose which one we will use in udev_device_get_devnum() and
 * udev_device_new_from_devnum() calls.
 */
/* #define	ENABLE_FREEBSD_DEVNUM	1 */

#ifdef ENABLE_FREEBSD_DEVNUM
#define	ST_RDEV	st_ino
#else
#define	ST_RDEV	st_rdev
#endif

typedef int (* scandir_cb_t)(const char *path, mode_t type, void *args);

/* If .recursive is true, then .cb gets called for non-dir
 * paths, an the overall scandir is recursive. If .recursive
 * is false, then .cb gets called for all paths in the
 * directory, and scandir is non-recursive.
 */
struct scandir_ctx {
	bool recursive;
	scandir_cb_t cb;
	void *args;
};

char *strbase(const char *path);
char *get_kern_prop_value(const char *buf, const char *prop, size_t *len);
int match_kern_prop_value(const char *buf, const char *prop, const char *value);
int path_to_fd(const char *path);
int scandir_recursive(char *path, size_t len, struct scandir_ctx *ctx);
#ifdef HAVE_DEVINFO_H
struct devinfo_dev;
typedef int (* scandev_cb_t)(struct devinfo_dev *dev, void *args);
struct scandev_ctx {
	scandev_cb_t cb;
	void *args;
};
int scandev_recursive(struct scandev_ctx *ctx);
#endif
#ifndef HAVE_DEVNAME_R
char *devname_r(dev_t dev, mode_t type, char *buf, int len);
#endif
#ifndef HAVE_PIPE2
int pipe2(int fildes[2], int flags);
#endif
#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *p, int ch);
#endif
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif

#endif /* UTILS_H_ */
