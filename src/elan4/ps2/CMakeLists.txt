target_sources(${PROJECT_NAME} PRIVATE
    ps2.cpp
    ps2.pio.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ps2.pio)
