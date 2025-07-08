#include "mqtt_quic_transport.h"
#include "esp_log.h"
#include "ngtcp2_sample.h"
#include <string.h>

static const char *TAG = "MQTT_QUIC";

// Global transport interface
TransportInterface_t xTransportInterface = {0};

// Forward declarations for ngtcp2 client functions
extern struct client g_client;
extern int quic_client_write_safe(const uint8_t *data, size_t datalen);
extern int quic_client_read_safe(uint8_t *buffer, size_t buffer_size, size_t *bytes_read);
extern bool quic_client_is_connected(void);

// Time function required by MQTT
uint32_t mqtt_get_time_ms(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
}

/**
 * @brief Decode MQTT variable byte integer (remaining length)
 * @param data Pointer to the data buffer
 * @param data_len Length of the data buffer
 * @param remaining_length Pointer to store the decoded remaining length
 * @param bytes_used Pointer to store the number of bytes used for encoding
 * @return 0 on success, -1 on failure
 */
static int decode_mqtt_remaining_length(const uint8_t *data, size_t data_len, 
                                      uint32_t *remaining_length, size_t *bytes_used) {
    if (data_len < 2) {
        return -1; // Not enough data to decode
    }
    
    *remaining_length = 0;
    *bytes_used = 0;
    uint32_t multiplier = 1;
    
    for (size_t i = 1; i < data_len && i < 5; i++) { // Skip first byte (packet type)
        uint8_t byte = data[i];
        *remaining_length += (byte & 0x7F) * multiplier;
        (*bytes_used)++;
        
        if ((byte & 0x80) == 0) {
            // Most significant bit is 0, this is the last byte
            return 0;
        }
        
        multiplier *= 128;
        if (multiplier > 128 * 128 * 128) {
            return -1; // Malformed remaining length
        }
    }
    
    return -1; // Incomplete or malformed
}

/**
 * @brief Determine if we have enough data to know the complete MQTT packet length
 * @param context Network context containing the send buffer
 * @return true if packet length is determined, false otherwise
 */
static bool determine_mqtt_packet_length(NetworkContext_t *context) {
    if (context->packet_length_determined) {
        return true;
    }
    
    if (context->send_buffer_len < 2) {
        return false; // Need at least 2 bytes (packet type + 1 byte of remaining length)
    }
    
    uint32_t remaining_length;
    size_t bytes_used;
    
    if (decode_mqtt_remaining_length(context->send_buffer, context->send_buffer_len,
                                   &remaining_length, &bytes_used) == 0) {
        // Successfully decoded remaining length
        context->expected_packet_length = 1 + bytes_used + remaining_length; // 1 byte for packet type + bytes_used + remaining_length
        context->packet_length_determined = true;
        
        // Check if this is a CONNECT packet
        if (context->send_buffer[0] == 0x10) {
            context->is_mqtt_connect_packet = true;
        }
        
        ESP_LOGI(TAG, "Determined MQTT packet length: %u bytes (remaining_length=%u, bytes_used=%zu)", 
                 context->expected_packet_length, remaining_length, bytes_used);
        return true;
    }
    
    return false;
}

/**
 * @brief Send the complete MQTT packet over QUIC
 * @param context Network context containing the complete packet
 * @return 0 on success, -1 on failure
 */
static int send_complete_mqtt_packet(NetworkContext_t *context) {
    ESP_LOGI(TAG, "=== SENDING COMPLETE MQTT PACKET ===");
    ESP_LOGI(TAG, "Packet length: %zu bytes", context->send_buffer_len);
    ESP_LOGI(TAG, "Is CONNECT packet: %s", context->is_mqtt_connect_packet ? "YES" : "NO");
    
    // Validate packet before sending
    if (context->send_buffer_len == 0 || context->send_buffer_len > sizeof(context->send_buffer)) {
        ESP_LOGE(TAG, "Invalid packet length: %zu", context->send_buffer_len);
        return -1;
    }
    
    // Log the complete packet in hex
    char hex_str[257];
    size_t hex_len = (context->send_buffer_len > 128) ? 128 : context->send_buffer_len;
    
    for (size_t i = 0; i < hex_len; i++) {
        snprintf(&hex_str[i*2], 3, "%02x", context->send_buffer[i]);
    }
    hex_str[hex_len*2] = '\0';
    
    ESP_LOGI(TAG, "Complete MQTT packet hex (%zu bytes): %s%s", 
             context->send_buffer_len, hex_str, (context->send_buffer_len > 128) ? "..." : "");
    
    // Check if QUIC client is still connected
    extern bool quic_client_is_connected(void);
    if (!quic_client_is_connected()) {
        ESP_LOGE(TAG, "QUIC client is not connected, cannot send data");
        return -1;
    }
    
    // Add a small delay before sending to ensure QUIC is ready
    vTaskDelay(pdMS_TO_TICKS(10));
    
    int result = quic_client_write_safe(context->send_buffer, 
                                       context->send_buffer_len);
    
    if (result != 0) {
        ESP_LOGE(TAG, "Failed to send complete MQTT packet over QUIC, error %d", result);
        return -1;
    }
    
    ESP_LOGI(TAG, "Successfully sent complete MQTT packet (%zu bytes) over QUIC", context->send_buffer_len);
    return 0;
}

/**
 * @brief Reset the send buffer for a new packet
 * @param context Network context to reset
 */
static void reset_send_buffer(NetworkContext_t *context) {
    context->send_buffer_len = 0;
    context->expected_packet_length = 0;
    context->packet_length_determined = false;
    context->is_mqtt_connect_packet = false;
}

int32_t mqtt_quic_transport_send(NetworkContext_t *pNetworkContext,
                              const void *pBuffer,
                              size_t bytesToSend)
{
    ESP_LOGI(TAG, "=== TRANSPORT SEND CALLED ===");
    ESP_LOGI(TAG, "Function: %s", __func__);
    ESP_LOGI(TAG, "Parameters: pNetworkContext=%p, pBuffer=%p, bytesToSend=%zu", pNetworkContext, pBuffer, bytesToSend);
    
    if (pNetworkContext == NULL || pBuffer == NULL) {
        ESP_LOGE(TAG, "Invalid parameters: pNetworkContext=%p, pBuffer=%p", pNetworkContext, pBuffer);
        return -1;
    }

    if (bytesToSend == 0) {
        ESP_LOGW(TAG, "Attempting to send 0 bytes");
        return 0;
    }

    ESP_LOGI(TAG, "Received fragment of %zu bytes", bytesToSend);
    
    // Log the fragment in hex for debugging
    const uint8_t *data = (const uint8_t *)pBuffer;
    char hex_str[257];
    size_t hex_len = (bytesToSend > 128) ? 128 : bytesToSend;
    
    for (size_t i = 0; i < hex_len; i++) {
        snprintf(&hex_str[i*2], 3, "%02x", data[i]);
    }
    hex_str[hex_len*2] = '\0';
    
    ESP_LOGI(TAG, "Fragment hex (%zu bytes): %s%s", bytesToSend, hex_str, (bytesToSend > 128) ? "..." : "");
    
    // Check if this is the start of a new packet
    if (pNetworkContext->send_buffer_len == 0) {
        ESP_LOGI(TAG, "Starting new MQTT packet");
        
        // Check if this looks like a valid MQTT packet start
        if (data[0] == 0x10) {
            ESP_LOGI(TAG, "*** This looks like the start of an MQTT CONNECT packet! ***");
        } else {
            ESP_LOGI(TAG, "MQTT packet type: 0x%02x", data[0]);
        }
    }
    
    // Check if we have enough space in the send buffer
    if (pNetworkContext->send_buffer_len + bytesToSend > sizeof(pNetworkContext->send_buffer)) {
        ESP_LOGE(TAG, "Send buffer overflow! Current: %zu, Adding: %zu, Max: %zu",
                 pNetworkContext->send_buffer_len, bytesToSend, sizeof(pNetworkContext->send_buffer));
        return -1;
    }
    
    // Add this fragment to the send buffer
    memcpy(pNetworkContext->send_buffer + pNetworkContext->send_buffer_len, data, bytesToSend);
    pNetworkContext->send_buffer_len += bytesToSend;
    
    ESP_LOGD(TAG, "Added fragment to buffer. Total buffered: %zu bytes", pNetworkContext->send_buffer_len);
    
    // Try to determine the packet length if we haven't already
    if (!pNetworkContext->packet_length_determined) {
        if (determine_mqtt_packet_length(pNetworkContext)) {
            ESP_LOGD(TAG, "Determined packet length: %u bytes", pNetworkContext->expected_packet_length);
        } else {
            ESP_LOGD(TAG, "Still determining packet length, need more data");
        }
    }
    
    // Check if we have the complete packet
    if (pNetworkContext->packet_length_determined && 
        pNetworkContext->send_buffer_len >= pNetworkContext->expected_packet_length) {
        
        ESP_LOGD(TAG, "*** COMPLETE MQTT PACKET READY TO SEND ***");
        ESP_LOGD(TAG, "Expected: %u bytes, Buffered: %zu bytes",
                 pNetworkContext->expected_packet_length, pNetworkContext->send_buffer_len);
        
        // Send the complete packet
        int result = send_complete_mqtt_packet(pNetworkContext);
        
        // Reset buffer for next packet
        reset_send_buffer(pNetworkContext);
        
        if (result != 0) {
            ESP_LOGE(TAG, "Failed to send complete MQTT packet");
            return -1;
        }
        
        ESP_LOGI(TAG, "Successfully sent complete MQTT packet");
    } else {
        ESP_LOGI(TAG, "Packet not complete yet, continuing to buffer");
        if (pNetworkContext->packet_length_determined) {
            ESP_LOGD(TAG, "Need %u more bytes",
                     pNetworkContext->expected_packet_length - pNetworkContext->send_buffer_len);
        }
    }
    
    return (int32_t)bytesToSend;
}

int32_t mqtt_quic_transport_recv(NetworkContext_t *pNetworkContext,
                              void *pBuffer,
                              size_t bytesToRecv)
{
    if (pNetworkContext == NULL || pBuffer == NULL) {
        ESP_LOGE(TAG, "Invalid parameters: pNetworkContext=%p, pBuffer=%p", pNetworkContext, pBuffer);
        return -1;
    }
    
    ESP_LOGD(TAG, "Attempting to receive up to %zu bytes", bytesToRecv);
    
    size_t bytesReceived = 0;
    
    // Check if QUIC client is still connected
    extern bool quic_client_is_connected(void);
    if (!quic_client_is_connected()) {
        ESP_LOGW(TAG, "QUIC client is not connected, cannot receive data");
        return 0; // Return 0 to indicate no data available, not an error
    }
    
    int result = quic_client_read_safe((uint8_t *)pBuffer,
                                      bytesToRecv,
                                      &bytesReceived);
    
    if (result != 0) {
        if (result == -2) {  // No data available
            ESP_LOGD(TAG, "No data available from QUIC");
            return 0;
        }
        ESP_LOGE(TAG, "Failed to receive data over QUIC, error %d", result);
        return -1;
    }
    
    if (bytesReceived > 0) {
        ESP_LOGI(TAG, "=== RECEIVED %zu BYTES FROM QUIC ===", bytesReceived);
        
        // Log the received data in hex
        const uint8_t *data = (const uint8_t *)pBuffer;
        char hex_str[257];
        size_t hex_len = (bytesReceived > 128) ? 128 : bytesReceived;
        
        for (size_t i = 0; i < hex_len; i++) {
            snprintf(&hex_str[i*2], 3, "%02x", data[i]);
        }
        hex_str[hex_len*2] = '\0';
        
        ESP_LOGI(TAG, "Received packet hex (%zu bytes): %s%s", 
                 bytesReceived, hex_str, (bytesReceived > 128) ? "..." : "");
        
        // Try to identify the MQTT packet type
        if (bytesReceived > 0) {
            uint8_t packet_type = (data[0] >> 4) & 0x0F;
            const char* packet_name = "UNKNOWN";
            
            switch (packet_type) {
                case 1: packet_name = "CONNECT"; break;
                case 2: packet_name = "CONNACK"; break;
                case 3: packet_name = "PUBLISH"; break;
                case 4: packet_name = "PUBACK"; break;
                case 5: packet_name = "PUBREC"; break;
                case 6: packet_name = "PUBREL"; break;
                case 7: packet_name = "PUBCOMP"; break;
                case 8: packet_name = "SUBSCRIBE"; break;
                case 9: packet_name = "SUBACK"; break;
                case 10: packet_name = "UNSUBSCRIBE"; break;
                case 11: packet_name = "UNSUBACK"; break;
                case 12: packet_name = "PINGREQ"; break;
                case 13: packet_name = "PINGRESP"; break;
                case 14: packet_name = "DISCONNECT"; break;
            }
            
            ESP_LOGI(TAG, "*** MQTT Packet Type: %s (0x%02x) ***", packet_name, packet_type);
        }
    }
    
    return (int32_t)bytesReceived;
}

BaseType_t mqtt_quic_transport_init(NetworkContext_t *pNetworkContext,
                                  const ServerInfo_t *pServerInfo,
                                  const MQTTQUICConfig_t *pMqttQuicConfig)
{
    if (pNetworkContext == NULL || pServerInfo == NULL || pMqttQuicConfig == NULL) {
        return pdFAIL;
    }

    ESP_LOGI(TAG, "Initializing MQTT-over-QUIC transport");
    
    pNetworkContext->pServerInfo = pServerInfo;
    pNetworkContext->pMqttQuicConfig = pMqttQuicConfig;
    
    // Initialize the send buffer
    reset_send_buffer(pNetworkContext);
    
    return pdPASS;
}
