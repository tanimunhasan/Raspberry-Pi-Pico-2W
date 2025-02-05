# Add the executable and source files
add_executable(Zsx 
    Zsx.c
    comms.c 
    DynamentComms.c 
    halgpio.c 
    ModbusComms.c 

)

# Include the header files from the inc directory
target_include_directories(Zsx PRIVATE ${CMAKE_SOURCE_DIR}/inc)

# Link required Pico libraries
target_link_libraries(Zsx
    pico_stdlib
    hardware_adc
    hardware_dma
    hardware_i2c
    hardware_pio
    hardware_spi
    hardware_watchdog
    hardware_uart
    hardware_irq
    hardware_sync
)

# Enable extra outputs for Raspberry Pi Pico
pico_add_extra_outputs(Zsx)
