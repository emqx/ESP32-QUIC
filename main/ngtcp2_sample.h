#ifndef NGTCP2_SAMPLE_H
#define NGTCP2_SAMPLE_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Configuration structure for QUIC client
typedef struct {
    const char *hostname;
    const char *port;
    const char *alpn;
} quic_client_config_t;

// Mutex for QUIC connection protection
extern SemaphoreHandle_t quic_mutex;

// Non-blocking QUIC client functions
int quic_client_init_with_config(const quic_client_config_t *config);
int quic_client_process(void);  // Non-blocking process function
bool quic_client_is_connected(void);
int quic_client_local_stream_avail(void);
void quic_client_cleanup(void);

// Thread-safe QUIC operations
int quic_client_write_safe(const uint8_t *data, size_t datalen);
int quic_client_read_safe(uint8_t *buffer, size_t buffer_size, size_t *bytes_read);

#endif
