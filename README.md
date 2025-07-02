# MQTT over QUIC Demo for esp32C3

This is a modified esp32 IDF hello_world example to prove QUIC could run on  
RISC-V [ESP32C3](https://www.espressif.com/en/products/socs/esp32-c3). 

Using following components:

- [wolfSSL](https://components.espressif.com/components/wolfssl/wolfssl)

  TLS stack, from esp32 component registry.

- [ngtcp2](https://github.com/ngtcp2/ngtcp2)

  QUIC stack, git submodule and patched by this repo.
  
- protocol_examples_common

  WiFi example code.
  
  esp-idf common component.
  
- esp_timer
  
  for eventLoop, timer implementation.
  
  esp-idf common component.

## What it does?

1. Connects to WIFI
1. Establish QUIC connection 
1. Send MQTT.Connect packet then shutdown


## How to use example

1. Select the instructions depending on Espressif chip installed on your development board:

   - [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)

1. Clone this repo with `git clone --recursive https://github.com/qzhuyan/POC-ESP32-QUIC.git`

1. Patch the ngtcp2
   ```bash
   patch  -p 1 -d components/ngtcp2/ngtcp2 < ngtcp2.patch
   ```

1. Change the QUIC server (`REMOTE_HOST` and `REMOTE_PORT`) in [ngtcp2_sample](main/ngtcp2_sample.c)

1. Change the WIFI username/password

    ``` bash
   idf.py menuconfig
   ``` 

1. Build 
   ``` bash
   idf.py build
   ```
   if build failed due to undefined reference to `TlsSessionCacheGetAndRdLock', 
      comment out `#define NO_SESSION_CACHE` from [wolffSSL usersettings](managed_components/wolfssl__wolfssl/include/user_settings.h)
      and retry.

1. After success build:
  ```
  ...
  quic_demo.bin binary size 0x16d060 bytes. Smallest app partition is 0x1a9000 bytes. 0x3bfa0 bytes (14%) free.
  ```

1. Download to the board refer to [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)

1. Check Loggings: 
<details>

<summary> idf monitoring </summary>

```
Starting in 2 seconds...
Starting in 1 seconds...
Starting in 0 seconds...
init random number generator
init client ...
I00000000 0x con next skip pkn=205
I (15531) esp_ev_compat: Starting IO watcher for fd 54
client write ...
I00000002 0x pkt tx pkn=0 dcid=0x22355bfc62522839 scid=0x version=0x00000001 type=Initial len=0
I00000002 0x frm tx 0 Initial CRYPTO(0x06) offset=0 len=321
I00000002 0x frm tx 0 Initial PADDING(0x00) len=838
I00000002 0x ldc loss_detection_timer=16173295000 timeout=999
ev run ...
I00000435 0x con recv packet len=1220
I00000435 0x pkt rx pkn=0 dcid=0x scid=0xe9317922180d89c885 version=0x00000001 type=Initial len=152
I00000435 0x frm rx 0 Initial ACK(0x02) largest_ack=0 ack_delay=0(0) ack_range_count=0
I00000435 0x frm rx 0 Initial ACK(0x02) range=[0..0] len=0
I00000435 0x ldc latest_rtt=432 min_rtt=432 smoothed_rtt=432 rttvar=216 ack_delay=0
I00000435 0x con path is not ECN capable
I00000435 0x cca 1200 bytes acked, slow start cwnd=13200
I00000435 0x ldc loss_detection_timer=16902967000 timeout=1296
I00000435 0x frm rx 0 Initial CRYPTO(0x06) offset=0 len=123
I00000435 0x con the negotiated version is 0x00000001
I00000435 0x pkt read packet 171 left 1049
I00000435 0x pkt rx pkn=1 dcid=0x scid=0xe9317922180d89c885 version=0x00000001 type=Handshake len=1031
I00000435 0x frm rx 1 Handshake CRYPTO(0x06) offset=0 len=1006
I00000435 0x frm rx 1 Handshake PADDING(0x00) len=1
I00000435 0x pkt read packet 1049 left 0
I00000435 0x con processing buffered handshake packet
I00000939 0x con recv packet len=306
I00000939 0x pkt rx pkn=2 dcid=0x scid=0xe9317922180d89c885 version=0x00000001 type=Handshake len=263
I00000939 0x frm rx 2 Handshake CRYPTO(0x06) offset=1006 len=238
I00000939 0x cry remote transport_parameters stateless_reset_token=0x55588194816b8d599bdfd6640755a2a5
I00000939 0x cry remote transport_parameters original_destination_connection_id=0x22355bfc62522839
I00000939 0x cry remote transport_parameters initial_source_connection_id=0xe9317922180d89c885
I00000939 0x cry remote transport_parameters initial_max_stream_data_bidi_local=65536
I00000939 0x cry remote transport_parameters initial_max_stream_data_bidi_remote=65536
I00000939 0x cry remote transport_parameters initial_max_stream_data_uni=65536
I00000939 0x cry remote transport_parameters initial_max_data=16777216
I00000939 0x cry remote transport_parameters initial_max_streams_bidi=0
I00000939 0x cry remote transport_parameters initial_max_streams_uni=0
I00000939 0x cry remote transport_parameters max_idle_timeout=0
I00000939 0x cry remote transport_parameters max_udp_payload_size=1472
I00000939 0x cry remote transport_parameters ack_delay_exponent=8
I00000939 0x cry remote transport_parameters max_ack_delay=26
I00000939 0x cry remote transport_parameters active_connection_id_limit=4
I00000939 0x cry remote transport_parameters disable_active_migration=0
I00000939 0x cry remote transport_parameters max_datagram_frame_size=0
I00000939 0x cry remote transport_parameters grease_quic_bit=0
I00000939 0x pkt read packet 281 left 25
I00000939 0x con processing buffered handshake packet
I00000939 0x con processing buffered protected packet
I00000939 0x pkt rx pkn=3 dcid=0x type=1RTT k=0
I00000939 0x frm rx 3 1RTT MAX_STREAMS(0x12) max_streams=10
in CB: extend_max_local_streams_bidi : -1
send msg: =
I00000939 0x frm rx 3 1RTT MAX_STREAMS(0x13) max_streams=1
I00000939 0x pkt read packet 25 left 0
I00001146 0x pkt tx pkn=1 dcid=0xe9317922180d89c885 scid=0x version=0x00000001 type=Initial len=0
I00001146 0x frm tx 1 Initial ACK(0x02) largest_ack=0 ack_delay=0(0) ack_range_count=0
I00001146 0x frm tx 1 Initial ACK(0x02) range=[0..0] len=0
I00001146 0x pkt tx pkn=0 dcid=0xe9317922180d89c885 scid=0x version=0x00000001 type=Handshake len=0
I00001146 0x frm tx 0 Handshake ACK(0x02) largest_ack=2 ack_delay=0(0) ack_range_count=0
I00001146 0x frm tx 0 Handshake ACK(0x02) range=[2..1] len=1
I00001146 0x frm tx 0 Handshake CRYPTO(0x06) offset=0 len=52
I00001146 0x ldc loss_detection_timer=17614503000 timeout=1296
I00001146 0x con discarding Initial packet number space
I00001146 0x ldc loss_detection_timer=17614503000 timeout=1296
I00001146 0x pkt tx pkn=0 dcid=0xe9317922180d89c885 type=1RTT k=0
I00001146 0x frm tx 0 1RTT ACK(0x02) largest_ack=3 ack_delay=206(25854) ack_range_count=0
I00001146 0x frm tx 0 1RTT ACK(0x02) range=[3..3] len=0
I00001146 0x frm tx 0 1RTT STREAM(0x0b) id=0x0 fin=1 offset=0 len=63 uni=0
I00001146 0x frm tx 0 1RTT PADDING(0x00) len=959
I00001146 0x ldc loss_detection_timer=17614503000 timeout=1296
I00001146 0x con sending PMTUD probe packet len=1406
I00001146 0x pkt tx pkn=1 dcid=0xe9317922180d89c885 type=1RTT k=0
I00001146 0x frm tx 1 1RTT PING(0x01)
I00001146 0x frm tx 1 1RTT PADDING(0x00) len=1378
I00001146 0x ldc loss_detection_timer=17614503000 timeout=1296
I00001297 0x con recv packet len=1220
I00001297 0x pkt rx pkn=4 dcid=0x type=1RTT k=0
I00001297 0x frm rx 4 1RTT ACK(0x02) largest_ack=0 ack_delay=0(0) ack_range_count=0
I00001297 0x frm rx 4 1RTT ACK(0x02) range=[0..0] len=0
I00001297 0x ldc latest_rtt=150 min_rtt=150 smoothed_rtt=396 rttvar=232 ack_delay=0
I00001297 0x cca 1060 bytes acked, slow start cwnd=14260
I00001297 0x ldc loss_detection_timer=17644928375 timeout=1176
I00001297 0x frm rx 4 1RTT HANDSHAKE_DONE(0x1e)
I00001297 0x con discarding Handshake packet number space
I00001297 0x ldc loss_detection_timer=17670928375 timeout=1202
I00001297 0x ldc loss_detection_timer=17670928375 timeout=1202
I00001297 0x frm rx 4 1RTT NEW_CONNECTION_ID(0x18) seq=1 cid=0xe9312058b40617d5bf retire_prior_to=0 stateless_reset_token=0x57942e9233e4c9f6e74afd184da871e4
I00001297 0x frm rx 4 1RTT PADDING(0x00) len=1164
I00001297 0x pkt read packet 1220 left 0
I00001374 0x con recv packet len=1220
I00001374 0x pkt rx pkn=5 dcid=0x type=1RTT k=0
I00001374 0x frm rx 5 1RTT PING(0x01)
I00001374 0x frm rx 5 1RTT PADDING(0x00) len=1198
I00001374 0x pkt read packet 1220 left 0
I00001393 0x con recv packet len=314
I00001393 0x pkt rx pkn=6 dcid=0x type=1RTT k=0
I00001393 0x frm rx 6 1RTT CRYPTO(0x06) offset=0 len=289
I00001393 0x pkt read packet 314 left 0
I00001409 0x con recv packet len=49
I00001409 0x pkt rx pkn=7 dcid=0x type=1RTT k=0
I00001409 0x frm rx 7 1RTT STREAM(0x0b) id=0x0 fin=1 offset=0 len=24 uni=0
I00001409 0x pkt read packet 49 left 0
I00001427 0x con recv packet len=27
I00001427 0x pkt rx pkn=8 dcid=0x type=1RTT k=0
I00001427 0x frm rx 8 1RTT ACK(0x02) largest_ack=1 ack_delay=25(98) ack_range_count=0
I00001427 0x frm rx 8 1RTT ACK(0x02) range=[1..0] len=1
I00001427 0x ldc latest_rtt=280 min_rtt=150 smoothed_rtt=379 rttvar=209 ack_delay=25
I00001427 0x cca 1406 bytes acked, slow start cwnd=15666
I00001427 0x ldc loss detection timer canceled
I00001427 0x pkt read packet 27 left 0
I00001467 0x con sending PMTUD probe packet len=1444
I00001467 0x pkt tx pkn=2 dcid=0xe9317922180d89c885 type=1RTT k=0
I00001467 0x frm tx 2 1RTT PING(0x01)
I00001467 0x frm tx 2 1RTT PADDING(0x00) len=1416
I00001467 0x ldc loss_detection_timer=17882809452 timeout=1244
I00001467 0x pkt tx pkn=3 dcid=0xe9317922180d89c885 type=1RTT k=0
I00001467 0x frm tx 3 1RTT ACK(0x02) largest_ack=8 ack_delay=40(5002) ack_range_count=0
I00001467 0x frm tx 3 1RTT ACK(0x02) range=[8..4] len=4
I00001518 0x con recv packet len=28
I00001518 0x pkt rx pkn=9 dcid=0x type=1RTT k=0
I00001518 0x frm rx 9 1RTT ACK(0x02) largest_ack=3 ack_delay=0(0) ack_range_count=0
I00001518 0x frm rx 9 1RTT ACK(0x02) range=[3..2] len=1
I00001518 0x ldc loss detection timer canceled
I00001518 0x frm rx 9 1RTT MAX_STREAMS(0x12) max_streams=11
in CB: extend_max_local_streams_bidi : 0
I00001518 0x pkt read packet 28 left 0
I00001546 0x con recv packet len=1252
I00001546 0x pkt rx pkn=10 dcid=0x type=1RTT k=0
I00001546 0x frm rx 10 1RTT PING(0x01)
I00001546 0x frm rx 10 1RTT PADDING(0x00) len=1230
I00001546 0x pkt read packet 1252 left 0
I00001565 0x con recv packet len=24
I00001565 0x pkt rx pkn=11 dcid=0x type=1RTT k=0
I00001565 0x frm rx 11 1RTT CONNECTION_CLOSE(0x1d) error_code=(unknown)(0x0) frame_type=0 reason_len=0 reason=[]
ngtcp2_conn_read_pkt: ERR_DRAINING
```
</details>

<details>
<summary> EMQX debug log </summary>

``` bash

2025-07-02T07:12:37.402059+00:00 [debug] version: 16777216, local_addr: 10.0.19.164:14567, remote_addr: 192.168.17.168:50619, server_name: <<"10.0.19.164">>, conn: #Ref<0.2617253814.2250375168.142871>, crypto_buffer: <<1,0,1,61,3,3,23,238,71,55,105,165,33,227,94,131,143,243,30,203,102,15,89,143,132,160,78,112,33,154,246,233,190,23,28,13,37,129,0,0,32,19,2,19,1,192,44,192,43,192,48,192,47,192,39,192,35,192,40,192,36,192,10,192,9,192,8,192,20,192,19,192,18,1,0,0,244,255,165,0,27,15,0,5,4,128,2,0,0,4,4,128,16,0,0,9,1,3,17,...>>, alpns: <<"mqtt">>, client_alpns: <<4,109,113,116,116>>
2025-07-02T07:12:38.257067+00:00 [debug] is_resumed: false, alpns: <<"mqtt">>
2025-07-02T07:12:38.257403+00:00 [debug] tag: MQTT, msg: raw_bin_received, peername: 192.168.17.168:50619, size: 63, type: hex, bin: 103D00044D5154540502012C051100000000002B636F6E76696E63696E672D6A656C6C79666973685F45535033325F7075625F313438333032343633335F31
2025-07-02T07:12:38.257657+00:00 [debug] tag: MQTT, clientid: convincing-jellyfish_ESP32_pub_1483024633_1, msg: mqtt_packet_received, peername: 192.168.17.168:50619, packet: CONNECT(Q0, R0, D0, ClientId=convincing-jellyfish_ESP32_pub_1483024633_1, ProtoName=MQTT, ProtoVsn=5, CleanStart=true, KeepAlive=300, Username=undefined, Password=)
2025-07-02T07:12:38.258279+00:00 [debug] clientid: convincing-jellyfish_ESP32_pub_1483024633_1, msg: insert_channel_info, peername: 192.168.17.168:50619
2025-07-02T07:12:38.258387+00:00 [debug] tag: MQTT, clientid: convincing-jellyfish_ESP32_pub_1483024633_1, msg: mqtt_packet_sent, peername: 192.168.17.168:50619, packet: CONNACK(Q0, R0, D0, AckFlags=0, ReasonCode=0)
2025-07-02T07:12:38.493229+00:00 [debug] tag: SOCKET, clientid: convincing-jellyfish_ESP32_pub_1483024633_1, msg: emqx_connection_terminated, peername: 192.168.17.168:50619, reason: {shutdown,normal}
2025-07-02T07:12:38.493440+00:00 [info] clientid: convincing-jellyfish_ESP32_pub_1483024633_1, msg: terminate, peername: 192.168.17.168:50619, reason: {shutdown,normal}
2025-07-02T07:12:38.493768+00:00 [debug] msg: emqx_cm_clean_down, client_id: <<"convincing-jellyfish_ESP32_pub_1483024633_1">>
2025-07-02T07:12:44.536028+00:00 [debug] is_peer_acked: false, is_app_closing: false, is_handshake_completed: true
```

<details>

## Source

``` bash
main/
├── esp_ev_compat.c      libev compat code based on ESP eventloop
├── idf_component.yml    Defines component dependency
├── ngtcp2_sample.c      From ngtcp2 example code
└── quic_demo_main.c     main

components/ngtcp2        Wrap NGTCP2 as IDF component.
ngtcp2.patch             Patch for ngtpc2
partition.csv            Partition customization, default partition is too small.
sdkconfig                platform-specific config, wifi pass, partition file, tuned main stack size ...
```

## Non-standard tweaks

- TLS certificate verify is disabled.

- Large factory app Partition:

```
# Name,   Type, SubType, Offset,  Size, Flags
# Note: if you have increased the bootloader size, make sure to update the offsets to avoid overlap
nvs,      data, nvs,     ,        0x6000,
phy_init, data, phy,     ,        0x1000,
factory,  app,  factory, ,        1700K,
```
