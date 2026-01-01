#pragma once
#include <stdint.h>

#define ETH_TYPE_ARP  0x0806
#define ETH_TYPE_IP   0x0800

typedef struct {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
} __attribute__((packed)) eth_hdr_t;

void eth_init(uint8_t *mac);
void eth_rx(uint8_t *data, uint16_t len);
void eth_tx(uint8_t *dst, uint16_t type, uint8_t *payload, uint16_t len);
uint8_t *eth_mac(void);
