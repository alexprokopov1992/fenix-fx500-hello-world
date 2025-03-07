#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

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


    printk("Hello World from minimal!\n");

    for(;;) {
        k_msleep(1000);
        gpio_pin_toggle_dt(&led);
    }
    // gpio_pin_set_dt(&led, state);

    return 0;
}
