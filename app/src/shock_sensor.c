#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <sensor/shock_sensor/shock-sensor.h>
#include "shock_sensor_state.h"

#define SHOCK_NODE       DT_ALIAS(shock_sensor)

#define INITIAL_WARN_LEVEL 10
#define INITIAL_MAIN_LEVEL 60
#define MAX_WARN_LEVEL 60
#define MAX_MAIN_LEVEL 120
#define MIN_COARSENING_INTERVAL_MS 60000
#define MIN_TAP_INTERVAL_MS 5000
#define MAX_TAP_INTERVAL_SEC 30

static const struct device *dev = DEVICE_DT_GET(SHOCK_NODE);

struct Sensor_state shock_sensor_state;

void update_zones()
{
    if (shock_sensor_state.prev_warn_zone == shock_sensor_state.warn_zone && shock_sensor_state.prev_main_zone == shock_sensor_state.main_zone) return;
    struct sensor_value val = { .val1 = shock_sensor_state.warn_zone, .val2 = shock_sensor_state.main_zone };
    sensor_attr_set(dev, SENSOR_CHAN_PROX, SENSOR_ATTR_UPPER_THRESH, &val);
    shock_sensor_state.prev_warn_zone = shock_sensor_state.warn_zone;
    shock_sensor_state.prev_main_zone = shock_sensor_state.main_zone;
}

void reset_timer_handler(struct k_timer *timer)
{
    printk("Tap count: %d\n", shock_sensor_state.tap_count);
    shock_sensor_state.tap_count = 0;
    shock_sensor_state.warn_zone = INITIAL_WARN_LEVEL;
    shock_sensor_state.main_zone = INITIAL_MAIN_LEVEL;
    shock_sensor_state.last_coarsering_time = k_uptime_get();
    update_zones();
}

void coarsening()
{
    if (k_uptime_get() - shock_sensor_state.last_coarsering_time < MIN_COARSENING_INTERVAL_MS) return;
    shock_sensor_state.warn_zone *= 1.1;
    shock_sensor_state.main_zone *= 1.1;
    if (shock_sensor_state.warn_zone > MAX_WARN_LEVEL) shock_sensor_state.warn_zone = MAX_WARN_LEVEL;
    if (shock_sensor_state.main_zone > MAX_MAIN_LEVEL) shock_sensor_state.main_zone = MAX_MAIN_LEVEL;
    shock_sensor_state.last_coarsering_time = k_uptime_get();
    update_zones();
}

void register_tap()
{
    int64_t current_time = k_uptime_get();
    shock_sensor_state.tap_count++;
    if (current_time - shock_sensor_state.last_tap_time < MIN_TAP_INTERVAL_MS) {
        printk("Warning: Possible abuse detected, taps too frequent\n");
        //тут можна додати додаткові дії, які виконуються при занадто частому тапі
    } else if (shock_sensor_state.tap_count > 1) {
        coarsening();
    }

    shock_sensor_state.last_tap_time = current_time;
    k_timer_start(&shock_sensor_state.reset_timer, K_SECONDS(MAX_TAP_INTERVAL_SEC), K_NO_WAIT);
}

void sensor_warn_handler(const struct device *dev, const struct sensor_trigger *trig)
{
	printk("Tap (Warn)! %d\n", shock_sensor_state.tap_count + 1);
    register_tap();
}

void sensor_main_handler(const struct device *dev, const struct sensor_trigger *trig)
{
    printk("Tap (Main)! %d\n", shock_sensor_state.tap_count + 1);
    register_tap();
}

const struct sensor_trigger trig_warn = {
    .chan = SENSOR_CHAN_PROX,
    .type = SENSOR_TRIG_TAP,
};

const struct sensor_trigger trig_main = {
    .chan = SENSOR_CHAN_PROX,
    .type = SENSOR_TRIG_THRESHOLD,
};

void init_shock_sensor_state(struct Sensor_state *state) {
    state->tap_count = 0;
    state->warn_zone = INITIAL_WARN_LEVEL;
    state->main_zone = INITIAL_MAIN_LEVEL;
    state->prev_warn_zone = INITIAL_WARN_LEVEL;
    state->prev_main_zone = INITIAL_MAIN_LEVEL;
    state->last_coarsering_time = k_uptime_get();
    state->last_tap_time = k_uptime_get();
    k_timer_init(&state->reset_timer, reset_timer_handler, NULL);
}

static int init()
{
    if(!device_is_ready(dev)) {
        printk("ADC VBUS controller device %s not ready\n", dev->name);
        return -ENODEV;
    }

    init_shock_sensor_state(&shock_sensor_state);
    
    struct sensor_value val = { .val1 = shock_sensor_state.warn_zone, .val2 = shock_sensor_state.main_zone };
    sensor_attr_set(dev, SENSOR_CHAN_PROX, SENSOR_ATTR_UPPER_THRESH, &val);

    int rc = sensor_trigger_set(dev, &trig_warn, sensor_warn_handler);
    rc = sensor_trigger_set(dev, &trig_main, sensor_main_handler);

    printk("Shock sensor controller device %s is ready\n", dev->name);
    return 0;
}

SYS_INIT(init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
