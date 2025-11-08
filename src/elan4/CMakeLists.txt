target_sources(${PROJECT_NAME} PRIVATE
    elan4.cpp
    elan4poll.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(ps2)
