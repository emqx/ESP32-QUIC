#ifndef CORE_MQTT_CONFIG_H_
#define CORE_MQTT_CONFIG_H_

/* Standard include. */
#include <stdint.h>

/**
 * @brief The maximum number of MQTT PUBLISH messages that may be pending
 * acknowledgement at any time.
 *
 * QoS 1 and 2 MQTT PUBLISHes require acknowledgment from the server before
 * they can be completed. While they are awaiting the acknowledgment, the
 * client must maintain information about their state. The value of this
 * macro sets the limit on how many simultaneous PUBLISH messages can
 * be pending acknowledgment.
 */
#define MQTT_STATE_ARRAY_MAX_COUNT              10U

/**
 * @brief Number of milliseconds to wait for a ping response to a ping
 * request as part of the keep-alive mechanism.
 *
 * If a ping response is not received before this timeout, then
 * #MQTT_ProcessLoop will return #MQTTKeepAliveTimeout.
 */
#define MQTT_PINGRESP_TIMEOUT_MS                5000U

/**
 * @brief Maximum number of uint8_t values that can be stored in the MQTT buffer.
 */
#define MQTT_MAX_BUFFER_SIZE                    1024U

/**
 * @brief Set to 1 to enable broken behavior for some mqtt functionality, such as calling
 * the disconnect callback when calling #MQTT_Connect fails.
 *
 * This setting could be deprecated in the future.
 */
#define MQTT_COMPATIBILITY_MODE                 0

/**
 * @brief Set to 1 to enable interpretation of topic filters with wildcards.
 *
 * This setting can be used to enable or disable the use of wildcards such as '+' and '#'
 * in topic filters. When disabled, the topic filter string comparison becomes a direct
 * string comparison.
 */
#define MQTT_ENABLE_TOPIC_WILDCARD_MATCHING     1

#endif /* CORE_MQTT_CONFIG_H_ */