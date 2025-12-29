#define LOG_LEVEL CONFIG_USB_DEVICE_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/can.h>
#include <zephyr/device.h>

// Callback для отримання CAN фреймів
static void can_rx_callback(const struct device *dev, struct can_frame *frame, void *user_data)
{
    const char *name = (const char *)user_data;
    
    printk("%s received frame - ID: 0x%03X, DLC: %d, Data: ", 
           name, frame->id, frame->dlc);
    
    for (int i = 0; i < frame->dlc; i++) {
        printk("0x%02X ", frame->data[i]);
    }
    printk("\n");
}

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
    k_msleep(10);
    
    return 0;
}

static int can_setup_rx_filter(const struct device *dev, const char *name)
{
    struct can_filter filter = {
        .flags = 0,
        .id = 0x123,  // Приймаємо фрейми з ID 0x123
        .mask = CAN_STD_ID_MASK  // Точна відповідність ID
    };

    int filter_id = can_add_rx_filter(dev, can_rx_callback, (void *)name, &filter);
    if (filter_id < 0) {
        printk("Failed to add %s RX filter: %d\n", name, filter_id);
        return filter_id;
    }

    printk("%s RX filter added (ID: %d)\n", name, filter_id);
    return filter_id;
}

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

    ret = can_send(dev, &frame, K_MSEC(500), NULL, NULL);
    if (ret != 0) {
        printk("Failed to send %s frame: %d\n", name, ret);
        return ret;
    }

    printk("%s frame sent (ID: 0x%03X, DLC: %d)\n", name, id, dlc);
    return 0;
}

int first_can_test();
int second_can_test();
int together_can_test();


int main(void)
{
    int ret;
    
    printk("Hello World from minimal!\n");

    first_can_test();
    second_can_test();
    
    for(;;) {
        k_msleep(1000);
    }

    return 0;
}

int first_can_test(){
    int ret;

    const struct device *can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
    struct can_timing timing = {
        .sjw = 1,
        .prop_seg = 0,
        .phase_seg1 = 13,
        .phase_seg2 = 2,
        .prescaler = 20
    };
    ret = can_setup_and_start(can1, &timing, "FDCAN1");
    if (ret != 0) {
        return ret;
    }
     ret = can_setup_rx_filter(can1, "FDCAN1");
    if (ret < 0) {
        return ret;
    }
    printk("Waiting for CAN bus stabilization...\n");
    k_msleep(50);
    
    int8_t data1[] = {0x01};
    ret = can_send_frame(can1, 0x123, data1, 1, "FDCAN1");
    if (ret != 0) {
        printk("FDCAN1 send failed, continuing...\n");
        return ret;
    }

    k_msleep(100);

    ret = can_stop(can1);
    if (ret != 0) {
        printk("FDCAN1 stop failed, continuing...\n");
        return ret;
    }
    return 0;
}

int second_can_test(){
    int ret;

    const struct device *can2 = DEVICE_DT_GET(DT_NODELABEL(fdcan2));
    struct can_timing timing = {
        .sjw = 1,
        .prop_seg = 0,
        .phase_seg1 = 13,
        .phase_seg2 = 2,
        .prescaler = 20
    };
    ret = can_setup_and_start(can2, &timing, "FDCAN2");
    if (ret != 0) {
        return ret;
    }
     ret = can_setup_rx_filter(can2, "FDCAN2");
    if (ret < 0) {
        return ret;
    }
    printk("Waiting for CAN bus stabilization...\n");
    k_msleep(50);
    
    int8_t data2[] = {0x02};
    ret = can_send_frame(can2, 0x123, data2, 1, "FDCAN2");
    if (ret != 0) {
        printk("FDCAN2 send failed, continuing...\n");
        return ret;
    }

    k_msleep(100);

    ret = can_stop(can2);
    if (ret != 0) {
        printk("FDCAN2 stop failed, continuing...\n");
        return ret;
    }
    return 0;
}

int together_can_test(){
    int ret;
    
    const struct device *can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
    const struct device *can2 = DEVICE_DT_GET(DT_NODELABEL(fdcan2));

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

    // Додаємо RX фільтр для FDCAN1
    ret = can_setup_rx_filter(can1, "FDCAN1");
    if (ret < 0) {
        return ret;
    }

    // ========== FDCAN2 ==========
    ret = can_setup_and_start(can2, &timing, "FDCAN2");
    if (ret != 0) {
        return ret;
    }

    // Додаємо RX фільтр для FDCAN2
    ret = can_setup_rx_filter(can2, "FDCAN2");
    if (ret < 0) {
        return ret;
    }

    printk("Waiting for CAN bus stabilization...\n");
    k_msleep(50);

    // Тепер відправляємо фрейми
    uint8_t data1[] = {0x01};
    ret = can_send_frame(can1, 0x123, data1, 1, "FDCAN1");
    if (ret != 0) {
        printk("FDCAN1 send failed, continuing...\n");
        return ret;
    }

    k_msleep(100);  // Невелика затримка між відправками

    uint8_t data2[] = {0x02};
    ret = can_send_frame(can2, 0x123, data2, 1, "FDCAN2");
    if (ret != 0) {
        printk("FDCAN2 send failed, continuing...\n");
        return ret;
    }

    return 0;
}
