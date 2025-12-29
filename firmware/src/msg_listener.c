#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/sys/socket.h>

LOG_MODULE_REGISTER(msg_listener, LOG_LEVEL_INF);

#include "blink.h"

#define PORT 9666
#define ADDR4 "239.255.6.6"

static uint8_t buf[1024];

static void wait_up(void)
{
	struct net_if *iface;

	iface = net_if_get_default();

	while (true) {
		if (net_if_is_up(iface)) {
			return;
		}

		k_sleep(K_MSEC(200));
	}
}

static int open_mcast4(void)
{
	struct sockaddr_in addr4;
	struct ip_mreqn mreqn;
	int optval;
	int sock;
	int ret;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		LOG_ERR("socket failed: %d", errno);
		return -errno;
	}

	optval = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (ret < 0) {
		LOG_ERR("setsockopt failed: %d", errno);
		return -errno;
	}

	memset(&addr4, 0, sizeof(addr4));
	addr4.sin_family = AF_INET;
	addr4.sin_port = htons(PORT);

	ret = bind(sock, (struct sockaddr *)&addr4, sizeof(addr4));
	if (ret < 0) {
		LOG_ERR("bind failed: %d", errno);
		return -errno;
	}

	memset(&mreqn, 0, sizeof(mreqn));
	inet_pton(AF_INET, ADDR4, &mreqn.imr_multiaddr);
	mreqn.imr_ifindex = net_if_get_by_iface(net_if_get_default());

	ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreqn, sizeof(mreqn));
	if (ret < 0) {
		LOG_ERR("setsockopt failed: %d", errno);
		return -errno;
	}

	return sock;
}

static int listener_thread4(void)
{
	int sock;
	int ret;

	wait_up();

	ready();

	sock = open_mcast4();
	if (sock < 0) {
		return sock;
	}

	LOG_INF("open socket 4");

	while (true) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		char abuf[NET_IPV4_ADDR_LEN];

		ret = recvfrom(sock, buf, sizeof(buf), 0,
			       (struct sockaddr *)&client_addr, &client_addr_len);
		if (ret < 0) {
			LOG_ERR("recvfrom failed: %d", errno);
			continue;
		}

		LOG_INF("recv from: %s",
			net_addr_ntop(AF_INET, &client_addr.sin_addr, abuf, sizeof(abuf)));
		LOG_HEXDUMP_INF(buf, ret, "buf");

		blink();
	}

	return 0;
}

K_THREAD_DEFINE(listener4, 1024, listener_thread4, NULL, NULL, NULL, 1, 0, 0);

#define ADDR6 "ff12::fab:666"

static int open_mcast6(void)
{
	struct sockaddr_in6 addr6;
	struct ipv6_mreq mreq;
	int optval;
	int sock;
	int ret;

	sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		LOG_ERR("socket failed: %d", errno);
		return -errno;
	}

	optval = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (ret < 0) {
		LOG_ERR("setsockopt failed: %d", errno);
		return -errno;
	}

	memset(&addr6, 0, sizeof(addr6));
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(PORT);
	addr6.sin6_addr = net_in6addr_any;

	ret = bind(sock, (struct sockaddr *)&addr6, sizeof(addr6));
	if (ret < 0) {
		LOG_ERR("bind failed: %d", errno);
		return -errno;
	}

	memset(&mreq, 0, sizeof(mreq));
	inet_pton(AF_INET6, ADDR6, &mreq.ipv6mr_multiaddr);
	mreq.ipv6mr_ifindex = net_if_get_by_iface(net_if_get_default());

	ret = setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq));
	if (ret < 0) {
		LOG_ERR("setsockopt failed: %d", errno);
		return -errno;
	}

	return sock;
}

static int listener_thread6(void)
{
	int sock;
	int ret;

	wait_up();

	ready();

	sock = open_mcast6();
	if (sock < 0) {
		return sock;
	}

	LOG_INF("open socket 6");

	while (true) {
		struct sockaddr_in6 client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		char abuf[NET_IPV6_ADDR_LEN];

		ret = recvfrom(sock, buf, sizeof(buf), 0,
			       (struct sockaddr *)&client_addr, &client_addr_len);
		if (ret < 0) {
			LOG_ERR("recvfrom failed: %d", errno);
			continue;
		}

		LOG_INF("recv from: %s",
			net_addr_ntop(AF_INET6, &client_addr.sin6_addr, abuf, sizeof(abuf)));
		LOG_HEXDUMP_INF(buf, ret, "buf");

		blink();
	}

	return 0;
}

K_THREAD_DEFINE(listener6, 1024, listener_thread6, NULL, NULL, NULL, 1, 0, 0);
