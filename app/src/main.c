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
#include <string.h>

/* ================= AUTO-BAUD SUPPORT ================= */

struct can_timing_entry {
    struct can_timing timing;
    uint32_t bitrate;
};

/* STM32H563, FDCAN kernel clock = 160 MHz */
static const struct can_timing_entry can_bitrate_table[] = {
    { { .sjw = 1, .prop_seg = 0, .phase_seg1 = 13, .phase_seg2 = 2, .prescaler = 10 }, 1000000 },
    { { .sjw = 1, .prop_seg = 0, .phase_seg1 = 13, .phase_seg2 = 2, .prescaler = 20 }, 500000  },
    { { .sjw = 1, .prop_seg = 0, .phase_seg1 = 13, .phase_seg2 = 2, .prescaler = 40 }, 250000  },
    { { .sjw = 1, .prop_seg = 0, .phase_seg1 = 13, .phase_seg2 = 2, .prescaler = 80 }, 125000  },
};

static volatile bool can_rx_seen;

static void can_rx_detect_cb(const struct device *dev,
                             struct can_frame *frame,
                             void *user_data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(frame);
    ARG_UNUSED(user_data);
    can_rx_seen = true;
}

static bool wait_for_can_rx(uint32_t timeout_ms)
{
    can_rx_seen = false;
    uint32_t start = k_uptime_get_32();

    while ((k_uptime_get_32() - start) < timeout_ms) {
        if (can_rx_seen) {
            return true;
        }
        k_msleep(5);
    }
    return false;
}

static int can_auto_baud(const struct device *can,
                         uint32_t *detected_bitrate)
{
    struct can_filter sniff_filter = {
        .flags = 0,
        .id = 0x000,
        .mask = 0x000, /* accept all */
    };

    printk("FDCAN auto-baud started...\n");

    for (int i = 0; i < ARRAY_SIZE(can_bitrate_table); i++) {

        printk("Trying bitrate %u...\n",
               can_bitrate_table[i].bitrate);

        can_stop(can);

        if (can_set_timing(can,
            &can_bitrate_table[i].timing)) {
            printk("can_set_timing failed\n");
            continue;
        }

        can_set_mode(can, CAN_MODE_LISTENONLY);

        can_add_rx_filter(can,
                          can_rx_detect_cb,
                          NULL,
                          &sniff_filter);

        if (can_start(can)) {
            printk("can_start failed\n");
            continue;
        }

        if (wait_for_can_rx(300)) {

            enum can_state state;
            struct can_bus_err_cnt err;

            can_get_state(can, &state, &err);

            printk("RX detected: state=%d tx=%d rx=%d\n",
                state, err.tx_err_cnt, err.rx_err_cnt);

            if (state != CAN_STATE_BUS_OFF) {

                printk("✔ Bitrate detected: %u\n",
                    can_bitrate_table[i].bitrate);

                can_stop(can);
                can_set_mode(can, CAN_MODE_NORMAL);
                can_start(can);

                if (detected_bitrate) {
                    *detected_bitrate = can_bitrate_table[i].bitrate;
                }
                return 0;
            }
        }

        can_stop(can);
        k_msleep(50);
    }

    printk("❌ Auto-baud failed\n");
    return -ENODEV;
}

/* ================= NORMAL CAN CALLBACK ================= */

static void can_rx_callback(const struct device *dev,
                            struct can_frame *frame,
                            void *user_data)
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
    struct can_frame frame = {
        .id = id,
        .dlc = dlc
    };

    if (data && dlc) {
        memcpy(frame.data, data, MIN(dlc, 8));
    }

    printk("[%s] Sending frame ID=0x%03X\n", name, id);

    int ret = can_send(dev, &frame, K_MSEC(2000), NULL, NULL);
    if (ret) {
        enum can_state state;
        struct can_bus_err_cnt err;

        printk("[%s] Send failed: %d\n", name, ret);
        if (!can_get_state(dev, &state, &err)) {
            printk("[%s] state=%d tx=%d rx=%d\n",
                   name, state, err.tx_err_cnt, err.rx_err_cnt);
        }
        return ret;
    }

    printk("[%s] ✓ Frame sent\n", name);
    return 0;
}

/* ============================= MAIN ============================= */

int main(void)
{
    const struct device *can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
    const struct device *can2 = DEVICE_DT_GET(DT_NODELABEL(fdcan2));

    struct can_timing fixed_timing = {
        .sjw = 1,
        .prop_seg = 0,
        .phase_seg1 = 13,
        .phase_seg2 = 2,
        .prescaler = 20 /* 500 kbit */
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

    printk("CAN init started\n");

    /* ---------- CAN1 : AUTO-BAUD ---------- */
    uint32_t detected_bitrate;
    if (can_auto_baud(can1, &detected_bitrate) == 0) {
        printk("CAN1 ready @ %u\n", detected_bitrate);
        can_add_rx_filter(can1,
                          can_rx_callback,
                          "FDCAN1",
                          &filter1);
    } else {
        printk("CAN1 auto-baud FAILED\n");
    }

    /* ---------- CAN2 : FIXED 500k ---------- */
    can_set_timing(can2, &fixed_timing);
    can_add_rx_filter(can2, can_rx_callback, "FDCAN2", &filter2);
    can_set_mode(can2, CAN_MODE_NORMAL);
    can_start(can2);

    printk("Both CAN started\n");
    k_msleep(1000);

    can_send_frame(can1, 0x123, data1, 1, "FDCAN1");
    can_send_frame(can2, 0x124, data2, 1, "FDCAN2");

    for (;;) {
        k_msleep(1000);
    }
}
