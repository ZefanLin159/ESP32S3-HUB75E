idf_component_register(
    SRCS "hub75e_driver.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver heap esp_timer ESP32-HUB75-MatrixPanel-I2S-DMA
)
