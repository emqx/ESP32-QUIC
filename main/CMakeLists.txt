idf_component_register(
    SRCS 
        "quic_demo_main.c"
        "ngtcp2_sample.c" 
        "esp_ev_compat.c"
        "mqtt_quic_transport.c"
    PRIV_REQUIRES 
        spi_flash 
        nvs_flash
    REQUIRES 
        ngtcp2 
        wolfssl 
        coreMQTT
        protocol_examples_common
    INCLUDE_DIRS "")

