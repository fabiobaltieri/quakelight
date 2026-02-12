#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(leds, LOG_LEVEL_INF);

#define STRIP_1_NODE DT_NODELABEL(leds_1)
#define STRIP_2_NODE DT_NODELABEL(leds_2)
#define STRIP_3_NODE DT_NODELABEL(leds_3)

#define DEFINE_BUFS(n, node) \
	static struct led_rgb pixels_##n##_buf[DT_PROP_OR(node, chain_length, 0)]; \
	static struct led_rgb pixels_##n##_scratch[DT_PROP_OR(node, chain_length, 0)]; \
	static struct led_rgb pixels_##n##_target[DT_PROP_OR(node, chain_length, 0)]; \

DEFINE_BUFS(0, STRIP_1_NODE);
DEFINE_BUFS(1, STRIP_2_NODE);
DEFINE_BUFS(2, STRIP_3_NODE);

static struct {
	const struct device *dev;
	int num_pixels;
	struct led_rgb *px_buf;
	struct led_rgb *px_scratch;
	struct led_rgb *px_target;
} led_strip[] = {
	{
		.dev = DEVICE_DT_GET_OR_NULL(STRIP_1_NODE),
		.num_pixels = DT_PROP_OR(STRIP_1_NODE, chain_length, 0),
		.px_buf = pixels_0_buf,
		.px_scratch = pixels_0_scratch,
		.px_target = pixels_0_target,
	},
	{
		.dev = DEVICE_DT_GET_OR_NULL(STRIP_2_NODE),
		.num_pixels = DT_PROP_OR(STRIP_2_NODE, chain_length, 0),
		.px_buf = pixels_1_buf,
		.px_scratch = pixels_1_scratch,
		.px_target = pixels_1_target,
	},
	{
		.dev = DEVICE_DT_GET_OR_NULL(STRIP_3_NODE),
		.num_pixels = DT_PROP_OR(STRIP_3_NODE, chain_length, 0),
		.px_buf = pixels_2_buf,
		.px_scratch = pixels_2_scratch,
		.px_target = pixels_2_target,
	},
};

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

#define LEVEL 0x60

static const struct led_rgb colors[] = {
	RGB(LEVEL, 0x00, 0x00), /* red */
	RGB(0x00, LEVEL, 0x00), /* green */
	RGB(0x00, 0x00, LEVEL), /* blue */

	RGB(LEVEL/2, LEVEL/2, 0x00),
	RGB(0x00, LEVEL/2, LEVEL/2),
	RGB(LEVEL/2, 0x00, LEVEL/2),

	RGB(LEVEL/3, LEVEL/3, LEVEL/3),
};

#define DELAY_TIME_MS 10
#define CYCLES 40
#define STEP (LEVEL / CYCLES)

static void step_pixel(struct led_rgb *value, struct led_rgb *target)
{
	if (value->r < target->r) {
		value->r = clamp(value->r + STEP, 0, LEVEL);
	} else {
		value->r = clamp(value->r - STEP, 0, LEVEL);
	}

	if (value->g < target->g) {
		value->g = clamp(value->g + STEP, 0, LEVEL);
	} else {
		value->g = clamp(value->g - STEP, 0, LEVEL);
	}

	if (value->b < target->b) {
		value->b = clamp(value->b + STEP, 0, LEVEL);
	} else {
		value->b = clamp(value->b - STEP, 0, LEVEL);
	}
}

static void leds_random(int id)
{
	const struct device *dev = led_strip[id].dev;
	int num_pixels = led_strip[id].num_pixels;
	struct led_rgb *pixels = led_strip[id].px_buf;
	struct led_rgb *scratch = led_strip[id].px_scratch;
	struct led_rgb *target = led_strip[id].px_target;

	int ret;

	if (num_pixels == 0) {
		return;
	}

	while (1) {
		for (uint16_t i = 0; i < num_pixels; i++) {
			uint8_t color;

			color = sys_rand8_get() % ARRAY_SIZE(colors);

			memcpy(&target[i], &colors[color], sizeof(struct led_rgb));
		}

		for (uint8_t i = 0; i < CYCLES; i++) {
			for (uint16_t i = 0; i < num_pixels; i++) {
				step_pixel(&pixels[i], &target[i]);
			}

			memcpy(scratch, pixels, sizeof(*pixels) * num_pixels);
			ret = led_strip_update_rgb(dev, scratch, num_pixels);
			if (ret) {
				LOG_ERR("led_strip_update_rgb error: %d", ret);
			}

			k_sleep(K_MSEC(DELAY_TIME_MS));
		}
	}
}

static void leds_sequence(int id)
{
	const struct device *dev = led_strip[id].dev;
	int num_pixels = led_strip[id].num_pixels;
	struct led_rgb *pixels = led_strip[id].px_buf;
	struct led_rgb *scratch = led_strip[id].px_scratch;

	int ret;

	if (num_pixels == 0) {
		return;
	}

	int i = 0;
	while (1) {
		memset(pixels, 0, sizeof(*pixels) * num_pixels);

		memcpy(&pixels[i], &colors[6], sizeof(struct led_rgb));

		memcpy(scratch, pixels, sizeof(*pixels) * num_pixels);
		ret = led_strip_update_rgb(dev, scratch, num_pixels);
		if (ret) {
			LOG_ERR("led_strip_update_rgb error: %d", ret);
		}

		k_sleep(K_MSEC(DELAY_TIME_MS));

		i = (i + 1) % num_pixels;
	}
}

static void leds_color_sequence(int id)
{
	const struct device *dev = led_strip[id].dev;
	int num_pixels = led_strip[id].num_pixels;
	struct led_rgb *pixels = led_strip[id].px_buf;
	struct led_rgb *scratch = led_strip[id].px_scratch;

	int ret;

	if (num_pixels == 0) {
		return;
	}

	memset(pixels, 0, sizeof(*pixels) * num_pixels);

	int i = 0;
	int j = 0;
	while (1) {
		memcpy(&pixels[i], &colors[j], sizeof(struct led_rgb));

		memcpy(scratch, pixels, sizeof(*pixels) * num_pixels);
		ret = led_strip_update_rgb(dev, scratch, num_pixels);
		if (ret) {
			LOG_ERR("led_strip_update_rgb error: %d", ret);
		}

		k_sleep(K_MSEC(DELAY_TIME_MS));

		i = (i + 1) % num_pixels;
		if (i == 0)
			j = (j + 1) % 7;
	}
}

static void leds_thread(void *p1, void *p2, void *p3)
{
	int id = (int)p1;
	const struct device *dev = led_strip[id].dev;

	if (!dev) {
		LOG_INF("LED strip device %d is NULL", id);
		return;
	}

	if (!device_is_ready(dev)) {
		LOG_ERR("LED strip device %s is not ready", dev->name);
		return;
	}

	switch (id) {
	case 0:
		leds_random(id);
		break;
	case 1:
		leds_sequence(id);
		break;
	case 2:
		leds_color_sequence(id);
		break;
	}

	k_sleep(K_FOREVER);
}

K_THREAD_DEFINE(leds_1, 1024, leds_thread, 0, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(leds_2, 1024, leds_thread, 1, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(leds_3, 1024, leds_thread, 2, NULL, NULL, 1, 0, 0);
