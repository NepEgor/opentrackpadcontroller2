target_sources(${PROJECT_NAME} PRIVATE
    touch_control.cpp
    touch_dpad.cpp
    touch_joystick.cpp
    touch_mouse_joystick.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
