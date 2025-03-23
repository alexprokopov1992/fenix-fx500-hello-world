#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>


int main(void)
{
    printk("Hello World from minimal!\n");
    const struct device *devices;
    size_t device_count = z_device_get_all_static(&devices);

    // Перебираем и выводим имена устройств
    for (size_t i = 0; i < device_count; i++) {
        const struct device *dev = &devices[i];
        if (dev && dev->name) {
            printk("Device %zu: %s\n", i, dev->name);
        }
    }
    for(;;) {
        k_msleep(1000);
    }

    return 0;
}
