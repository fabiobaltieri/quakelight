#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>

LOG_MODULE_REGISTER(net_led, LOG_LEVEL_INF);

#if DT_NODE_EXISTS(DT_NODELABEL(eth_led))

static const struct led_dt_spec eth_led = LED_DT_SPEC_GET(DT_NODELABEL(eth_led));

static void iface_up_handler(struct net_mgmt_event_callback *cb,
			     uint64_t mgmt_event, struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_IF_UP:
		led_on_dt(&eth_led);
		break;
	case NET_EVENT_IF_DOWN:
		led_off_dt(&eth_led);
		break;
	}
}

struct net_mgmt_event_callback iface_up_cb;

static int net_led_init(void)
{
	net_mgmt_init_event_callback(&iface_up_cb, iface_up_handler,
				     NET_EVENT_IF_UP | NET_EVENT_IF_DOWN);
	net_mgmt_add_event_callback(&iface_up_cb);

	led_on_dt(&eth_led);
	k_sleep(K_MSEC(200));
	led_off_dt(&eth_led);

	return 0;
}

SYS_INIT(net_led_init, APPLICATION, 0);
#endif
