#include "hw_config.h"

static spi_t spi = {
    .hw_inst = spi0,
    .sck_gpio = 2,
    .mosi_gpio = 3,
    .miso_gpio = 4,
    .baud_rate = 125 * 1000 * 1000 / 8  // 15625000 Hz
    //.baud_rate = 125 * 1000 * 1000 / 6  // 20833333 Hz
    //.baud_rate = 125 * 1000 * 1000 / 4  // 31250000 Hz
    //.baud_rate = 125 * 1000 * 1000 / 2  // 62500000 Hz
};

static sd_spi_if_t spi_if = {
    .spi = &spi,
    .ss_gpio = 5
};

static sd_card_t sd_card = {
    .type = SD_IF_SPI,
    .spi_if_p = &spi_if
};

size_t sd_get_num() { return 1; }

/**
 * @brief Get a pointer to an SD card object by its number.
 *
 * @param[in] num The number of the SD card to get.
 *
 * @return A pointer to the SD card object, or @c NULL if the number is invalid.
 */
sd_card_t *sd_get_by_num(size_t num)
{
    if (0 == num)
    {
        return &sd_card;
    }
    else
    {
        return NULL;
    }
}
