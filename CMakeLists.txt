add_library(ADS1015-Pico INTERFACE)

target_sources(ADS1015-Pico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ADS1015.cpp
)

target_link_libraries(ADS1015-Pico INTERFACE
    pico_stdlib
    hardware_i2c
)