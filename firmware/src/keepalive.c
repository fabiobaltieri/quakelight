#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/posix/arpa/inet.h>
#include <zephyr/posix/sys/socket.h>

LOG_MODULE_REGISTER(keepalive, LOG_LEVEL_INF);

#define PORT 9667
#define ADDR4 "239.255.6.6"
#define ADDR6 "ff12::fab:666"

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

static char buf[64];

static int keepalive_thread(void)
{
	int sock;
	int sock6;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;
	int ret;

	wait_up();

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		LOG_ERR("socket failed: %d", errno);
		return -errno;
	}

	sock6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		LOG_ERR("socket failed: %d", errno);
		return -errno;
	}

	LOG_INF("open keepalive sockets");

	inet_pton(AF_INET, ADDR4, &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);

	inet_pton(AF_INET6, ADDR6, &addr6.sin6_addr);
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(PORT);

	while (true) {
		int len = snprintf(buf, sizeof(buf), "ping %lld", k_uptime_get());

		ret = sendto(sock, buf, len, 0, (const struct sockaddr *)&addr, sizeof(addr));
		if (ret < 0) {
			LOG_WRN("sendto failed: %d", errno);
		}

		ret = sendto(sock6, buf, len, 0, (const struct sockaddr *)&addr6, sizeof(addr6));
		if (ret < 0) {
			LOG_WRN("sendto failed: %d", errno);
			return -ret;
		}

		k_sleep(K_SECONDS(1));
	}

	return 0;
}

K_THREAD_DEFINE(keepalive, 1024, keepalive_thread, NULL, NULL, NULL, 1, 0, 0);
