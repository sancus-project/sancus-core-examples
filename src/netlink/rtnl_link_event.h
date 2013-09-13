#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

enum {
	READ_BUFFER_SIZE = 8192,
};

struct netlink_rtlink_receiver {
	struct sancus_nl_receiver receiver;
};

