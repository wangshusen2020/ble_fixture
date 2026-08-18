cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ble_beacon)

# 添加板型定义路径
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/boards)

target_sources(app PRIVATE src/main.c)
