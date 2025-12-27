#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(temp, LOG_LEVEL_INF);

static const struct device *temp_dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(digi_die_temp));

static bool temp_log_on;

static void temp_worker(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(temp_work, temp_worker);

static void print_temp()
{
	struct sensor_value val;
	double temp;

	sensor_sample_fetch_chan(temp_dev, SENSOR_CHAN_DIE_TEMP);

	sensor_channel_get(temp_dev, SENSOR_CHAN_DIE_TEMP, &val);
	temp = sensor_value_to_float(&val);

	LOG_INF("t=%.1f", temp);
}

static void temp_worker(struct k_work *work)
{
	if (!temp_log_on) {
		return;
	}

	print_temp();

	k_work_reschedule(&temp_work, K_SECONDS(1));
}

static int cmd_temp(const struct shell *sh, size_t argc, char **argv)
{
	if (temp_dev == NULL) {
		return -ENODEV;
	}

	if (argc == 2) {
		int err = 0;
		bool en;

		en = shell_strtobool(argv[1], 0, &err);
		if (err) {
			shell_error(sh, "Invalid argument: %s", argv[2]);
			return err;
		}

		temp_log_on = en;

		if (en) {
			k_work_reschedule(&temp_work, K_SECONDS(1));
		}

		return 0;
	}

	print_temp();

	return 0;
}

SHELL_CMD_ARG_REGISTER(temp, NULL, "Temperature", cmd_temp, 0, 1);
