#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("Hello World from minimal!\n");

    for(;;) {
        k_msleep(1000);
    }

    return 0;
}
