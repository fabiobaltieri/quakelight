#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(blink, LOG_LEVEL_INF);

static const struct led_dt_spec led_a = LED_DT_SPEC_GET_OR(DT_NODELABEL(led_a), {0});
static const struct led_dt_spec led_b = LED_DT_SPEC_GET_OR(DT_NODELABEL(led_b), {0});

static void blink_worker(struct k_work *work)
{
	led_off_dt(&led_b);
}

static K_WORK_DELAYABLE_DEFINE(blink_work, blink_worker);

void blink(void)
{
	led_on_dt(&led_b);

	k_work_reschedule(&blink_work, K_MSEC(100));
}

void ready(void)
{
	led_on_dt(&led_a);
}
