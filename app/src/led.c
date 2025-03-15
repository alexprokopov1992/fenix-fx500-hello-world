#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define CONFIG_LED_THREAD_PRIORITY 7

struct k_thread led_thread;
static K_KERNEL_STACK_DEFINE(led_thread_stack, 512);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void led_thread_fun(void *dummy1, void *dummy2, void *dummy3)
{
    while (1) {
        gpio_pin_toggle_dt(&led);
        k_msleep(100);
    }
}

static int init()
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

	k_tid_t tid = k_thread_create(&led_thread, led_thread_stack,
        K_KERNEL_STACK_SIZEOF(led_thread_stack),
        led_thread_fun, (void *)&led, NULL, NULL,
        CONFIG_LED_THREAD_PRIORITY,
        0, K_NO_WAIT);
    if (!tid) {
        printk("thread creation failed\n");
        return -ENODEV;
    }
    k_thread_name_set(&led_thread, "LED");
    return 0;
}

SYS_INIT(init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
