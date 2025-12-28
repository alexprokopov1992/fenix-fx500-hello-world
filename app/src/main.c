#define LOG_LEVEL CONFIG_USB_DEVICE_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/can.h>
#include <zephyr/device.h>

/**
 * @brief Налаштування та запуск CAN пристрою
 * 
 * @param dev Вказівник на CAN пристрій
 * @param timing Вказівник на структуру з параметрами тайміну
 * @param name Назва пристрою для логування
 * @return int 0 при успіху, негативне значення при помилці
 */
static int can_setup_and_start(const struct device *dev, 
                                const struct can_timing *timing,
                                const char *name)
{
    int ret;

    if (!device_is_ready(dev)) {
        printk("%s device not ready\n", name);
        return -1;
    }

    ret = can_set_timing(dev, timing);
    if (ret != 0) {
        printk("Failed to set %s timing: %d\n", name, ret);
        return ret;
    }

    ret = can_set_mode(dev, CAN_MODE_NORMAL);
    if (ret != 0) {
        printk("Failed to set %s mode: %d\n", name, ret);
        return ret;
    }

    ret = can_start(dev);
    if (ret != 0) {
        printk("Failed to start %s: %d\n", name, ret);
        return ret;
    }

    printk("%s started successfully\n", name);
    return 0;
}

/**
 * @brief Відправка CAN фрейму
 * 
 * @param dev Вказівник на CAN пристрій
 * @param id Ідентифікатор CAN фрейму
 * @param data Вказівник на дані для відправки
 * @param dlc Довжина даних (0-8)
 * @param name Назва пристрою для логування
 * @return int 0 при успіху, негативне значення при помилці
 */
static int can_send_frame(const struct device *dev,
                          uint32_t id,
                          const uint8_t *data,
                          uint8_t dlc,
                          const char *name)
{
    int ret;
    struct can_frame frame = {
        .id = id,
        .dlc = dlc
    };

    if (data != NULL && dlc > 0) {
        memcpy(frame.data, data, MIN(dlc, 8));
    }

    ret = can_send(dev, &frame, K_MSEC(100), NULL, NULL);
    if (ret != 0) {
        printk("Failed to send %s frame: %d\n", name, ret);
        return ret;
    }

    printk("%s frame sent (ID: 0x%03X, DLC: %d)\n", name, id, dlc);
    return 0;
}

int main(void)
{
    int ret;
    
    printk("Hello World from minimal!\n");

    // Отримуємо пристрої CAN за правильними labels з DTS
    const struct device *can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
    const struct device *can2 = DEVICE_DT_GET(DT_NODELABEL(fdcan2));

    // Налаштування тайміну: sjw=1, prop_seg=0, phase_seg1=13, phase_seg2=2, prescaler=20
    struct can_timing timing = {
        .sjw = 1,
        .prop_seg = 0,
        .phase_seg1 = 13,
        .phase_seg2 = 2,
        .prescaler = 20
    };

    // ========== FDCAN1 ==========
    ret = can_setup_and_start(can1, &timing, "FDCAN1");
    if (ret != 0) {
        return ret;
    }

    uint8_t data1[] = {0x01};
    ret = can_send_frame(can1, 0x123, data1, 1, "FDCAN1");
    if (ret != 0) {
        printk("FDCAN1 send failed, continuing...\n");
    }

    // ========== FDCAN2 ==========
    ret = can_setup_and_start(can2, &timing, "FDCAN2");
    if (ret != 0) {
        return ret;
    }

    uint8_t data2[] = {0x01};
    ret = can_send_frame(can2, 0x123, data2, 1, "FDCAN2");
    if (ret != 0) {
        printk("FDCAN2 send failed, continuing...\n");
    }

    for(;;) {
        k_msleep(1000);
    }

    return 0;
}