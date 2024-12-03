#include "ads1015.hpp"

ADS1015::ADS1015(i2c_inst_t *i2c_type) {
    i2c = i2c_type;
}

bool ADS1015::write_register(const uint8_t reg, const uint16_t val) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (val >> 8) & 0xFF;
    buf[2] = val & 0xFF;

    ret = i2c_write_blocking(i2c, ADS1015_ADDR, buf, 3, true);
    if (ret < 1) {
        return false;
    }

    return true;
}

bool ADS1015::read_register(const uint8_t reg, uint8_t *val) {

    // Write the register address first
    int ret = i2c_write_blocking(i2c, ADS1015_ADDR, &reg, 1, true);
    if (ret < 1) {
        return false;
    }

    // Read the register value
    ret = i2c_read_blocking(i2c, ADS1015_ADDR, val, 2, false);
    if (ret != 2) {
        return false;
    }

    return true;
}

bool ADS1015::begin(adsDataRate_t dr) {

    data_rate = dr;

    if (!configure_adc(MUX_MASK)) {
        return false;
    }

    return true;
}

bool ADS1015::configure_adc(adsMux_t mux, adsGain_t gain) {

    config = ADS1015_REG_CONFIG_CQUE_NONE | // Disable the comparator (default val)
             ADS1015_REG_CONFIG_CLAT_NONLAT | // Non-latching (default val)
             ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
             ADS1015_REG_CONFIG_CMODE_TRAD | // Traditional comparator (default val)
             data_rate | // Data rate
             ADS1015_REG_CONFIG_MODE_SINGLE |   // Single conversion mode
             gain | mux |
             ADS1015_REG_CONFIG_OS_SINGLE;  // Single-conversion

    printf("Config: %x\n", config);

    // Write config register to the ADC
    if (!write_register(ADS1015_REG_POINTER_CONFIG, config)) {
        return false;
    }

    sleep_ms(2); // Allow time for ADC to stabilize

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

uint16_t ADS1015::read_single_ended(uint8_t channel, adsGain_t gain) {
    if (channel > 3) {
        return 0;
    }
    adsMux_t mux;

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
    if (!configure_adc(mux, gain)) {
        return 0;
    }

    // Wait for the conversion to complete
    sleep_us(1000000 * ADS1015_CONVERSION_DELAY);

    // Read the conversion results
    uint8_t val[2];
    uint16_t result;
    if (read_register(ADS1015_REG_POINTER_CONVERT, val)) {
        result = (val[0] << 8) | val[1];
        result >>= 4;
        printf("ADC Value: %u\n", result);
    } else {
        result = 0xFFFF;
        printf("Failed to read register!\n");
    }

    return result;
}

std::vector<uint16_t> ADS1015::read_data(std::vector<uint8_t> &channels) {
    // Read data for each channel in channels
    std::vector<uint16_t> data;
    for (int channel : channels) {
        data.push_back(read_single_ended(channel));
    }

    return data;
}
