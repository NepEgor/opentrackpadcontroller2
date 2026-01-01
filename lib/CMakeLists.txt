set(I2CDEVLIB_PATH ${CMAKE_CURRENT_LIST_DIR}/i2cdevlib/RP2040)

target_sources(${PROJECT_NAME} PRIVATE
    ${I2CDEVLIB_PATH}/I2Cdev/I2Cdev.cpp
    ${I2CDEVLIB_PATH}/MPU6050/MPU6050.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${I2CDEVLIB_PATH}/I2Cdev
    ${I2CDEVLIB_PATH}/MPU6050
)

add_subdirectory(sdcardlib/src)
target_link_libraries(${PROJECT_NAME}
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
)
