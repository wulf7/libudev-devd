#ifndef UTILS_HWDB_H_
#define UTILS_HWDB_H_

typedef int libhwdb_ins_cb_t(void *ctx, const char *key, const char *value);

struct hwdb;

struct hwdb *libhwdb_open(const char *path);
struct hwdb *libhwdb_close(struct hwdb *hwdb);
int libhwdb_get_properties_list_entry(libhwdb_ins_cb_t ins_cb_fn,
    void *ins_cb_ctx, struct hwdb *hwdb, const char *modalias);

#endif	/* UTILS_HWDB_H_ */
