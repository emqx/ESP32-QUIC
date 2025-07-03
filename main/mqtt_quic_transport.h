#ifndef MQTT_QUIC_TRANSPORT_H
#define MQTT_QUIC_TRANSPORT_H

#include "core_mqtt.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

/**
 * @brief Information about the server to connect to.
 */
typedef struct ServerInfo
{
    const char *pHostName;
    uint16_t port;
    const char *pAlpn;  // Application Layer Protocol Negotiation
} ServerInfo_t;

/**
 * @brief MQTT over QUIC connection configuration.
 */
typedef struct MQTTQUICConfig
{
    uint32_t timeoutMs;
    bool nonBlocking;
} MQTTQUICConfig_t;

/**
 * @brief Network context for the transport implementation.
 */
typedef struct NetworkContext
{
    const ServerInfo_t *pServerInfo;
    const MQTTQUICConfig_t *pMqttQuicConfig;
    
    // Buffer for accumulating MQTT packet fragments
    uint8_t send_buffer[512];  // Buffer for outgoing MQTT packets
    size_t send_buffer_len;    // Current length of data in send buffer
    uint32_t expected_packet_length;  // Expected total length of current MQTT packet
    bool packet_length_determined;    // Whether we've determined the packet length
    bool is_mqtt_connect_packet;      // Whether this is a CONNECT packet
} NetworkContext_t;

BaseType_t mqtt_quic_transport_init(NetworkContext_t *pNetworkContext,
                                  const ServerInfo_t *pServerInfo,
                                  const MQTTQUICConfig_t *pMqttQuicConfig);

int32_t mqtt_quic_transport_recv(NetworkContext_t *pNetworkContext,
                              void *pBuffer,
                              size_t bytesToRecv);

int32_t mqtt_quic_transport_send(NetworkContext_t *pNetworkContext,
                              const void *pBuffer,
                              size_t bytesToSend);

// TransportInterface declaration
extern TransportInterface_t xTransportInterface;

// Add time function declaration
uint32_t mqtt_get_time_ms(void);

#endif /* MQTT_QUIC_TRANSPORT_H */
