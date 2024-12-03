#ifndef ADS1015_HPP
#define ADS1015_HPP

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <cstdio>
#include <map>
#include <vector>

/* General Config */
#define ADS1015_ADDR (0x48)
#define ADS1015_REG_POINTER_CONFIG (0x01)
#define ADS1015_REG_POINTER_CONVERT (0x00)
#define ADS1015_CONVERSION_DELAY (1)
#define ADS1015_REG_CONFIG_CQUE_NONE (0x0003)
#define ADS1015_REG_CONFIG_CLAT_NONLAT (0x0000)
#define ADS1015_REG_CONFIG_CPOL_ACTVLOW (0x0000)
#define ADS1015_REG_CONFIG_CMODE_TRAD (0x0000)
#define ADS1015_REG_CONFIG_MODE_CONTINUOUS (0x0000) // Continuous conversion mode
#define ADS1015_REG_CONFIG_OS_SINGLE (0x8000)  // Single-conversion

/* Data Rate Config */
#define ADS1015_REG_CONFIG_DR_MASK (0x00E0)
#define ADS1015_REG_CONFIG_DR_128SPS (0x0000)  // 128 samples per second
#define ADS1015_REG_CONFIG_DR_250SPS (0x0020)  // 250 samples per second
#define ADS1015_REG_CONFIG_DR_490SPS (0x0040)  // 490 samples per second
#define ADS1015_REG_CONFIG_DR_920SPS (0x0060)  // 920 samples per second
#define ADS1015_REG_CONFIG_DR_1600SPS (0x0080)  // 1600 samples per second (default)
#define ADS1015_REG_CONFIG_DR_2400SPS (0x00A0)  // 2400 samples per second
#define ADS1015_REG_CONFIG_DR_3300SPS (0x00C0)  // 3300 samples per second

/* Gain Config */
#define ADS1015_REG_CONFIG_PGA_MASK (0x0E00)
#define ADS1015_REG_CONFIG_PGA_6_144V (0x0000)  // +/-6.144V range = Gain 2/3
#define ADS1015_REG_CONFIG_PGA_4_096V (0x0200)  // +/-4.096V range = Gain 1
#define ADS1015_REG_CONFIG_PGA_2_048V (0x0400)  // +/-2.048V range = Gain 2 (default)
#define ADS1015_REG_CONFIG_PGA_1_024V (0x0600)  // +/-1.024V range = Gain 4
#define ADS1015_REG_CONFIG_PGA_0_512V (0x0800)  // +/-0.512V range = Gain 8
#define ADS1015_REG_CONFIG_PGA_0_256V (0x0A00)  // +/-0.256V range = Gain 16

/* MUX Config */
#define ADS1015_REG_CONFIG_MUX_MASK (0x7000)
#define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000)  // Single-ended AIN0
#define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000)  // Single-ended AIN1
#define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000)  // Single-ended AIN2
#define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000)  // Single-ended AIN3

/* Data Rate mappings */
typedef enum {
    DR_128SPS = ADS1015_REG_CONFIG_DR_128SPS, // 128 samples per second
    DR_250SPS = ADS1015_REG_CONFIG_DR_250SPS, // 250 samples per second
    DR_490SPS = ADS1015_REG_CONFIG_DR_490SPS,  // 490 samples per second
    DR_920SPS = ADS1015_REG_CONFIG_DR_920SPS,  // 920 samples per second
    DR_1600SPS = ADS1015_REG_CONFIG_DR_1600SPS,  // 1600 samples per second (default)
    DR_2400SPS = ADS1015_REG_CONFIG_DR_2400SPS,  // 2400 samples per second
    DR_3300SPS = ADS1015_REG_CONFIG_DR_3300SPS
} adsDataRate_t;

/* Gain mappings */
typedef enum {
    GAIN_TWOTHIRDS = ADS1015_REG_CONFIG_PGA_6_144V,
    GAIN_ONE = ADS1015_REG_CONFIG_PGA_4_096V,
    GAIN_TWO = ADS1015_REG_CONFIG_PGA_2_048V,
    GAIN_FOUR = ADS1015_REG_CONFIG_PGA_1_024V,
    GAIN_EIGHT = ADS1015_REG_CONFIG_PGA_0_512V,
    GAIN_SIXTEEN = ADS1015_REG_CONFIG_PGA_0_256V
} adsGain_t;

/* MUX mappings */
typedef enum {
    MUX_MASK = ADS1015_REG_CONFIG_MUX_MASK,
    MUX_SINGLE_0 = ADS1015_REG_CONFIG_MUX_SINGLE_0,
    MUX_SINGLE_1 = ADS1015_REG_CONFIG_MUX_SINGLE_1,
    MUX_SINGLE_2 = ADS1015_REG_CONFIG_MUX_SINGLE_2,
    MUX_SINGLE_3 = ADS1015_REG_CONFIG_MUX_SINGLE_3
} adsMux_t;

/**
 * Representation of the ADS1015 sensor.
 */
class ADS1015 {
public:
    /**
     * Initializes an ADS1015 object on an I2C bus.
     * @param i2c_type The I2C bus that this sensor is on
     */
    ADS1015(i2c_inst_t *i2c_type);

    /**
     * Configures the ADC with data rate and default values.
     * @param data_rate The data rate to configure the ADC with
     */
    bool begin(adsDataRate_t data_rate = DR_250SPS);

    /**
     * Reads data from each channel passed in and returns data as a vector.
     * @param channels A vector of AIN channels to be read from
     */
    std::vector<uint16_t> read_data(std::vector<uint8_t> &channels);

private:
    /**
     * Write a value to a specified register.
     * @param reg The register to write to
     * @param val The value to write
     */
    bool write_register(const uint8_t reg, const uint16_t val);

    /**
     * Read the value from a specified register.
     * @param reg The register to read from
     * @param val The array to store the read value
     */
    bool read_register(const uint8_t reg, uint8_t *val);

    /**
     * Configures the ADC.
     * @param mux The MUX config indicating which channel to read from
     */
    bool configure_adc(adsMux_t mux, adsGain_t gain = GAIN_ONE);

    /**
     * Reads from a single AIN channel.
     * @param channel The AIN channel to read from
     * @param gain Optional parameter to set gain
     */
    uint16_t read_single_ended(uint8_t channel, adsGain_t gain = GAIN_ONE);

    /**
     * The config value for the ADC.
     */
    uint16_t config;

    /**
     * The data rate for the ADC.
     */
    adsDataRate_t data_rate;

    /**
     * Return value for I2C reads and writes.
     */
    int ret;

    /**
     * The I2C bus.
     */
    i2c_inst_t *i2c;
};

#endif // ADS1015_HPP