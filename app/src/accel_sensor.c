#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <sensor/accel-sensor.h>


#define ACCEL_NODE       DT_ALIAS(mma8652fc)

static const struct device *dev = DEVICE_DT_GET(ACCEL_NODE);

void accel_motion_handler(const struct device *dev, const struct sensor_trigger *trig)
{
    printk("Motion detected!\n");
    // sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &(struct sensor_value){ .val1 = 50, .val2 = 0 });
}

const struct sensor_trigger trig_motion = {
    .chan = SENSOR_CHAN_ACCEL_XYZ,
    .type = SENSOR_TRIG_THRESHOLD,
};

static int test()
{
    return 1;
}

static int init()
{
    if (!device_is_ready(dev)) {
        printk("Accelerometer sensor device %s not ready\n", dev->name);
        return -ENODEV;
    }

    // Set sensitivity parameters
    sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_FULL_SCALE, &(struct sensor_value){ .val1 = 2, .val2 = 0 });
    sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &(struct sensor_value){ .val1 = 50, .val2 = 0 });

    // Set motion detection trigger
    int rc = sensor_trigger_set(dev, &trig_motion, accel_motion_handler);
    if (rc) {
        printk("Failed to set motion trigger (error %d)\n", rc);
        return rc;
    }

    printk("Accelerometer sensor device %s is ready\n", dev->name);
    return 0;
}

SYS_INIT(init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);