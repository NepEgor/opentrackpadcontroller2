target_sources(${PROJECT_NAME} PRIVATE
    sdcard_config.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
