target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    input_mapper.cpp
    gyro.cpp
    triggers.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(elan4)
add_subdirectory(usb_device)
add_subdirectory(touch_controls)
add_subdirectory(sdcard)
add_subdirectory(config)
