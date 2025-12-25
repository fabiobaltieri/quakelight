set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD quakelight)

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(quakelight)

target_sources(app PRIVATE
	src/blink.c
	src/leds.c
	src/main.c
	src/msg_listener.c
	src/net_led.c
)

target_sources_ifdef(CONFIG_WIFI app PRIVATE src/wifi_creds.c)
