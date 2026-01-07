#define LOG_LEVEL CONFIG_USB_DEVICE_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/can.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/can/transceiver.h>

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

    printk("[%s] Sending frame (ID: 0x%03X, DLC: %d)...\n", name, id, dlc);
    
    ret = can_send(dev, &frame, K_MSEC(2000), NULL, NULL);
    if (ret != 0) {
        printk("[%s] ERROR: Failed to send frame: %d\n", name, ret);
        
        enum can_state state;
        struct can_bus_err_cnt err_cnt;
        if (can_get_state(dev, &state, &err_cnt) == 0) {
            printk("[%s] State: %d, TX errors: %d, RX errors: %d\n", 
                   name, state, err_cnt.tx_err_cnt, err_cnt.rx_err_cnt);
        }
        return ret;
    }

    printk("[%s] ✓ Frame sent successfully\n", name);
    return 0;
}

int main(void)
{
    const struct device *can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
    const struct device *can2 = DEVICE_DT_GET(DT_NODELABEL(fdcan2));
    struct can_timing timing = {
        .sjw = 1,
        .prop_seg = 0,
        .phase_seg1 = 13,
        .phase_seg2 = 2,
        .prescaler = 20
    };
    struct can_filter filter1 = {
        .flags = 0,
        .id = 0x123,
        .mask = CAN_STD_ID_MASK
    };
    struct can_filter filter2 = {
        .flags = 0,
        .id = 0x124,
        .mask = CAN_STD_ID_MASK
    };
 
    uint8_t data1[] = {0x01};
    uint8_t data2[] = {0x02};

    can_set_timing(can1, &timing);
    can_add_rx_filter(can1, can_rx_callback, "FDCAN1", &filter1);
    can_set_mode(can1, CAN_MODE_NORMAL);
    can_start(can1);
    
    can_set_timing(can2, &timing);
    can_add_rx_filter(can2, can_rx_callback, "FDCAN2", &filter2);
    can_set_mode(can2, CAN_MODE_NORMAL);
    can_start(can2);
    
    printk("Both can started\n");
    
    // int ret = can_send_frame(can1, 0x123, data1, 1, "FDCAN1");
    // if (ret != 0) {
    //     printk("FDCAN1 send failed, continuing...\n");
    // }
    int ret = can_send_frame(can2, 0x124, data2, 1, "FDCAN2");
    if (ret != 0) {
        printk("FDCAN2 send failed, continuing...\n");
    }

    // k_msleep(1000);

    // can_stop(can1);
    // can_stop(can2);
    
    for(;;) {
        k_msleep(1000);
    }

    return 0;
}