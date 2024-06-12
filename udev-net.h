#ifndef UDEV_NET_H_
#define UDEV_NET_H_

#include "config.h"
#include "utils.h"

#include "udev-utils.h"

create_node_handler_t	create_net_handler;

int udev_net_enumerate(struct scan_ctx *ctx);
int udev_net_monitor(char *msg, char *syspath, size_t syspathlen);

#endif /* UDEV_NET_H_ */
