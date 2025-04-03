#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <sensor/accel-sensor.h>


#define ACCEL_NODE       DT_ALIAS(accelerometer)

static const struct device *dev = DEVICE_DT_GET(ACCEL_NODE);

void accel_motion_handler_warn(const struct device *dev, const struct sensor_trigger *trig)
{
    printk("WARN Motion detected!\n");
    // sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &(struct sensor_value){ .val1 = 50, .val2 = 0 });
}

void accel_motion_handler_main(const struct device *dev, const struct sensor_trigger *trig)
{
    printk("MAIN Motion detected!\n");
    // sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &(struct sensor_value){ .val1 = 50, .val2 = 0 });
}

// const struct sensor_trigger trig_warn = {
//     .chan = SENSOR_CHAN_ACCEL_XYZ,
//     .type = ACCEL_SENSOR_TRIG_WARN,
// };

// const struct sensor_trigger trig_main = {
//     .chan = SENSOR_CHAN_ACCEL_XYZ,
//     .type = ACCEL_SENSOR_TRIG_MAIN,
// };

static int init()
{
    if (!device_is_ready(dev)) {
        printk("Accelerometer sensor device %s not ready\n", dev->name);
        return -ENODEV;
    } else {
        printk("Accelerometer sensor device %s is ready\n", dev->name);
    }
    // Set sensitivity parameters
    // sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_FULL_SCALE, &(struct sensor_value){ .val1 = 2, .val2 = 0 });
    // sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &(struct sensor_value){ .val1 = 50, .val2 = 0 });
    // accel_sensor_set_current_position_as_reference(dev);
    int rez = sensor_attr_set(dev, ACCEL_SENSOR_MODE, ACCEL_SENSOR_SPECIAL_ATTRS, &(struct sensor_value){ .val1 = ACCEL_SENSOR_MODE_ARMED, .val2 = 0 });
    printk("REZ IS %d \n", rez);
    // Set motion detection trigger
    // int rc = sensor_trigger_set(dev, &trig_warn, accel_motion_handler_warn);
    // if (rc) {
    //     printk("Failed to set WARN motion trigger (error %d)\n", rc);
    //     return rc;
    // }

    // int rc = sensor_trigger_set(dev, &trig_main, accel_motion_handler_main);
    // if (rc) {
    //     printk("Failed to set MAIN motion trigger (error %d)\n", rc);
    //     return rc;
    // }

    return 0;
}

SYS_INIT(init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);