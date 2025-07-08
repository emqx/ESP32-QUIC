idf_component_register(
    SRCS
        "coreMQTT/source/core_mqtt.c"
        "coreMQTT/source/core_mqtt_serializer.c"
        "coreMQTT/source/core_mqtt_state.c"
    INCLUDE_DIRS
        "coreMQTT/source/include"
        "coreMQTT/source/interface"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../main" 
    REQUIRES mbedtls
)