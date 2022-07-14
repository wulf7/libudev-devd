/***
  This file is part of systemd.

  Copyright 2012 Kay Sievers <kay@vrfy.org>
  Copyright 2008 Alan Jenkins <alan.christopher.jenkins@googlemail.com>

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <sys/endian.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils-hwdb.h"
#include "utils-hwdb-def.h"

struct hwdb {
	int fd;
	size_t size;
	union {
		struct trie_header_f *head;
        	const char *map;
	};
	libhwdb_ins_cb_t *ins_cb_fn;
	void *ins_cb_ctx;
};

struct linebuf {
	char bytes[LINE_MAX];
	size_t size;
	size_t len;
};

static void linebuf_init(struct linebuf *buf) {
	buf->size = 0;
	buf->len = 0;
}

static const char *linebuf_get(struct linebuf *buf) {
        if (buf->len + 1 >= sizeof(buf->bytes))
                return NULL;
        buf->bytes[buf->len] = '\0';
        return buf->bytes;
}

static bool linebuf_add(struct linebuf *buf, const char *s, size_t len) {
        if (buf->len + len >= sizeof(buf->bytes))
                return false;
        memcpy(buf->bytes + buf->len, s, len);
        buf->len += len;
        return true;
}

static bool linebuf_add_char(struct linebuf *buf, char c)
{
        if (buf->len + 1 >= sizeof(buf->bytes))
                return false;
        buf->bytes[buf->len++] = c;
        return true;
}

static void linebuf_rem(struct linebuf *buf, size_t count) {
        assert(buf->len >= count);
        buf->len -= count;
}

static void linebuf_rem_char(struct linebuf *buf) {
        linebuf_rem(buf, 1);
}

static const struct trie_child_entry_f *trie_node_children(struct hwdb *hwdb, const struct trie_node_f *node) {
        return (const struct trie_child_entry_f *)((const char *)node + le64toh(hwdb->head->node_size));
}

static const struct trie_value_entry_f *trie_node_values(struct hwdb *hwdb, const struct trie_node_f *node) {
        const char *base = (const char *)node;

        base += le64toh(hwdb->head->node_size);
        base += node->children_count * le64toh(hwdb->head->child_entry_size);
        return (const struct trie_value_entry_f *)base;
}

static const struct trie_node_f *trie_node_from_off(struct hwdb *hwdb, uint64_t off) {
        return (const struct trie_node_f *)(hwdb->map + le64toh(off));
}

static const char *trie_string(struct hwdb *hwdb, uint64_t off) {
        return hwdb->map + le64toh(off);
}

static int trie_children_cmp_f(const void *v1, const void *v2) {
        const struct trie_child_entry_f *n1 = v1;
        const struct trie_child_entry_f *n2 = v2;

        return n1->c - n2->c;
}

static const struct trie_node_f *node_lookup_f(struct hwdb *hwdb, const struct trie_node_f *node, uint8_t c) {
        struct trie_child_entry_f *child;
        struct trie_child_entry_f search;

        search.c = c;
        child = bsearch(&search, trie_node_children(hwdb, node), node->children_count,
                        le64toh(hwdb->head->child_entry_size), trie_children_cmp_f);
        if (child)
                return trie_node_from_off(hwdb, child->child_off);
        return NULL;
}

static int
hwdb_add_property(struct hwdb *hwdb, const char *key, const char *value)
{
	if (key[0] == ' ')
		return (hwdb->ins_cb_fn(hwdb->ins_cb_ctx, key+1, value));
	return (0);
}

static int trie_fnmatch_f(struct hwdb *hwdb, const struct trie_node_f *node, size_t p,
                          struct linebuf *buf, const char *search) {
        size_t len;
        size_t i;
        const char *prefix;
        int err;

        prefix = trie_string(hwdb, node->prefix_off);
        len = strlen(prefix + p);
        linebuf_add(buf, prefix + p, len);

        for (i = 0; i < node->children_count; i++) {
                const struct trie_child_entry_f *child = &trie_node_children(hwdb, node)[i];

                linebuf_add_char(buf, child->c);
                err = trie_fnmatch_f(hwdb, trie_node_from_off(hwdb, child->child_off), 0, buf, search);
                if (err != 0)
                        return err;
                linebuf_rem_char(buf);
        }

        if (le64toh(node->values_count) && fnmatch(linebuf_get(buf), search, 0) == 0)
                for (i = 0; i < le64toh(node->values_count); i++) {
                        err = hwdb_add_property(hwdb, trie_string(hwdb, trie_node_values(hwdb, node)[i].key_off),
                                                trie_string(hwdb, trie_node_values(hwdb, node)[i].value_off));
                        if (err != 0)
                                return err;
                }

        linebuf_rem(buf, len);
        return 0;
}

static int trie_search_f(struct hwdb *hwdb, const char *search) {
        struct linebuf buf;
        const struct trie_node_f *node;
        size_t i = 0;
        int err;

        linebuf_init(&buf);

        node = trie_node_from_off(hwdb, hwdb->head->nodes_root_off);
        while (node) {
                const struct trie_node_f *child;
                size_t p = 0;

                if (node->prefix_off) {
                        uint8_t c;

                        for (; (c = trie_string(hwdb, node->prefix_off)[p]); p++) {
                                if (c == '*' || c == '?' || c == '[')
                                        return trie_fnmatch_f(hwdb, node, p, &buf, search + i + p);
                                if (c != search[i + p])
                                        return 0;
                        }
                        i += p;
                }

                child = node_lookup_f(hwdb, node, '*');
                if (child) {
                        linebuf_add_char(&buf, '*');
                        err = trie_fnmatch_f(hwdb, child, 0, &buf, search + i);
                        if (err != 0)
                                return err;
                        linebuf_rem_char(&buf);
                }

                child = node_lookup_f(hwdb, node, '?');
                if (child) {
                        linebuf_add_char(&buf, '?');
                        err = trie_fnmatch_f(hwdb, child, 0, &buf, search + i);
                        if (err != 0)
                                return err;
                        linebuf_rem_char(&buf);
                }

                child = node_lookup_f(hwdb, node, '[');
                if (child) {
                        linebuf_add_char(&buf, '[');
                        err = trie_fnmatch_f(hwdb, child, 0, &buf, search + i);
                        if (err != 0)
                                return err;
                        linebuf_rem_char(&buf);
                }

                if (search[i] == '\0') {
                        size_t n;

                        for (n = 0; n < le64toh(node->values_count); n++) {
                                err = hwdb_add_property(hwdb, trie_string(hwdb, trie_node_values(hwdb, node)[n].key_off),
                                                        trie_string(hwdb, trie_node_values(hwdb, node)[n].value_off));
                                if (err != 0)
                                        return err;
                        }
                        return 0;
                }

                child = node_lookup_f(hwdb, node, search[i]);
                node = child;
                i++;
        }
        return 0;
}

struct hwdb *
libhwdb_open(const char *path)
{
	struct hwdb *hwdb;
	struct stat st;
	const char sig[] = HWDB_SIG;

	hwdb = calloc(1, sizeof(struct hwdb));
	if (hwdb == NULL)
		return (NULL);

	hwdb->fd = open(path, O_RDONLY);
	if (hwdb->fd < 0)
		goto close;

	if (fstat(hwdb->fd, &st) < 0)
	        goto close;

	hwdb->size = st.st_size;
	if (hwdb->size < offsetof(struct trie_header_f, strings_len) + 8)
		goto close;

	hwdb->map = mmap(0, hwdb->size, PROT_READ, MAP_SHARED, hwdb->fd, 0);
	if (hwdb->map == MAP_FAILED)
		goto close;

	if (memcmp(hwdb->map, sig, sizeof(hwdb->head->signature)) != 0 ||
	    (size_t)st.st_size != le64toh(hwdb->head->file_size))
		goto close;

        return (hwdb);

close:
	libhwdb_close(hwdb);
	return (NULL);
}

struct hwdb *
libhwdb_close(struct hwdb *hwdb)
{
	if (hwdb != NULL) {
		if (hwdb->map != NULL)
			munmap((void *)hwdb->map, hwdb->size);
		if (hwdb->fd >= 0)
			close(hwdb->fd);
		free(hwdb);
        }
	return (NULL);
}

int
libhwdb_get_properties_list_entry(libhwdb_ins_cb_t ins_cb_fn, void *ins_cb_ctx,
    struct hwdb *hwdb, const char *modalias)
{
	if (hwdb == NULL || hwdb->fd < 0)
		return (EINVAL);

	hwdb->ins_cb_fn = ins_cb_fn;
	hwdb->ins_cb_ctx = ins_cb_ctx;
	return (trie_search_f(hwdb, modalias));
}
