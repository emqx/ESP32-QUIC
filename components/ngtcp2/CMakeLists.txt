# Wrapper CMakeLists.txt for ngtcp2 in ESP-IDF
     cmake_minimum_required(VERSION 3.16)

     # Define source files from ngtcp2/lib and ngtcp2/crypto/quictls
     file(GLOB LIB_SRCS "ngtcp2/lib/*.c")
     set(CRYPTO_SRCS
         "ngtcp2/crypto/shared.c"
         "ngtcp2/crypto/wolfssl/wolfssl.c"
     )
     set(SRCS ${LIB_SRCS} ${CRYPTO_SRCS})

     # Define and verify version.h.in path
     set(VERSION_H_IN "${CMAKE_CURRENT_LIST_DIR}/ngtcp2/lib/includes/ngtcp2/version.h.in")
     if(NOT EXISTS "${VERSION_H_IN}")
         message(FATAL_ERROR "version.h.in not found at ${VERSION_H_IN}")
     else()
         message(STATUS "Found version.h.in at ${VERSION_H_IN}")
     endif()

     # Generate version.h from version.h.in
     set(PACKAGE_VERSION "1.13.90") # From ngtcp2/CMakeLists.txt
     math(EXPR PACKAGE_VERSION_NUM "0x011390" OUTPUT_FORMAT HEXADECIMAL) # For v1.13.90
     configure_file(
         "${VERSION_H_IN}"
         "${CMAKE_CURRENT_BINARY_DIR}/ngtcp2/version.h"
         @ONLY
     )

     # Register as ESP-IDF component
     idf_component_register(SRCS "${SRCS}"
                           INCLUDE_DIRS
                               "ngtcp2/lib/includes"
                               "ngtcp2/lib/"
                               "ngtcp2/lib/includes/ngtcp2"
                               "ngtcp2/crypto/includes"
                               "${CMAKE_CURRENT_BINARY_DIR}/ngtcp2"
                               "ngtcp2/crypto"
                           PRIV_INCLUDE_DIRS
                               "${CMAKE_CURRENT_LIST_DIR}/../../managed_components/wolfssl__wolfssl/include"
                               "${CMAKE_CURRENT_LIST_DIR}/../../managed_components/wolfssl__wolfssl/wolfssl"
                               "${CMAKE_CURRENT_LIST_DIR}/../../managed_components/wolfssl__wolfssl/wolfssl/wolfcrypt"
                               "${CMAKE_CURRENT_LIST_DIR}/../../managed_components/wolfssl__wolfssl/wolfssl/wolfcrypt/port/Espressif"
                           REQUIRES wolfssl)

     # Add wolfSSL-specific compile definitions
     target_compile_definitions(${COMPONENT_LIB} PUBLIC
        -DWITH_WOLFSSL
        -DWOLFSSL_USER_SETTINGS
        -DWOLFSSL_QUIC
        -DOPENSSL_EXTRA
        -DOPENSSL_ALL
        -DHAVE_SESSION_TICKET
        -DWOLFSSL_TLS13
        -DWOLFSSL_EVP_INCLUDED
        -DWOLFSSL_KEY_GEN
        -DWOLFSSL_AES_DIRECT
        -DWOLFSSL_RAND
        -DHAVE_ALPN
        -DWOLFSSL_SNI
        -DHAVE_AES_ECB
        -DHAVE_ECC
        -DHAVE_AESGCM
        -DHAVE_AESCCM
        -DHAVE_HKDF
        -DHAVE_CHACHA
        -DHAVE_POLY1305
        -DWOLFSSL_HAVE_EVP_PKEY
        -DWOLFSSL_EVP_AES
        -DWOLFSSL_ADVANCED_EXTENSIONS
        -DOPENSSL_ALL
        -DHAVE_ALPN
     )

     # Add necessary platform-specific definitions
     target_compile_definitions(${COMPONENT_LIB} PUBLIC -DHAVE_CONFIG_H)

     # Suppress maybe-uninitialized warning for ngtcp2
     target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=maybe-uninitialized)

     # Provide a minimal config.h for ESP32-C3
     file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.h"
         "#define HAVE_ARPA_INET_H 1\n"
         "#define HAVE_NETINET_IN_H 1\n"
         "#define HAVE_UNISTD_H 1\n"
         "#define HAVE_ENDIAN_H 1\n"
         "#define HAVE_DECL_BE64TOH 1\n"
         "#define WORDS_BIGENDIAN 0\n" # ESP32-C3 is little-endian
         "#define HAVE_WOLFSSL 1\n"
     )

     target_include_directories(${COMPONENT_LIB} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

     # Debug: Print paths to verify
     message(STATUS "VERSION_H_IN: ${VERSION_H_IN}")
     message(STATUS "Output version.h: ${CMAKE_CURRENT_BINARY_DIR}/ngtcp2/version.h")
     message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
     message(STATUS "wolfSSL include: ${CMAKE_CURRENT_LIST_DIR}/../../managed_components/wolfssl__wolfssl/wolfssl")
     message(STATUS "wolfSSL wolfcrypt include: ${CMAKE_CURRENT_LIST_DIR}/../../managed_components/wolfssl__wolfssl/wolfssl/wolfcrypt")
