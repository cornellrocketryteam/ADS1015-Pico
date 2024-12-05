#include "../ads1015.hpp"
#include "tusb.h"

#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13

ADS1015 ads(I2C_PORT);

int main() {

    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    while (!tud_cdc_connected()) {
        sleep_ms(500);
    }

    uint16_t data[3];
    std::vector<uint8_t> channels = {1, 2, 3};

    if (!ads.begin(DR_250SPS)) {
        printf("ADC initialization failed");
        return 1;
    }

    while (true) {
        if (ads.read_data(channels, data)) {
            for (size_t i = 0; i < channels.size(); ++i) {
                printf("Channel %zu: %d\n", i + 1, channels[i], data[i]);
            }
        } else {
            printf("Failed to read data\n");
        }
    }
    return 0;
}
