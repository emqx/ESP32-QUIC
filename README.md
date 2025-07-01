# MQTT over QUIC Demo for esp32C3

1. Connects to WIFI
1. Establish QUIC connection 
1. Send MQTT.Connect packet then shutdown

Stack:

- TLS implementation: 
  
        wolfSSL
      
- QUIC implementation: 

        ngtcp2

Other support components:

- protocol_examples_common

  Simple wifi connection
  
- esp_timer
  
  for eventLoop timer handler

## How to use example


1. Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)

1. Clone this repo with `git clone --recursive `

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
   if build failed remove 

1. Download to the chip

## Contents 

``` bash
main/
├── esp_ev_compat.c      libev compat code based on ESP eventloop
├── idf_component.yml    Defines component dependency
├── ngtcp2_sample.c      From ngtcp2 example code
└── quic_demo_main.c     main

components/              NGTCP2 as IDF component.
ngtcp2.patch             Patch for ngtpc2
partition.csv            Partition customization, default partition is too small.
sdkconfig                platform-specific config, wifi pass, partition file, tuned main stack size ...
```

