cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(keyboard C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
	main.c
	usb_descriptors.c
	keyboard.c
	PC_keyboard.c
	LayerKeyboard.c
	USART_keyboard.c
	)

pico_add_extra_outputs(${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME}
    pico_stdlib
	tinyusb_device 
	tinyusb_board
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

target_include_directories(${PROJECT_NAME} PRIVATE inc)