#include <zephyr/drivers/hwinfo.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static void update_hostname(void)
{
	char hostname[32];
	uint8_t dev_id[16];
	ssize_t length;

	length = hwinfo_get_device_id(dev_id, sizeof(dev_id));

	if (length < 3) {
		LOG_ERR("unexpected hwinfo length: %d", length);
		return;
	}

	snprintf(hostname, sizeof(hostname), "%s-%02x%02x%02x",
		 CONFIG_NET_HOSTNAME,
		 dev_id[0], dev_id[1], dev_id[2]);

	LOG_INF("hostname=%s", hostname);

	net_hostname_set(hostname, strlen(hostname));
}

int main(void)
{
	LOG_INF("started");

	update_hostname();

#if CONFIG_NET_DHCPV4
	struct net_if *iface = net_if_get_default();

	if (!iface) {
		LOG_ERR("no default network interface");
		return 0;
	}

	net_dhcpv4_start(iface);
#endif

	k_sleep(K_FOREVER);
}
