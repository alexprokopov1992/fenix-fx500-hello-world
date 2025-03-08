# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fenix-fx500-hello-world)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_DT_HAS_GPIO_LEDS_ENABLED app PRIVATE src/led.c)
target_sources_ifdef(CONFIG_DT_HAS_ZEPHYR_SHOCK_SENSOR_ENABLED app PRIVATE src/shock_sensor.c)
