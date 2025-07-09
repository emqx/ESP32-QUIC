# MQTT over QUIC Demo for ESP32-C3

This is a modified esp32 IDF hello_world example to prove QUIC could run on  
RISC-V [ESP32C3](https://www.espressif.com/en/products/socs/esp32-c3). 

## Components Used

- **[wolfSSL](https://components.espressif.com/components/wolfssl/wolfssl)**
  - TLS/SSL stack from ESP32 component registry
  - Provides secure cryptographic operations for QUIC

- **[ngtcp2](https://github.com/ngtcp2/ngtcp2)**
  - High-performance QUIC implementation (git submodule)
  - Patched for ESP32 compatibility and enhanced reliability

- **[coreMQTT](https://github.com/FreeRTOS/coreMQTT)**
  - AWS IoT Device SDK MQTT library
  - Provides robust MQTT protocol implementation

- **protocol_examples_common**
  - ESP-IDF WiFi example code for network connectivity

- **esp_timer**
  - ESP-IDF component for high-resolution timers and event loop implementation

## Features

### Core Functionality
1. **WiFi Connection Management** - Automatic WiFi connection with retry logic
2. **QUIC Connection Establishment** - Reliable QUIC handshake with proper error handling
3. **MQTT Communication** - Full MQTT protocol operations (connect, publish, subscribe)
4. **Graceful Shutdown** - Proper connection cleanup and resource management

## Continuous Integration

[![ESP32-QUIC CI](https://github.com/emqx/ESP32-QUIC/workflows/ESP32-QUIC%20CI/badge.svg)](https://github.com/emqx/ESP32-QUIC/actions)

This project includes a comprehensive CI/CD pipeline that:

- **Automated Building**: Builds the project with ESP-IDF for ESP32-C3 target
- **Matrix Testing**: Tests across multiple ESP-IDF versions (v5.1, v5.2)  
- **Automatic Patching**: Applies wolfSSL and ngtcp2 patches automatically
- **Artifact Generation**: Produces firmware binaries for each successful build
- **Quality Checks**: Runs pytest tests and basic code quality validation

### CI Workflows

1. **Main CI** (`ci.yml`) - Primary build and validation
2. **Matrix Build** (`build-matrix.yml`) - Multi-version compatibility testing  
3. **Tests** (`test.yml`) - Automated testing and code quality checks

The CI automatically handles the known wolfSSL session cache build issue and applies necessary patches.

For detailed CI documentation, see [CI Documentation](.github/CI.md).

## Usage Guide

### Prerequisites

1. **ESP-IDF Setup**: Follow the [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html) for your development board

We tested with ESP-IDF 6.0.0

### Installation Steps

1. **Clone Repository**
   ```bash
   git clone --recursive https://github.com/qzhuyan/POC-ESP32-QUIC.git
   cd demo_mqtt_over_quic
   ```

2. **Apply ngtcp2 Patches**
   ```bash
   patch -p 1 -d components/ngtcp2/ngtcp2 < ngtcp2.patch
   ```

3. **Configure QUIC Server Settings**
   
   Edit the server configuration in [ngtcp2_sample.c](main/ngtcp2_sample.c):
   ```c
   #define REMOTE_HOST "your.mqtt.server.com"
   #define REMOTE_PORT "14567"
   #define ALPN "\x4mqtt"
   ```

4. **Configure WiFi Credentials**
   ```bash
   idf.py menuconfig
   ```
   Navigate to: `Example Connection Configuration` → Set your WiFi SSID and password

5. **Build the Project**
   ```bash
   idf.py build
   ```
   
   **Note**: If you encounter `undefined reference to TlsSessionCacheGetAndRdLock` error:
   - Comment out `#define NO_SESSION_CACHE` in [managed_components/wolfssl__wolfssl/include/user_settings.h](managed_components/wolfssl__wolfssl/include/user_settings.h)
   - Retry the build

6. **Flash and Monitor**
   ```bash
   idf.py flash monitor
   ```

### Expected Build Output
```
quic_demo.bin binary size 0x16d060 bytes. Smallest app partition is 0x1a9000 bytes. 0x3bfa0 bytes (14%) free.
```

## Configuration Options

### WiFi Configuration
- **SSID/Password**: Set via menuconfig or modify [sdkconfig](sdkconfig)
- **Connection Retry**: Configurable retry attempts and timeouts
- **Security**: Support for various WiFi security protocols

### QUIC Configuration
- **Server Endpoint**: Hostname and port configuration
- **ALPN Protocol**: Application Layer Protocol Negotiation settings
- **Connection Parameters**: Timeout, retry, and buffer size settings

### MQTT Configuration
- **Client ID**: Automatically generated or custom configuration
- **Keep Alive**: Configurable keep-alive intervals
- **QoS Levels**: Support for all MQTT QoS levels
- **Topic Management**: Publish/Subscribe topic configuration 

## TODO: Comparison with TCP-based MQTT

| Feature | MQTT over QUIC | MQTT over TCP |
|---------|----------------|---------------|
| **Connection Establishment** | 0-RTT resumption | 3-way handshake |
| **Multiplexing** | Native stream multiplexing | Requires multiple connections |
| **Head-of-line Blocking** | Eliminated | Present |
| **Connection Migration** | Supported | Not supported |
| **Packet Loss Recovery** | Per-stream | Entire connection |
| **Bandwidth Efficiency** | Higher | Standard |
| **Implementation Complexity** | Higher | Lower |
| **Ecosystem Maturity** | Emerging | Mature |

## Hardware Requirements

### Minimum Requirements
- **MCU**: ESP32-C3 (RISC-V 160MHz)
- **RAM**: 400KB (320KB available for application)
- **Flash**: 2MB minimum (4MB recommended)
- **Network**: WiFi 802.11 b/g/n

## Troubleshooting

### Common Build Issues

**1. wolfSSL Session Cache Error**
```
undefined reference to 'TlsSessionCacheGetAndRdLock'
```
**Solution**: Comment out `#define NO_SESSION_CACHE` in wolfSSL user_settings.h

**2. Partition Table Too Small**
```
Error: app partition is too small
```
**Solution**: Use the provided custom partitions.csv with larger app partition

**3. Submodule Not Found**
```
fatal: not a git repository (or any of the parent directories)
```
**Solution**: Clone with `--recursive` flag or run `git submodule update --init --recursive`

### Runtime Issues

**1. WiFi Connection Failures**
- Check SSID/password in menuconfig
- Verify WiFi signal strength
- Check for WiFi security protocol compatibility

**2. QUIC Connection Timeouts**
- Verify MQTT server is running and accessible
- Check firewall settings on server for **UDP***
- Ensure ALPN protocol matches server configuration

**3. Memory Issues**
- Monitor heap usage with `esp_get_free_heap_size()`
- Adjust buffer sizes in configuration
- Check for memory leaks in custom code

### Debug Configuration

Enable detailed logging by modifying log levels:
```c
esp_log_level_set("QUIC", ESP_LOG_DEBUG);
esp_log_level_set("MQTT", ESP_LOG_DEBUG);
```
## Project Structure

```
main/
├── core_mqtt_config.h      CoreMQTT configuration and settings
├── esp_ev_compat.c         libev compatibility layer for ESP event loop
├── esp_ev_compat.h         Event loop compatibility headers
├── idf_component.yml       Component dependencies definition
├── mqtt_quic_transport.c   MQTT transport layer over QUIC implementation
├── mqtt_quic_transport.h   MQTT transport layer header definitions
├── ngtcp2_sample.c         Enhanced ngtcp2 client with thread safety and error handling
├── ngtcp2_sample.h         ngtcp2 client header definitions
└── quic_demo_main.c        Main application entry point and MQTT demo logic

components/
├── coreMQTT/               AWS IoT CoreMQTT library integration
└── ngtcp2/                 ngtcp2 QUIC implementation as IDF component

Configuration Files:
├── ngtcp2.patch           ESP32-specific patches for ngtcp2
├── partitions.csv         Custom partition table (larger app partition)
└── sdkconfig              Platform-specific configuration
```

## Configuration Details

### Memory Configuration
- **Partition Table**: Custom partition layout with larger app partition (1700K)
- **Stack Sizes**: Tuned stack sizes for optimal memory usage
- **Buffer Management**: Optimized buffer allocation for QUIC streams

### Security Settings
- **TLS Configuration**: wolfSSL integration with ESP32 crypto acceleration
- **Certificate Handling**: Configurable certificate verification (disabled for demo)
- **Cryptographic Operations**: Hardware-accelerated encryption/decryption

### Performance Tuning
- **CPU Frequency**: 160MHz operation for optimal performance
- **WiFi Optimization**: Tuned WiFi parameters for reliability
- **QUIC Parameters**: Optimized congestion control and flow control settings

## Custom Partition Configuration

The project uses a custom partition table to accommodate the larger application size required by QUIC and MQTT libraries:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
# Note: if you have increased the bootloader size, make sure to update the offsets to avoid overlap
nvs,      data, nvs,     ,        0x6000,
phy_init, data, phy,     ,        0x1000,
factory,  app,  factory, ,        1700K,
```

### Partition Details
- **NVS (Non-Volatile Storage)**: 24KB for WiFi credentials and configuration
- **PHY Init**: 4KB for RF calibration data
- **Factory App**: 1700KB for the main application (significantly larger than default 1MB)

### Why Custom Partitions?
The default ESP32-C3 partition table provides only ~1MB for the application, which is insufficient for:
- wolfSSL TLS implementation (~200KB)
- ngtcp2 QUIC library (~300KB)
- coreMQTT implementation (~100KB)
- ESP-IDF framework overhead (~400KB)
- Application code and data structures

The custom 1700KB factory partition provides adequate space for all components with room for future enhancements.

## Logs

### ESP32C3

<details>

<summary> monitor log </summary>

``` 
Rebooting...
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0xc (RTC_SW_CPU_RST),boot:0xc (SPI_FAST_FLASH_BOOT)
Saved PC:0x4038079a
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5820,len:0x15c0
load:0x403cbf10,len:0xd2c
load:0x403ce710,len:0x34ec
entry 0x403cbf28
I (24) boot: ESP-IDF v6.0-dev-527-gadcbdd7da4 2nd stage bootloader
I (24) boot: compile time Jun 30 2025 15:39:14
I (24) boot: chip revision: v0.4
I (25) boot: efuse block revision: v1.3
I (29) boot.esp32c3: SPI Speed      : 80MHz
I (33) boot.esp32c3: SPI Mode       : DIO
I (37) boot.esp32c3: SPI Flash Size : 2MB
I (40) boot: Enabling RNG early entropy source...
I (45) boot: Partition Table:
I (47) boot: ## Label            Usage          Type ST Offset   Length
I (54) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (60) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (67) boot:  2 factory          factory app      00 00 00010000 001a9000
I (73) boot: End of partition table
I (77) esp_image: segment 0: paddr=00010020 vaddr=3c140020 size=3cbb4h (248756) map
I (125) esp_image: segment 1: paddr=0004cbdc vaddr=3fc96000 size=02f14h ( 12052) load
I (127) esp_image: segment 2: paddr=0004faf8 vaddr=40380000 size=00520h (  1312) load
I (129) esp_image: segment 3: paddr=00050020 vaddr=42000020 size=131f64h (1253220) map
I (340) esp_image: segment 4: paddr=00181f8c vaddr=40380520 size=15984h ( 88452) load
I (357) esp_image: segment 5: paddr=00197918 vaddr=50000000 size=00020h (    32) load
I (364) boot: Loaded app from partition at offset 0x10000
I (364) boot: Disabling RNG early entropy source...
I (375) cpu_start: Unicore app
I (383) cpu_start: Pro cpu start user code
I (383) cpu_start: cpu freq: 160000000 Hz
I (383) app_init: Application information:
I (383) app_init: Project name:     quic_demo
I (387) app_init: App version:      57f4b92-dirty
I (392) app_init: Compile time:     Jul  7 2025 15:19:17
I (397) app_init: ELF file SHA256:  9ede74013...
I (401) app_init: ESP-IDF:          v6.0-dev-527-gadcbdd7da4
I (407) efuse_init: Min chip rev:     v0.3
I (410) efuse_init: Max chip rev:     v1.99
I (414) efuse_init: Chip rev:         v0.4
I (418) heap_init: Initializing. RAM available for dynamic allocation:
I (425) heap_init: At 3FC9EFE0 len 00021020 (132 KiB): RAM
I (430) heap_init: At 3FCC0000 len 0001C710 (113 KiB): Retention RAM
I (436) heap_init: At 3FCDC710 len 00002950 (10 KiB): Retention RAM
I (442) heap_init: At 50000020 len 00001FC8 (7 KiB): RTCRAM
I (448) spi_flash: detected chip: generic
I (451) spi_flash: flash io: dio
W (454) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (466) sleep_gpio: Configure to isolate all GPIO pins in sleep state
I (472) sleep_gpio: Enable automatic switching of GPIO sleep configuration
I (479) main_task: Started on CPU0
I (479) main_task: Calling app_main()
I (479) simple_connect_example: Initializing...
I (489) simple_connect_example: Connecting to WiFi...
I (489) example_connect: Start example_connect.
I (489) pp: pp rom version: 9387209
I (499) net80211: net80211 rom version: 9387209
I (509) wifi:wifi driver task: 3fcaa9b8, prio:23, stack:6656, core=0
I (509) wifi:wifi firmware version: 35ceb4f
I (509) wifi:wifi certification version: v7.0
I (519) wifi:config NVS flash: enabled
I (519) wifi:config nano formatting: disabled
I (519) wifi:Init data frame dynamic rx buffer num: 32
I (529) wifi:Init static rx mgmt buffer num: 5
I (529) wifi:Init management short buffer num: 32
I (539) wifi:Init dynamic tx buffer num: 32
I (539) wifi:Init static tx FG buffer num: 2
I (549) wifi:Init static rx buffer size: 1600
I (549) wifi:Init static rx buffer num: 10
I (549) wifi:Init dynamic rx buffer num: 32
I (559) wifi_init: rx ba win: 6
I (559) wifi_init: accept mbox: 6
I (559) wifi_init: tcpip mbox: 32
I (569) wifi_init: udp mbox: 6
I (569) wifi_init: tcp mbox: 6
I (569) wifi_init: tcp tx win: 5760
I (569) wifi_init: tcp rx win: 5760
I (579) wifi_init: tcp mss: 1440
I (579) wifi_init: WiFi IRAM OP enabled

I (589) phy_init: phy_version 1201,bae5dd99,Mar  3 2025,15:36:21
I (629) wifi:mode : sta (f0:f5:bd:de:e6:1c)
I (629) wifi:enable tsf
I (629) example_connect: Connecting to doghouse...
W (629) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
I (639) example_connect: Waiting for IP(s)
I (3159) wifi:new:<3,0>, old:<1,0>, ap:<255,255>, sta:<3,0>, prof:1, snd_ch_cfg:0x0
I (3159) wifi:state: init -> auth (0xb0)
I (3169) wifi:state: auth -> assoc (0x0)
I (3169) wifi:state: assoc -> run (0x10)
I (3179) wifi:<ba-add>idx:0 (ifx:0, 04:d9:f5:1a:87:b0), tid:0, ssn:0, winSize:64
I (3199) wifi:connected with Wifi-AP, aid = 6, channel 3, BW20, bssid = 04:d9:f5:11:88:b2
I (3199) wifi:security: WPA2-PSK, phy: bgn, rssi: -36
I (3199) wifi:pm start, type: 1

I (3199) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (3209) wifi:set rx beacon pti, rx_bcn_pti: 0, bcn_timeout: 25000, mt_pti: 0, mt_time: 10000
I (3289) wifi:AP's beacon interval = 102400 us, DTIM period = 3
I (4229) esp_netif_handlers: example_netif_sta ip: 192.168.1.252, mask: 255.255.255.0, gw: 192.168.1.1
I (4229) example_connect: Got IPv4 event: Interface "example_netif_sta" address: 192.168.1.252
I (4489) example_connect: Got IPv6 event: Interface "example_netif_sta" address: fe80:0000:0000:0000:f2f5:bdff:fede:e61c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4489) example_common: Connected to example_netif_sta
I (4489) example_common: - IPv4 address: 192.168.1.252,
I (4499) example_common: - IPv6 address: fe80:0000:0000:0000:f2f5:bdff:fede:e61c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (4509) simple_connect_example: WiFi connected, starting combined QUIC+MQTT task...
I (4519) simple_connect_example: Free heap before task creation: 189452 bytes
I (4519) quic_demo_main: Starting combined QUIC+MQTT task
I (4529) quic_demo_main: Free heap at task start: 160436 bytes
I (4529) quic_demo_main: Initializing QUIC client with x.x.x.x:14567
I (4539) quic_demo_main: Free heap before QUIC init: 160436 bytes
I (4549) QUIC: QUIC mutex created successfully
I (4549) QUIC: QUIC client config: x.x.x.x:14567 with ALPN mqtt
I (4559) QUIC: init random number generator
I (4559) QUIC: init client ...
I (4569) QUIC: Set ALPN: mqtt (length: 4)
I (4569) QUIC: In client_quic_init
I (4569) QUIC: ===>  INITIAL TS: 479
I00000000 0x con next skip pkn=132
I (4579) ESP_EV_COMPAT: Starting IO watcher for fd 54
I (4579) QUIC: QUIC client initialization completed
I (4589) quic_demo_main: QUIC client initialized, waiting for connection...
I (4589) quic_demo_main: Free heap after QUIC init: 64792 bytes
I00000028 0x pkt tx pkn=0 dcid=0x3f5ddd637dec6d92 scid=0x version=0x00000001 type=Initial len=0
I00000028 0x frm tx 0 Initial CRYPTO(0x06) offset=0 len=321
I00000028 0x frm tx 0 Initial PADDING(0x00) len=838
I00000028 0x ldc loss_detection_timer=5154787000 timeout=999
I (5029) QUIC: QUIC connection established!
E (5059) task_wdt: esp_task_wdt_reset(707): task not found
I00000484 0x con recv packet len=1220
I00000484 0x pkt rx pkn=0 dcid=0x scid=0xa175f23f5e5d6857bb version=0x00000001 type=Initial len=152
I00000484 0x frm rx 0 Initial ACK(0x02) largest_ack=0 ack_delay=0(0) ack_range_count=0
I00000484 0x frm rx 0 Initial ACK(0x02) range=[0..0] len=0
I00000484 0x ldc latest_rtt=456 min_rtt=456 smoothed_rtt=456 rttvar=228 ack_delay=0
I00000484 0x con path is not ECN capable
I00000484 0x cca 1200 bytes acked, slow start cwnd=13200
I00000484 0x ldc loss_detection_timer=5982191000 timeout=1369
I00000484 0x frm rx 0 Initial CRYPTO(0x06) offset=0 len=123
I00000484 0x con the negotiated version is 0x00000001
I (5139) quic_demo_main: QUIC connection established! Waiting a bit more for stability...
I00000484 0x pkt read packet 171 left 1049
I00000484 0x pkt rx pkn=1 dcid=0x scid=0xa175f23f5e5d6857bb version=0x00000001 type=Handshake len=1031
I00000484 0x frm rx 1 Handshake CRYPTO(0x06) offset=0 len=1006
I00000484 0x frm rx 1 Handshake PADDING(0x00) len=1
I00000484 0x pkt read packet 1049 left 0
I00000484 0x con processing buffered handshake packet
I00001018 0x con recv packet len=306
I00001018 0x pkt rx pkn=2 dcid=0x scid=0xa175f23f5e5d6857bb version=0x00000001 type=Handshake len=263
I00001018 0x frm rx 2 Handshake CRYPTO(0x06) offset=1006 len=238
I00001018 0x cry remote transport_parameters stateless_reset_token=0x517356096616fbeb0496d4ddd24bb58d
I00001018 0x cry remote transport_parameters original_destination_connection_id=0x3f5ddd637dec6d92
I00001018 0x cry remote transport_parameters initial_source_connection_id=0xa175f23f5e5d6857bb
I00001018 0x cry remote transport_parameters initial_max_stream_data_bidi_local=65536
I00001018 0x cry remote transport_parameters initial_max_stream_data_bidi_remote=65536
I00001018 0x cry remote transport_parameters initial_max_stream_data_uni=65536
I00001018 0x cry remote transport_parameters initial_max_data=16777216
I00001018 0x cry remote transport_parameters initial_max_streams_bidi=0
I00001018 0x cry remote transport_parameters initial_max_streams_uni=0
I00001018 0x cry remote transport_parameters max_idle_timeout=0
I00001018 0x cry remote transport_parameters max_udp_payload_size=1472
I00001018 0x cry remote transport_parameters ack_delay_exponent=8
I00001018 0x cry remote transport_parameters max_ack_delay=26
I00001018 0x cry remote transport_parameters active_connection_id_limit=4
I00001018 0x cry remote transport_parameters disable_active_migration=0
I00001018 0x cry remote transport_parameters max_datagram_frame_size=0
I00001018 0x cry remote transport_parameters grease_quic_bit=0
I00001018 0x pkt read packet 281 left 25
I00001018 0x con processing buffered handshake packet
I (5779) QUIC: QUIC handshake completed callback triggered!
I00001018 0x con processing buffered protected packet
I00001018 0x pkt rx pkn=3 dcid=0x type=1RTT k=0
I00001018 0x frm rx 3 1RTT MAX_STREAMS(0x12) max_streams=10
I (5789) QUIC: Extending max local streams bidi to 10

I00001018 0x frm rx 3 1RTT MAX_STREAMS(0x13) max_streams=1
I00001018 0x pkt read packet 25 left 0
I00001237 0x pkt tx pkn=1 dcid=0xa175f23f5e5d6857bb scid=0x version=0x00000001 type=Initial len=0
I00001237 0x frm tx 1 Initial ACK(0x02) largest_ack=0 ack_delay=0(0) ack_range_count=0
I00001237 0x frm tx 1 Initial ACK(0x02) range=[0..0] len=0
I00001237 0x pkt tx pkn=0 dcid=0xa175f23f5e5d6857bb scid=0x version=0x00000001 type=Handshake len=0
I00001237 0x frm tx 0 Handshake ACK(0x02) largest_ack=2 ack_delay=0(0) ack_range_count=0
I00001237 0x frm tx 0 Handshake ACK(0x02) range=[2..1] len=1
I00001237 0x frm tx 0 Handshake CRYPTO(0x06) offset=0 len=52
I00001237 0x ldc loss_detection_timer=6735157000 timeout=1369
I00001237 0x con discarding Initial packet number space
I00001237 0x ldc loss_detection_timer=6735157000 timeout=1369
I00001237 0x pkt tx pkn=0 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00001237 0x frm tx 0 1RTT ACK(0x02) largest_ack=3 ack_delay=218(27341) ack_range_count=0
I00001237 0x frm tx 0 1RTT ACK(0x02) range=[3..3] len=0
I00001237 0x frm tx 0 1RTT PADDING(0x00) len=1025
I00001237 0x ldc loss_detection_timer=6735157000 timeout=1369
I00001237 0x con sending PMTUD probe packet len=1406
I00001237 0x pkt tx pkn=1 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00001237 0x frm tx 1 1RTT PING(0x01)
I00001237 0x frm tx 1 1RTT PADDING(0x00) len=1378
I00001237 0x ldc loss_detection_timer=6735157000 timeout=1369
E (5949) task_wdt: esp_task_wdt_reset(707): task not found
I00001383 0x con recv packet len=1220
I00001383 0x pkt rx pkn=4 dcid=0x type=1RTT k=0
I00001383 0x frm rx 4 1RTT ACK(0x02) largest_ack=0 ack_delay=0(0) ack_range_count=0
I00001383 0x frm rx 4 1RTT ACK(0x02) range=[0..0] len=0
I00001383 0x cca 1060 bytes acked, slow start cwnd=14260
I00001383 0x ldc loss_detection_timer=6735157000 timeout=1223
I00001383 0x frm rx 4 1RTT HANDSHAKE_DONE(0x1e)
I00001383 0x con discarding Handshake packet number space
I00001383 0x ldc loss_detection_timer=6761157000 timeout=1249
I00001383 0x ldc loss_detection_timer=6761157000 timeout=1249
I00001383 0x frm rx 4 1RTT NEW_CONNECTION_ID(0x18) seq=1 cid=0xa175b24559925baeb0 retire_prior_to=0 stateless_reset_token=0x94c0cd4f883fbbc50f94f26e99572fda
I00001383 0x frm rx 4 1RTT PADDING(0x00) len=1164
I00001383 0x pkt read packet 1220 left 0
I00001456 0x con recv packet len=1220
I00001456 0x pkt rx pkn=5 dcid=0x type=1RTT k=0
I00001456 0x frm rx 5 1RTT PING(0x01)
I00001456 0x frm rx 5 1RTT PADDING(0x00) len=1198
I00001456 0x pkt read packet 1220 left 0
I00001475 0x con recv packet len=314
I00001475 0x pkt rx pkn=6 dcid=0x type=1RTT k=0
I00001475 0x frm rx 6 1RTT CRYPTO(0x06) offset=0 len=289
I00001475 0x pkt read packet 314 left 0
I00001491 0x con recv packet len=27
I00001491 0x pkt rx pkn=7 dcid=0x type=1RTT k=0
I00001491 0x frm rx 7 1RTT ACK(0x02) largest_ack=1 ack_delay=25(99) ack_range_count=0
I00001491 0x frm rx 7 1RTT ACK(0x02) range=[1..0] len=1
I00001491 0x ldc latest_rtt=254 min_rtt=254 smoothed_rtt=431 rttvar=221 ack_delay=25
I00001491 0x cca 1406 bytes acked, slow start cwnd=15666
I00001491 0x ldc loss detection timer canceled
I00001491 0x pkt read packet 27 left 0
I00001531 0x con sending PMTUD probe packet len=1444
I00001531 0x pkt tx pkn=2 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00001531 0x frm tx 2 1RTT PING(0x01)
I00001531 0x frm tx 2 1RTT PADDING(0x00) len=1416
I00001531 0x ldc loss_detection_timer=7004148625 timeout=1344
I00001531 0x pkt tx pkn=3 dcid=0xa175f23f5e5d6857bb type=1RTT k=0

I00001531 0x frm tx 3 1RTT ACK(0x02) range=[7..4] len=3
I (6139) MQTT_QUIC: Initializing MQTT-over-QUIC transport
I (6149) quic_demo_main: Transport interface configured:
I (6149) quic_demo_main:   pNetworkContext: 0x3fcb9790
E (6159) task_wdt: esp_task_wdt_reset(707): task not found
I (6159) quic_demo_main:   recv function: 0x4200f2a2
I (6169) quic_demo_main:   send function: 0x4200ef16
I (6169) quic_demo_main: MQTT initialized, connecting to broker...
I (6179) quic_demo_main: About to call MQTT_Connect with:
I (6189) quic_demo_main:   Client ID: esp32_quic_client
I (6189) quic_demo_main:   Clean session: true
I (6189) quic_demo_main:   QUIC connected: true
I (6199) quic_demo_main:   Free heap: 39564 bytes
I (6199) quic_demo_main: Calling MQTT_Connect with tieout...
I (6209) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (6209) MQTT_QUIC: Function: mqtt_quic_transport_send
I (6219) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3fcb9498, bytesToSend=12
I (6229) MQTT_QUIC: Received fragment of 12 bytes
I (6229) MQTT_QUIC: Fragment hex (12 bytes): 101d00044d51545404020000
I (6239) MQTT_QUIC: Starting new MQTT packet
I (6239) MQTT_QUIC: *** This looks like the start of an MQTT CONNECT packet! ***
I (6249) MQTT_QUIC: Determined MQTT packet length: 31 bytes (remaining_length=29, bytes_used=1)
I (6259) MQTT_QUIC: Packet not complete yet, continuing to buffer
I (6259) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (6269) MQTT_QUIC: Function: mqtt_quic_transport_send
I (6269) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3fcb94b8, bytesToSend=2
I (6279) MQTT_QUIC: Received fragment of 2 bytes
I (6279) MQTT_QUIC: Fragment hex (2 bytes): 0011
I (6289) MQTT_QUIC: Packet not complete yet, continuing to buffer
I (6289) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (6299) MQTT_QUIC: Function: mqtt_quic_transport_send
I (6299) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3c1456bc, bytesToSend=17
I (6309) MQTT_QUIC: Received fragment of 17 bytes
I (6319) MQTT_QUIC: Fragment hex (17 bytes): 65737033325f717569635f636c69656e74
I (6319) MQTT_QUIC: === SENDING COMPLETE MQTT PACKET ===
I (6329) MQTT_QUIC: Packet length: 31 bytes
I (6329) MQTT_QUIC: Is CONNECT packet: YES
I (6339) MQTT_QUIC: Complete MQTT packet hex (31 bytes): 101d00044d51545404020000001165737033325f717569635f636c69656e74
I00001600 0x con recv packet len=27
I00001600 0x pkt rx pkn=8 dcid=0x type=1RTT k=0
I00001600 0x frm rx 8 1RTT ACK(0x02) largest_ack=2 ack_delay=25(98) ack_range_count=0
I00001600 0x frm rx 8 1RTT ACK(0x02) range=[2..0] len=2
I (6359) QUIC: Opened new QUIC stream with ID: 0
I00001799 0x pkt tx pkn=4 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00001799 0x frm tx 4 1RTT STREAM(0x0a) id=0x0 fin=0 offset=0 len=31 uni=0
I00001799 0x ldc loss_detection_timer=7367676359 timeout=1439
I00001799 0x ldc latest_rtt=68 min_rtt=68 smoothed_rtt=385 rttvar=256 ack_delay=25
I00001799 0x cca 1444 bytes acked, slow start cwnd=17110
I00001799 0x ldc loss_detection_timer=7367676359 timeout=1639
I00001799 0x pkt read packet 27 left 0
I00001839 0x con recv packet len=1252
I00001839 0x pkt rx pkn=9 dcid=0x type=1RTT k=0
I00001839 0x frm rx 9 1RTT PING(0x01)
I00001839 0x frm rx 9 1RTT PADDING(0x00) len=1230
I00001839 0x pkt read packet 1252 left 0
I00001858 0x con recv packet len=34
I00001858 0x pkt rx pkn=10 dcid=0x type=1RTT k=0
I00001858 0x frm rx 10 1RTT ACK(0x02) largest_ack=4 ack_delay=1(7) ack_range_count=0
I00001858 0x frm rx 10 1RTT ACK(0x02) range=[4..2] len=2
I00001858 0x ldc latest_rtt=58 min_rtt=58 smoothed_rtt=345 rttvar=274 ack_delay=1
I00001858 0x ldc loss detection timer canceled
I00001858 0x frm rx 10 1RTT STREAM(0x0a) id=0x0 fin=0 offset=0 len=4 uni=0
I00001858 0x pkt read packet 34 left 0
I (6389) QUIC: Sent QUIC packet with 61 bytes, stream data: 31 bytes
I (6479) QUIC: Successfully wrote 31 bytes to QUIC stream
I (6479) MQTT_QUIC: Successfully sent complete MQTT packet (31 bytes) over QUIC
I (6489) MQTT_QUIC: Successfully sent complete MQTT packet
I (6489) MQTT_QUIC: === RECEIVED 1 BYTES FROM QUIC ===
I (6499) MQTT_QUIC: Received packet hex (1 bytes): 20
I (6499) MQTT_QUIC: *** MQTT Packet Type: CONNACK (0x02) ***
I (6509) MQTT_QUIC: === RECEIVED 1 BYTES FROM QUIC ===
I (6509) MQTT_QUIC: Received packet hex (1 bytes): 02
I (6519) MQTT_QUIC: *** MQTT Packet Type: UNKNOWN (0x00) ***
I (6519) MQTT_QUIC: === RECEIVED 2 BYTES FROM QUIC ===
I (6529) MQTT_QUIC: Received packet hex (2 bytes): 0000
I (6529) MQTT_QUIC: *** MQTT Packet Type: UNKNOWN (0x00) ***
I (6539) quic_demo_main: MQTT_Connect returned: 0, sessionPresent: false
I (6549) quic_demo_main: Connected to MQTT broker over QUIC!
I (6549) quic_demo_main: Waiting for CONNACK processing...
I00001904 0x pkt tx pkn=5 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00001904 0x frm tx 5 1RTT ACK(0x02) largest_ack=10 ack_delay=45(5721) ack_range_count=0
I00001904 0x frm tx 5 1RTT ACK(0x02) range=[10..8] len=2
E (6579) task_wdt: esp_task_wdt_reset(707): task not found
E (6579) task_wdt: esp_task_wdt_reset(707): task not found
E (6589) task_wdt: esp_task_wdt_reset(707): task not found
E (6589) task_wdt: esp_task_wdt_reset(707): task not found
I00002026 0x con recv packet len=1332
I00002026 0x pkt rx pkn=11 dcid=0x type=1RTT k=0
I00002026 0x frm rx 11 1RTT PING(0x01)
I00002026 0x frm rx 11 1RTT PADDING(0x00) len=1310
I00002026 0x pkt read packet 1332 left 0
E (6619) task_wdt: esp_task_wdt_reset(707): task not found
E (6619) task_wdt: esp_task_wdt_reset(707): task not found
I00002054 0x pkt tx pkn=6 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00002054 0x frm tx 6 1RTT ACK(0x02) largest_ack=11 ack_delay=27(3493) ack_range_count=0
I00002054 0x frm tx 6 1RTT ACK(0x02) range=[11..8] len=3
E (6649) task_wdt: esp_task_wdt_reset(707): task not found
E (6649) task_wdt: esp_task_wdt_reset(707): task not found
E (6649) task_wdt: esp_task_wdt_reset(707): task not found
E (6659) task_wdt: esp_task_wdt_reset(707): task not found
E (6669) task_wdt: esp_task_wdt_reset(707): task not found
E (6669) task_wdt: esp_task_wdt_reset(707): task not found
E (6679) task_wdt: esp_task_wdt_reset(707): task not found
E (6679) task_wdt: esp_task_wdt_reset(707): task not found
E (6689) task_wdt: esp_task_wdt_reset(707): task not found
E (6689) task_wdt: esp_task_wdt_reset(707): task not found
E (6699) task_wdt: esp_task_wdt_reset(707): task not found
E (6699) task_wdt: esp_task_wdt_reset(707): task not found
E (6709) task_wdt: esp_task_wdt_reset(707): task not found
E (6709) task_wdt: esp_task_wdt_reset(707): task not found
E (6719) task_wdt: esp_task_wdt_reset(707): task not found
E (6719) task_wdt: esp_task_wdt_reset(707): task not found
E (6729) task_wdt: esp_task_wdt_reset(707): task not found
E (6729) task_wdt: esp_task_wdt_reset(707): task not found
E (6739) task_wdt: esp_task_wdt_reset(707): task not found
E (6739) task_wdt: esp_task_wdt_reset(707): task not found
E (6749) task_wdt: esp_task_wdt_reset(707): task not found
E (6749) task_wdt: esp_task_wdt_reset(707): task not found
E (6759) task_wdt: esp_task_wdt_reset(707): task not found
E (6759) task_wdt: esp_task_wdt_reset(707): task not found
E (6769) task_wdt: esp_task_wdt_reset(707): task not found
E (6769) task_wdt: esp_task_wdt_reset(707): task not found
E (6779) task_wdt: esp_task_wdt_reset(707): task not found
E (6779) task_wdt: esp_task_wdt_reset(707): task not found
E (6789) task_wdt: esp_task_wdt_reset(707): task not found
E (6799) task_wdt: esp_task_wdt_reset(707): task not found
E (6799) task_wdt: esp_task_wdt_reset(707): task not found
E (6989) task_wdt: esp_task_wdt_reset(707): task not found
I00002419 0x con recv packet len=1412
I00002419 0x pkt rx pkn=12 dcid=0x type=1RTT k=0
I00002419 0x frm rx 12 1RTT PING(0x01)
I00002419 0x frm rx 12 1RTT PADDING(0x00) len=1390
I00002419 0x pkt read packet 1412 left 0
E (6999) task_wdt: esp_task_wdt_reset(707): task not found
E (7009) task_wdt: esp_task_wdt_reset(707): task not found
E (7009) task_wdt: esp_task_wdt_reset(707): task not found
I00002448 0x pkt tx pkn=7 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00002448 0x frm tx 7 1RTT ACK(0x02) largest_ack=12 ack_delay=28(3597) ack_range_count=0
I00002448 0x frm tx 7 1RTT ACK(0x02) range=[12..8] len=4
I00002448 0x frm tx 7 1RTT PING(0x01)
I00002448 0x ldc loss_detection_timer=8045986110 timeout=1469
E (7049) task_wdt: esp_task_wdt_reset(707): task not found
E (7049) task_wdt: esp_task_wdt_reset(707): task not found
E (7059) task_wdt: esp_task_wdt_reset(707): task not found
I00002491 0x con recv packet len=1472
I00002491 0x pkt rx pkn=13 dcid=0x type=1RTT k=0
I00002491 0x frm rx 13 1RTT PING(0x01)
I00002491 0x frm rx 13 1RTT PADDING(0x00) len=1450
I00002491 0x pkt read packet 1472 left 0
I00002511 0x con recv packet len=27
I00002511 0x pkt rx pkn=14 dcid=0x type=1RTT k=0
I00002511 0x frm rx 14 1RTT ACK(0x02) largest_ack=7 ack_delay=25(98) ack_range_count=0

I00002511 0x ldc latest_rtt=62 min_rtt=58 smoothed_rtt=309 rttvar=276 ack_delay=25
I00002511 0x ldc loss detection timer canceled
I00002511 0x pkt read packet 27 left 0
I00002546 0x pkt tx pkn=8 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00002546 0x frm tx 8 1RTT ACK(0x02) largest_ack=14 ack_delay=35(4385) ack_range_count=0
I00002546 0x frm tx 8 1RTT ACK(0x02) range=[14..13] len=1
E (7139) task_wdt: esp_task_wdt_reset(707): task not found
E (7139) task_wdt: esp_task_wdt_reset(707): task not found
E (7149) task_wdt: esp_task_wdt_reset(707): task not found
E (7149) task_wdt: esp_task_wdt_reset(707): task not found
E (7159) task_wdt: esp_task_wdt_reset(707): task not found
E (7169) task_wdt: esp_task_wdt_reset(707): task not found
E (7169) task_wdt: esp_task_wdt_reset(707): task not found
E (7169) task_wdt: esp_task_wdt_reset(707): task not found
E (7179) task_wdt: esp_task_wdt_reset(707): task not found
E (7179) task_wdt: esp_task_wdt_reset(707): task not found
I (7559) quic_demo_main: Processing pending MQTT messages...
I (8059) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (8059) MQTT_QUIC: Function: mqtt_quic_transport_send
I (8059) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3fcb949c, bytesToSend=4
I (8059) MQTT_QUIC: Received fragment of 4 bytes
I (8069) MQTT_QUIC: Fragment hex (4 bytes): 82140002
I (8069) MQTT_QUIC: Starting new MQTT packet
I (8069) MQTT_QUIC: MQTT packet type: 0x82
I (8079) MQTT_QUIC: Determined MQTT packet length: 22 bytes (remaining_length=20, bytes_used=1)
I (8089) MQTT_QUIC: Packet not complete yet, continuing to buffer
I (8089) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (8099) MQTT_QUIC: Function: mqtt_quic_transport_send
I (8099) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3fcb94a8, bytesToSend=2
I (8109) MQTT_QUIC: Received fragment of 2 bytes
I (8109) MQTT_QUIC: Fragment hex (2 bytes): 000f
I (8119) MQTT_QUIC: Packet not complete yet, continuing to buffer
I (8129) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (8129) MQTT_QUIC: Function: mqtt_quic_transport_send
I (8129) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3c145918, bytesToSend=15
I (8139) MQTT_QUIC: Received fragment of 15 bytes
I (8149) MQTT_QUIC: Fragment hex (15 bytes): 65737033322f717569632f74657374
I (8149) MQTT_QUIC: Packet not complete yet, continuing to buffer
I (8159) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (8159) MQTT_QUIC: Function: mqtt_quic_transport_send
I (8169) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3fcb9554, bytesToSend=1
I (8179) MQTT_QUIC: Received fragment of 1 bytes
I (8179) MQTT_QUIC: Fragment hex (1 bytes): 00
I (8189) MQTT_QUIC: === SENDING COMPLETE MQTT PACKET ===
I (8189) MQTT_QUIC: Packet length: 22 bytes
I (8189) MQTT_QUIC: Is CONNECT packet: NO
I (8199) MQTT_QUIC: Complete MQTT packet hex (22 bytes): 82140002000f65737033322f717569632f7465737400
I00003644 0x pkt tx pkn=9 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00003644 0x frm tx 9 1RTT STREAM(0x0e) id=0x0 fin=0 offset=31 len=22 uni=0
I00003644 0x ldc loss_detection_timer=9214585472 timeout=1441
I (8229) QUIC: Sent QUIC packet with 53 bytes, stream data: 22 bytes
I (8229) QUIC: Successfully wrote 22 bytes to QUIC stream
I (8239) MQTT_QUIC: Successfully sent complete MQTT packet (22 bytes) over QUIC
E (8249) task_wdt: esp_task_wdt_reset(707): task not found
I (8239) MQTT_QUIC: Successfully sent complete MQTT packet
I (8249) quic_demo_main: Subscribed to topic esp32/quic/test
I (8259) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (8259) MQTT_QUIC: Function: mqtt_quic_transport_send
I (8269) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3fcb94f8, bytesToSend=4
I (8279) MQTT_QUIC: Received fragment of 4 bytes
I (8279) MQTT_QUIC: Fragment hex (4 bytes): 3031000f
I (8289) MQTT_QUIC: Starting new MQTT packet
I (8289) MQTT_QUIC: MQTT packet type: 0x30
I (8289) MQTT_QUIC: Determined MQTT packet length: 51 bytes (remaining_length=49, bytes_used=1)
I (8299) MQTT_QUIC: Packet not complete yet, continuing to buffer
I (8309) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (8309) MQTT_QUIC: Function: mqtt_quic_transport_send
I (8319) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3c145918, bytesToSend=15
I (8329) MQTT_QUIC: Received fragment of 15 bytes
I (8329) MQTT_QUIC: Fragment hex (15 bytes): 65737033322f717569632f74657374
I (8339) MQTT_QUIC: Packet not complete yet, continuing to buffer
I (8339) MQTT_QUIC: === TRANSPORT SEND CALLED ===
I (8349) MQTT_QUIC: Function: mqtt_quic_transport_send
I (8349) MQTT_QUIC: Parameters: pNetworkContext=0x3fcb9790, pBuffer=0x3c145990, bytesToSend=32
I (8359) MQTT_QUIC: Received fragment of 32 bytes
I (8359) MQTT_QUIC: Fragment hex (32 bytes): 48656c6c6f2066726f6d204553503332206f766572204d5154542b5155494321
I (8369) MQTT_QUIC: === SENDING COMPLETE MQTT PACKET ===
I (8379) MQTT_QUIC: Packet length: 51 bytes
I (8379) MQTT_QUIC: Is CONNECT packet: NO
I (8389) MQTT_QUIC: Complete MQTT packet hex (51 bytes): 3031000f65737033322f717569632f7465737448656c6c6f2066726f6d204553503332206f766572204d5154542b5155494321
I00003679 0x con recv packet len=36
I00003679 0x pkt rx pkn=15 dcid=0x type=1RTT k=0
I00003834 0x frm rx 15 1RTT ACK(0x02) largest_ack=9 ack_delay=1(5) ack_range_count=0
I00003834 0x frm rx 15 1RTT ACK(0x02) range=[9..8] len=1
I00003834 0x ldc latest_rtt=34 min_rtt=34 smoothed_rtt=275 rttvar=276 ack_delay=1
I00003834 0x ldc loss detection timer canceled
I00003834 0x frm rx 15 1RTT STREAM(0x0e) id=0x0 fin=0 offset=4 len=5 uni=0
I00003834 0x pkt read packet 36 left 0
I00003834 0x pkt tx pkn=10 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00003874 0x frm tx 10 1RTT STREAM(0x0e) id=0x0 fin=0 offset=53 len=51 uni=0
I00003874 0x pkt tx pkn=10 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00003874 0x frm tx 10 1RTT ACK(0x02) largest_ack=15 ack_delay=194(24343) ack_range_count=0
I00003874 0x frm tx 10 1RTT ACK(0x02) range=[15..15] len=0
I00003874 0x ldc loss_detection_timer=9368192070 timeout=1406
E (8489) task_wdt: esp_task_wdt_reset(707): task not found
I (8479) QUIC: Sent QUIC packet with 35 bytes, stream data: 51 bytes
E (8489) task_wdt: esp_task_wdt_reset(707): task not found
I (8489) QUIC: Successfully wrote 51 bytes to QUIC stream
I (8499) MQTT_QUIC: Successfully sent complete MQTT packet (51 bytes) over QUIC
E (8509) task_wdt: esp_task_wdt_reset(707): task not found
I (8509) MQTT_QUIC: Successfully sent complete MQTT packet
I (8519) quic_demo_main: Published message to esp32/quic/test
E (8529) task_wdt: esp_task_wdt_reset(707): task not found
I (8519) quic_demo_main: Entering main processing loop...
E (8529) task_wdt: esp_task_wdt_reset(707): task not found
I (8629) MQTT_QUIC: === RECEIVED 5 BYTES FROM QUIC ===
I (8629) MQTT_QUIC: Received packet hex (5 bytes): 9003000200
I (8629) MQTT_QUIC: *** MQTT Packet Type: SUBACK (0x09) ***
I (8629) quic_demo_main: MQTT Event: Packet Type=144
I (8629) quic_demo_main: === MQTT SUBACK RECEIVED ===
I (8639) quic_demo_main: SUBACK - Packet ID: 2
I (8639) quic_demo_main: SUBACK - Status codes available in raw data
I (8649) quic_demo_main: Packet Details - Remaining Length: 3, Type: 0x90
I (9489) example_connect: Got IPv6 event: Interface "example_netif_sta" address: fd79:7e9e:2f87:fe4f:f2f5:bdff:fede:e61c, type: ESP_IP6_ADDR_IS_UNIQUE_LOCAL
I00005240 0x ldc loss detection timer fired
I00005240 0x ldc pto_count=1
I00005240 0x ldc loss_detection_timer=10774333140 timeout=1405
I00005242 0x con transmit probe pkt left=2
I00005242 0x pkt tx pkn=12 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00005242 0x frm tx 12 1RTT STREAM(0x0e) id=0x0 fin=0 offset=53 len=51 uni=0
I00005242 0x ldc loss_detection_timer=12182283140 timeout=2812
I00005242 0x con probe pkt size=82
I00005242 0x con transmit probe pkt left=1
I00005242 0x pkt tx pkn=13 dcid=0xa175f23f5e5d6857bb type=1RTT k=0
I00005242 0x frm tx 13 1RTT STREAM(0x0e) id=0x0 fin=0 offset=53 len=51 uni=0
I00005291 0x con recv packet len=39
I00005291 0x pkt rx pkn=16 dcid=0x type=1RTT k=0
I00005291 0x ldc loss_detection_timer=12182283140 timeout=2812
I00005291 0x con probe pkt size=82
```

</details>

### EMQX

<details>

<summary> EMQX debug log </summary>

```
025-07-08T07:37:03.345549+00:00 [debug] version: 16777216, local_addr: 10.0.19.164:14567, remote_addr: y.y.y.y:62082, server_name: <<"x.x.x.x">>, conn: #Ref<0.2617253814.2250375168.147187>, crypto_buffer: <<1,0,1,61,3,3,224,80,159,51,143,207,149,208,200,42,111,27,53,113,79,61,207,25,69,239,184,151,28,22,211,246,179,201,159,129,4,68,0,0,32,19,2,19,1,192,44,192,43,192,48,192,47,192,39,192,35,192,40,192,36,192,10,192,9,192,8,192,20,192,19,192,18,1,0,0,244,255,165,0,27,15,0,5,4,128,2,0,0,4,4,128,16,0,0,9,1,3,17,...>>, alpns: <<"mqtt">>, client_alpns: <<4,109,113,116,116>>
2025-07-08T07:37:04.234102+00:00 [debug] is_resumed: false, alpns: <<"mqtt">>
2025-07-08T07:37:04.677238+00:00 [debug] tag: MQTT, msg: raw_bin_received, peername: y.y.y.y:62082, size: 31, type: hex, bin: 101D00044D51545404020000001165737033325F717569635F636C69656E74
2025-07-08T07:37:04.677513+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_received, peername: y.y.y.y:62082, packet: CONNECT(Q0, R0, D0, ClientId=esp32_quic_client, ProtoName=MQTT, ProtoVsn=4, CleanStart=true, KeepAlive=0, Username=undefined, Password=)
2025-07-08T07:37:04.678395+00:00 [debug] clientid: esp32_quic_client, msg: insert_channel_info, peername: y.y.y.y:62082
2025-07-08T07:37:04.678520+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_sent, peername: y.y.y.y:62082, packet: CONNACK(Q0, R0, D0, AckFlags=0, ReasonCode=0)
2025-07-08T07:37:07.678355+00:00 [debug] tag: SOCKET, clientid: esp32_quic_client, msg: emqx_connection_terminated, peername: y.y.y.y:65354, reason: {shutdown,discarded}
2025-07-08T07:37:07.678558+00:00 [info] clientid: esp32_quic_client, msg: terminate, peername: y.y.y.y:65354, reason: {shutdown,discarded}
2025-07-08T07:37:07.678971+00:00 [debug] msg: emqx_cm_clean_down, client_id: <<"esp32_quic_client">>
2025-07-08T07:37:09.679469+00:00 [debug] clientid: esp32_quic_client, msg: cancel_stats_timer, peername: y.y.y.y:62082
2025-07-08T07:37:10.028597+00:00 [debug] version: 16777216, local_addr: 10.0.19.164:14567, remote_addr: y.y.y.y:49436, server_name: <<"x.x.x.x">>, conn: #Ref<0.2617253814.2250375168.147191>, crypto_buffer: <<1,0,1,61,3,3,140,211,117,119,222,158,229,15,172,2,242,67,1,250,151,185,147,172,116,184,21,216,219,212,78,153,224,103,244,186,105,227,0,0,32,19,2,19,1,192,44,192,43,192,48,192,47,192,39,192,35,192,40,192,36,192,10,192,9,192,8,192,20,192,19,192,18,1,0,0,244,255,165,0,27,15,0,5,4,128,2,0,0,4,4,128,16,0,0,9,1,3,17,...>>, alpns: <<"mqtt">>, client_alpns: <<4,109,113,116,116>>
2025-07-08T07:37:10.919950+00:00 [debug] is_resumed: false, alpns: <<"mqtt">>
2025-07-08T07:37:11.378389+00:00 [debug] tag: MQTT, msg: raw_bin_received, peername: y.y.y.y:49436, size: 31, type: hex, bin: 101D00044D51545404020000001165737033325F717569635F636C69656E74
2025-07-08T07:37:11.378660+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_received, peername: y.y.y.y:49436, packet: CONNECT(Q0, R0, D0, ClientId=esp32_quic_client, ProtoName=MQTT, ProtoVsn=4, CleanStart=true, KeepAlive=0, Username=undefined, Password=)
2025-07-08T07:37:11.379494+00:00 [debug] clientid: esp32_quic_client, msg: insert_channel_info, peername: y.y.y.y:49436
2025-07-08T07:37:11.379625+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_sent, peername: y.y.y.y:49436, packet: CONNACK(Q0, R0, D0, AckFlags=0, ReasonCode=0)
2025-07-08T07:37:13.214616+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: raw_bin_received, peername: y.y.y.y:49436, size: 22, type: hex, bin: 82140002000F65737033322F717569632F7465737400
2025-07-08T07:37:13.214842+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_received, peername: y.y.y.y:49436, packet: SUBSCRIBE(Q1, R0, D0, PacketId=2 TopicFilters=[esp32/quic/test(#{nl => 0,qos => 0,rap => 0,rh => 0})])
2025-07-08T07:37:13.215050+00:00 [debug] tag: AUTHZ, clientid: esp32_quic_client, msg: authorization_module_ignore, peername: y.y.y.y:49436, topic: esp32/quic/test, module: emqx_authz_client_info, action: SUBSCRIBE(Q0), authorize_type: client_info
2025-07-08T07:37:13.215188+00:00 [debug] tag: AUTHZ, clientid: esp32_quic_client, msg: authorization_matched_allow, peername: y.y.y.y:49436, topic: esp32/quic/test, module: emqx_authz_file, action: SUBSCRIBE(Q0), authorize_type: file
2025-07-08T07:37:13.215327+00:00 [info] tag: AUTHZ, clientid: esp32_quic_client, msg: authorization_permission_allowed, peername: y.y.y.y:49436, topic: esp32/quic/test, source: file, action: SUBSCRIBE(Q0)
2025-07-08T07:37:13.215443+00:00 [debug] tag: SUBSCRIBE, clientid: esp32_quic_client, msg: subscribe, peername: y.y.y.y:49436, topic: esp32/quic/test, sub_id: <<"esp32_quic_client">>, sub_opts: #{nl => 0,qos => 0,sub_props => #{},rap => 0,rh => 0}
2025-07-08T07:37:13.215707+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_sent, peername: y.y.y.y:49436, packet: SUBACK(Q0, R0, D0, PacketId=2, ReasonCodes=[0])
2025-07-08T07:37:13.215975+00:00 [debug] clientid: esp32_quic_client, msg: insert_channel_info, peername: y.y.y.y:49436
2025-07-08T07:37:13.928386+00:00 [debug] is_peer_acked: false, is_app_closing: false, is_handshake_completed: true
2025-07-08T07:37:14.379402+00:00 [debug] tag: SOCKET, clientid: esp32_quic_client, msg: emqx_connection_terminated, peername: y.y.y.y:62082, reason: {shutdown,discarded}
2025-07-08T07:37:14.379627+00:00 [info] clientid: esp32_quic_client, msg: terminate, peername: y.y.y.y:62082, reason: {shutdown,discarded}
2025-07-08T07:37:14.379901+00:00 [debug] msg: emqx_cm_clean_down, client_id: <<"esp32_quic_client">>
2025-07-08T07:37:14.827365+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: raw_bin_received, peername: y.y.y.y:49436, size: 51, type: hex, bin: 3031000F65737033322F717569632F7465737448656C6C6F2066726F6D204553503332206F766572204D5154542B5155494321
2025-07-08T07:37:14.827579+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_received, peername: y.y.y.y:49436, packet: PUBLISH(Q0, R0, D0, Topic=esp32/quic/test, PacketId=undefined, Payload(text)=Hello from ESP32 over MQTT+QUIC!)
2025-07-08T07:37:14.827751+00:00 [debug] tag: AUTHZ, clientid: esp32_quic_client, msg: authorization_module_ignore, peername: y.y.y.y:49436, topic: esp32/quic/test, module: emqx_authz_client_info, action: PUBLISH(Q0,R0), authorize_type: client_info
2025-07-08T07:37:14.827880+00:00 [debug] tag: AUTHZ, clientid: esp32_quic_client, msg: authorization_matched_allow, peername: y.y.y.y:49436, topic: esp32/quic/test, module: emqx_authz_file, action: PUBLISH(Q0,R0), authorize_type: file
2025-07-08T07:37:14.827990+00:00 [info] tag: AUTHZ, clientid: esp32_quic_client, msg: authorization_permission_allowed, peername: y.y.y.y:49436, topic: esp32/quic/test, source: file, action: PUBLISH(Q0,R0)
2025-07-08T07:37:14.828094+00:00 [debug] tag: PUBLISH, clientid: esp32_quic_client, msg: publish_to, peername: y.y.y.y:49436, topic: esp32/quic/test, payload: Hello from ESP32 over MQTT+QUIC!, payload_encode: text
2025-07-08T07:37:14.828371+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_sent, peername: y.y.y.y:49436, packet: PUBLISH(Q0, R0, D0, Topic=esp32/quic/test, PacketId=undefined, Payload(text)=Hello from ESP32 over MQTT+QUIC!)
2025-07-08T07:37:19.829514+00:00 [debug] clientid: esp32_quic_client, msg: cancel_stats_timer, peername: y.y.y.y:49436
2025-07-08T07:37:20.036479+00:00 [debug] version: 16777216, local_addr: 10.0.19.164:14567, remote_addr: y.y.y.y:63786, server_name: <<"x.x.x.x">>, conn: #Ref<0.2617253814.2250375168.147195>, crypto_buffer: <<1,0,1,61,3,3,84,197,141,121,241,61,158,79,6,242,23,224,211,63,119,125,227,98,24,152,5,95,98,26,84,201,252,158,104,235,110,20,0,0,32,19,2,19,1,192,44,192,43,192,48,192,47,192,39,192,35,192,40,192,36,192,10,192,9,192,8,192,20,192,19,192,18,1,0,0,244,255,165,0,27,15,0,5,4,128,2,0,0,4,4,128,16,0,0,9,1,3,17,...>>, alpns: <<"mqtt">>, client_alpns: <<4,109,113,116,116>>
2025-07-08T07:37:20.660726+00:00 [debug] is_peer_acked: false, is_app_closing: false, is_handshake_completed: true
2025-07-08T07:37:20.924554+00:00 [debug] is_resumed: false, alpns: <<"mqtt">>
2025-07-08T07:37:21.380226+00:00 [debug] tag: MQTT, msg: raw_bin_received, peername: y.y.y.y:63786, size: 31, type: hex, bin: 101D00044D51545404020000001165737033325F717569635F636C69656E74
2025-07-08T07:37:21.380489+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_received, peername: y.y.y.y:63786, packet: CONNECT(Q0, R0, D0, ClientId=esp32_quic_client, ProtoName=MQTT, ProtoVsn=4, CleanStart=true, KeepAlive=0, Username=undefined, Password=)
2025-07-08T07:37:21.381338+00:00 [debug] clientid: esp32_quic_client, msg: insert_channel_info, peername: y.y.y.y:63786
2025-07-08T07:37:21.381463+00:00 [debug] tag: MQTT, clientid: esp32_quic_client, msg: mqtt_packet_sent, peername: y.y.y.y:63786, packet: CONNACK(Q0, R0, D0, AckFlags=0, ReasonCode=0)
2025-07-08T07:37:24.381386+00:00 [debug] tag: SOCKET, clientid: esp32_quic_client, msg: emqx_connection_terminated, peername: y.y.y.y:49436, reason: {shutdown,discarded}
2025-07-08T07:37:24.381667+00:00 [info] clientid: esp32_quic_client, msg: terminate, peername: y.y.y.y:49436, reason: {shutdown,discarded}
2025-07-08T07:37:24.382106+00:00 [debug] msg: emqx_cm_clean_down, client_id: <<"esp32_quic_client">>
2025-07-08T07:37:26.382605+00:00 [debug] clientid: esp32_quic_client, msg: cancel_stats_timer, peername: y.y.y.y:63786
2025-07-08T07:37:29.216690+00:00 [debug] is_peer_acked: true, is_app_closing: false, is_handshake_completed: true
2025-07-08T07:37:37.382238+00:00 [debug] error: 1, status: connection_timeout
2025-07-08T07:37:37.382412+00:00 [debug] is_peer_acked: true, is_app_closing: false, is_handshake_completed: true
2025-07-08T07:37:37.382402+00:00 [debug] tag: SOCKET, clientid: esp32_quic_client, msg: emqx_connection_terminated, peername: y.y.y.y:63786, reason: {shutdown,connection_timeout}
2025-07-08T07:37:37.382581+00:00 [info] clientid: esp32_quic_client, msg: terminate, peername: y.y.y.y:63786, reason: {shutdown,connection_timeout}
2025-07-08T07:37:37.382843+00:00 [debug] msg: emqx_cm_clean_down, client_id: <<"esp32_quic_client">>
```
</details>


### KNOWN ISSUES


