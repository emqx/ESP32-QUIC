idf_component_register(SRCS "quic_demo_main.c" "ngtcp2_sample.c" "esp_ev_compat.c"
                       PRIV_REQUIRES spi_flash nvs_flash
                       REQUIRES ngtcp2 wolfssl 
                       INCLUDE_DIRS "")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")
