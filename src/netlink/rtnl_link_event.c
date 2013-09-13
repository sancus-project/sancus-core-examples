#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>	/* malloc(), free() */
#include <stddef.h>	/* offsetof */
#include <stdbool.h>	/* offsetof */
#include <unistd.h>
#include <netinet/in.h> /* struct sockaddr */
#include <stdio.h>	/* printf */
#include <errno.h>	/* sancus_fd */
#include <string.h>	/* sancus_zalloc uses memset */

#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <ev.h>
#include <sancus_common.h>
#include <sancus_alloc.h>
#include <sancus_buffer.h>
#include <sancus_stream.h>
#include <sancus_fd.h>
#include <sancus_netlink.h>

#include "rtnl_link_event.h"


static void netlink_rtlink_receiver_error(struct sancus_nl_receiver *UNUSED(recv),
		   struct ev_loop *UNUSED(loop),
		   enum sancus_nl_receiver_error error)
{
	switch (error) {
	case SANCUS_NL_RECEIVER_WATCHER_ERROR:
		printf("watcher error\n"); break;
	case SANCUS_NL_RECEIVER_RECVFROM_ERROR:
		printf("recvfrom error\n"); break;
	}
}

static int netlink_rtlink_receiver_attr_parse(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	uint16_t type = sancus_nl_attr_get_type(attr);

	/* skip unsupported attribute */
	if (sancus_nl_attr_type_valid(attr, IFLA_MAX) < 0)
		return SANCUS_NL_CB_OK;

	switch(type) {
	case IFLA_IFNAME:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_STRING)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_TXQLEN:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_U32)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_OPERSTATE:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_U8)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_LINKMODE:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_U8)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_MTU:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_U32)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_LINK:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_U32)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_MASTER:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_U32)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_QDISC:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_STRING)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;
	case IFLA_IFALIAS:
		if (sancus_nl_attr_validate_minlen(attr, SANCUS_NL_ATTR_TYPE_STRING)) {
			perror("sancus_nl_attr_validate_minlen");
			return SANCUS_NL_CB_ERROR;
		}
		break;

	/*
	case IFLA_MAP:
		printf("ifmap");
		break;
	case IFLA_ADDRESS:
		printf("address");
		break;
	case IFLA_BROADCAST:
		printf("broadcast");
		break;
	case IFLA_STATS:
		printf("stats");
		break;
	*/

	}

	/* store attribute */
	tb[type] = attr;

	return SANCUS_NL_CB_OK;
}

static bool netlink_rtlink_receiver_on_message(struct sancus_nl_receiver *UNUSED(recv), struct ev_loop *UNUSED(loop), const struct nlmsghdr *nlh)
{
	struct nlattr *tb[IFLA_MAX+1] = { NULL };
	struct ifinfomsg *ifm = sancus_nl_msg_get_payload(nlh);

	printf("index=%d type=%d flags=%d family=%d ",
		ifm->ifi_index, ifm->ifi_type,
		ifm->ifi_flags, ifm->ifi_family);

	if (ifm->ifi_flags & IFF_RUNNING)
		printf("[RUNNING] ");
	else
		printf("[NOT RUNNING] ");

	sancus_nl_attr_parse(nlh, sizeof(*ifm), netlink_rtlink_receiver_attr_parse, tb);

	if (tb[IFLA_IFNAME])
		printf("iface=%s ", sancus_nl_attr_get_string(tb[IFLA_IFNAME]));
	if (tb[IFLA_TXQLEN])
		printf("txqlen=%i ", sancus_nl_attr_get_u32(tb[IFLA_TXQLEN]));
	if (tb[IFLA_OPERSTATE])
		printf("operstate=%i ", sancus_nl_attr_get_u8(tb[IFLA_OPERSTATE]));
	if (tb[IFLA_LINKMODE])
		printf("linkmode=%i ", sancus_nl_attr_get_u8(tb[IFLA_LINKMODE]));
	if (tb[IFLA_MTU])
		printf("mtu=%i ", sancus_nl_attr_get_u32(tb[IFLA_MTU]));
	if (tb[IFLA_LINK])
		printf("link=%i ", sancus_nl_attr_get_u32(tb[IFLA_LINK]));
	if (tb[IFLA_MASTER])
		printf("master=%i ", sancus_nl_attr_get_u32(tb[IFLA_MASTER]));
	if (tb[IFLA_QDISC])
		printf("qdisc=%s ", sancus_nl_attr_get_string(tb[IFLA_QDISC]));
	if (tb[IFLA_IFALIAS])
		printf("ifalias=%s ", sancus_nl_attr_get_string(tb[IFLA_IFALIAS]));

	printf("\n");

	return true;
}

static int netlink_rtlink_receiver_init(struct netlink_rtlink_receiver *self, int bus, unsigned int groups, pid_t pid)
{
	struct ifinfomsg *ifm;

	static const struct sancus_nl_receiver_settings settings = {
		.on_message = netlink_rtlink_receiver_on_message,
		.on_error = netlink_rtlink_receiver_error,
		.attribute_offset = sizeof(*ifm),
	};

	/* socket */
	switch (sancus_nl_receiver_listen(&self->receiver, &settings, bus, groups, pid)) {
	case -1:
		perror("socket error");
		return -1;
	case 0:
		perror("socket error");
		return -1;
	}

	return self != NULL;
}

static void netlink_rtlink_receiver_start(struct netlink_rtlink_receiver *self, struct ev_loop *loop)
{
	sancus_nl_receiver_start(&self->receiver, loop);
}



/*
 * main
 */
int main(int UNUSED(argc), char * UNUSED(argv[]))
{
	struct ev_loop *loop = ev_default_loop(0);
	struct netlink_rtlink_receiver *receiver = sancus_zalloc(sizeof(*receiver));

	if (netlink_rtlink_receiver_init(receiver, NETLINK_ROUTE, RTMGRP_LINK, 0) < 0)
		return -1;

	netlink_rtlink_receiver_start(receiver, loop);

	ev_loop(loop, 0);

	return 0;
}
