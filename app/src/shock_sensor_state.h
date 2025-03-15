#ifndef SHOCK_SENSOR_STATE_H
#define SHOCK_SENSOR_STATE_H

#include <zephyr/kernel.h>

struct Sensor_state {
    struct k_timer reset_timer;
    int tap_count;
    int warn_zone;
    int main_zone;
    int prev_warn_zone;
    int prev_main_zone;
    int64_t last_coarsering_time;
    int64_t last_tap_time;
};

#endif // SHOCK_SENSOR_STATE_H