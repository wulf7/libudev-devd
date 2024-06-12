#ifndef UDEV_UTILS_H_
#define UDEV_UTILS_H_

#include "libudev.h"

#define	LIBUDEV_EXPORT	__attribute__((visibility("default")))

#define	DEV_PATH_ROOT	"/dev"
#define	DEV_PATH_MAX	80
#define	SYS_PATH_MAX	80

#define	DEVD_EVENT_ATTACH	'+'
#define	DEVD_EVENT_DETACH	'-'
#define	DEVD_EVENT_NOTICE	'!'
#define	DEVD_EVENT_UNKNOWN	'?'

#define	UNKNOWN_SUBSYSTEM	"#"

typedef void (create_node_handler_t)(struct udev_device *udev_device);

const char *get_subsystem_by_syspath(const char *syspath, const char **devtype);
const char *get_sysname_by_syspath(const char *syspath);
const char *get_devpath_by_syspath(const char *syspath);
const char *get_syspath_by_devpath(const char *devpath);
const char *get_syspath_by_devnum(dev_t devnum);

void invoke_create_handler(struct udev_device *ud);
size_t syspathlen_wo_units(const char *path);

#endif /* UDEV_UTILS_H_ */
