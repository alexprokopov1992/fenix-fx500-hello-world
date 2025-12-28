#define LOG_LEVEL CONFIG_USB_DEVICE_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/can.h>
#include <zephyr/device.h>

int main(void)
{
    int ret = 0;
    
    printk("Hello World from minimal!\n");

    // Отримуємо пристрої CAN за правильними labels з DTS
    const struct device *can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
    const struct device *can2 = DEVICE_DT_GET(DT_NODELABEL(fdcan2));

    if (!device_is_ready(can1)) {
        printk("FDCAN1 device not ready\n");
        return -1;
    }

    if (!device_is_ready(can2)) {
        printk("FDCAN2 device not ready\n");
        return -1;
    }

    // Налаштування тайміну: sjw=1, prop_seg=0, phase_seg1=13, phase_seg2=2, prescaler=20
    struct can_timing timing = {
        .sjw = 1,
        .prop_seg = 0,
        .phase_seg1 = 13,
        .phase_seg2 = 2,
        .prescaler = 20
    };

    // ========== FDCAN1 ==========
    ret = can_set_timing(can1, &timing);
    if (ret != 0) {
        printk("Failed to set FDCAN1 timing: %d\n", ret);
    }

    ret = can_set_mode(can1, CAN_MODE_NORMAL);
    if (ret != 0) {
        printk("Failed to set FDCAN1 mode: %d\n", ret);
    }

    ret = can_start(can1);
    if (ret != 0) {
        printk("Failed to start FDCAN1: %d\n", ret);
    } else {
        printk("FDCAN1 started successfully\n");
    }

    struct can_frame frame1 = {
        .id = 0x123,
        .dlc = 1,
        .data = {0x01}
    };

    ret = can_send(can1, &frame1, K_MSEC(100), NULL, NULL);
    if (ret != 0) {
        printk("Failed to send FDCAN1 frame: %d\n", ret);
    } else {
        printk("FDCAN1 frame sent\n");
    }

    // ========== FDCAN2 ==========
    ret = can_set_timing(can2, &timing);
    if (ret != 0) {
        printk("Failed to set FDCAN2 timing: %d\n", ret);
    }

    ret = can_set_mode(can2, CAN_MODE_NORMAL);
    if (ret != 0) {
        printk("Failed to set FDCAN2 mode: %d\n", ret);
    }

    ret = can_start(can2);
    if (ret != 0) {
        printk("Failed to start FDCAN2: %d\n", ret);
    } else {
        printk("FDCAN2 started successfully\n");
    }

    struct can_frame frame2 = {
        .id = 0x123,
        .dlc = 1,
        .data = {0x01}
    };

    ret = can_send(can2, &frame2, K_MSEC(100), NULL, NULL);
    if (ret != 0) {
        printk("Failed to send FDCAN2 frame: %d\n", ret);
    } else {
        printk("FDCAN2 frame sent\n");
    }

    for(;;) {
        k_msleep(1000);
    }

    return 0;
}