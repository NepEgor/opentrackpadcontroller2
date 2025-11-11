target_sources(${PROJECT_NAME} PRIVATE
    usb_descriptors.c
    usb_callbacks.c
    usb_device.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
