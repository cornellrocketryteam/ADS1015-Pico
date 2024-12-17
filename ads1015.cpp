#include "ads1015.hpp"

ADS1015::ADS1015(i2c_inst_t *i2c_type) {
    i2c = i2c_type;
}

// TODO: replace writes and reads with timeouts

bool ADS1015::write_register(const uint8_t reg, const uint16_t val) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (val >> 8) & 0xFF;
    buf[2] = val & 0xFF;

    if (i2c_write_blocking(i2c, ADS1015_ADDR, buf, 3, true) < 1) {
        return false;
    }

    return true;
}

bool ADS1015::read_register(const uint8_t reg, uint8_t *val) {

    // Write the register address first
    if (i2c_write_blocking(i2c, ADS1015_ADDR, &reg, 1, true) < 1) {
        return false;
    }

    // Read the register value
    if (i2c_read_blocking(i2c, ADS1015_ADDR, val, 2, false) != 2) {
        return false;
    }

    return true;
}

bool ADS1015::begin(ads_data_rate_t dr) {

    data_rate = dr;

    // Calculate conversion delay in microseconds
    switch (data_rate) {
    case DR_128SPS:
        delay = 1000000 / std::ceil(128);
        break;
    case DR_250SPS:
        delay = 1000000 / std::ceil(250);
        break;
    case DR_490SPS:
        delay = 1000000 / std::ceil(490);
        break;
    case DR_920SPS:
        delay = 1000000 / std::ceil(920);
        break;
    case DR_1600SPS:
        delay = 1000000 / std::ceil(1600);
        break;
    case DR_2400SPS:
        delay = 1000000 / std::ceil(2400);
        break;
    case DR_3300SPS:
        delay = 1000000 / std::ceil(3300);
        break;
    }

    if (!configure_adc(MUX_SINGLE_0)) {
        return false;
    }

    return true;
}

bool ADS1015::configure_adc(ads_mux_t mux) {

    config = ADS1015_REG_CONFIG_CQUE_NONE |
             ADS1015_REG_CONFIG_CLAT_NONLAT |
             ADS1015_REG_CONFIG_CPOL_ACTVLOW |
             ADS1015_REG_CONFIG_CMODE_TRAD |
             data_rate |
             ADS1015_REG_CONFIG_MODE_SINGLE |
             gain | mux |
             ADS1015_REG_CONFIG_OS_SINGLE;

#ifdef DEBUG
    printf("Config: %x\n", config);
#endif

    // Write config register to the ADC
    if (!write_register(ADS1015_REG_POINTER_CONFIG, config)) {
        return false;
    }

    // Confirm bits in config register match bits in config value
    uint8_t val[2];
    if (!read_register(ADS1015_REG_POINTER_CONFIG, val)) {
        return false;
    }

    uint16_t result = (val[0] << 8) | val[1];
    if ((result & ~ADS1015_REG_CONFIG_OS_SINGLE) != (config & ~ADS1015_REG_CONFIG_OS_SINGLE)) {
        return false;
    }

    return true;
}

uint16_t ADS1015::read_single_ended(uint8_t channel) {
    if (channel > 3) {
        return 0;
    }

    ads_mux_t mux;

    // Get mux config corresponding to given channel
    switch (channel) {
    case 0:
        mux = MUX_SINGLE_0;
        break;
    case 1:
        mux = MUX_SINGLE_1;
        break;
    case 2:
        mux = MUX_SINGLE_2;
        break;
    case 3:
        mux = MUX_SINGLE_3;
        break;
    }

    // Update config
    if (!configure_adc(mux)) {
        return 0;
    }

    // Wait for the conversion to complete
    sleep_us(delay);

    // Read the conversion results
    uint8_t val[2];
    uint16_t result;
    if (read_register(ADS1015_REG_POINTER_CONVERT, val)) {
        result = (val[0] << 8) | val[1];
        result >>= 4;
#ifdef DEBUG
        printf("ADC Value: %u\n", result);
#endif
    } else {
        result = 0xFFF;
#ifdef VERBOSE
        printf("Failed to read register!\n");
#endif
    }

    return result;
}

bool ADS1015::read_data(const uint8_t *channels, size_t channels_size, uint16_t *data) {
    if (!data) {
        return false;
    }
    // Read data for each channel in channels
    for (size_t i = 0; i < channels_size; i++) {
        uint16_t result = read_single_ended(channels[i]);
        if (result == 0xFFF) {
            return false;
        }
        data[i] = result;
    }

    return true;
}
