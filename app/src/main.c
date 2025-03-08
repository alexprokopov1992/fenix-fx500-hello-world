#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#include <zephyr/drivers/sensor.h>
#include <sensor/shock_sensor/shock-sensor.h>

#define SHOCK_NODE       DT_ALIAS(shock_sensor)
static const struct device *dev = DEVICE_DT_GET(SHOCK_NODE);

void sensor_warn_handler(const struct device *dev, const struct sensor_trigger *trig)
{
	printk("Tap (Warn)!\n");
}

void sensor_main_handler(const struct device *dev, const struct sensor_trigger *trig)
{
    printk("Tap (Main)!\n");
}

const struct sensor_trigger trig_warn = {
    .chan = SENSOR_CHAN_PROX,
    .type = SENSOR_TRIG_TAP,
};

const struct sensor_trigger trig_main = {
    .chan = SENSOR_CHAN_PROX,
    .type = SENSOR_TRIG_THRESHOLD,
};

static int init_shock_sensor()
{
    if(!device_is_ready(dev)) {
        printk("ADC VBUS controller device %s not ready\n", dev->name);
        return -ENODEV;
    }

    int warn_zone = 10;
    int main_zone = 60;
    struct sensor_value val = { .val1 = warn_zone, .val2 = main_zone };
    sensor_attr_set(dev, SENSOR_CHAN_PROX, SENSOR_ATTR_UPPER_THRESH, &val);

    int rc = sensor_trigger_set(dev, &trig_warn, sensor_warn_handler);
    rc = sensor_trigger_set(dev, &trig_main, sensor_main_handler);
    return 0;
}

int main(void)
{
	if (!gpio_is_ready_dt(&led)) {
        printk("LED0 is not ready.\n");
		return 0;
	}

    int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
        printk("Unable to configure LED0.\n");
		return 0;
	}

    init_shock_sensor();

    printk("Hello World from minimal!\n");

    for(;;) {
        k_msleep(1000);
        gpio_pin_toggle_dt(&led);
    }
    // gpio_pin_set_dt(&led, state);

    return 0;
}
