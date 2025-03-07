# SPDX-License-Identifier: Apache-2.0

board_runner_args(nulink "-f")
# board_runner_args(pyocd "--target=m2354sjfae" "--frequency=4000000")
board_runner_args(pyocd "--target=m2354sjfae" "--frequency=8000000")
board_runner_args(jlink "--device=M2354SJFAE" "--speed=4000" "--reset-after-load")

include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/nulink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
# include(${ZEPHYR_BASE}/boards/common/canopen.board.cmake)

# board_set_flasher(jlink)
