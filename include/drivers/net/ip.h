#pragma once
#include <stdint.h>

void ip_init(uint32_t ip);
void ip_rx(uint8_t *pkt, uint16_t len);
void ip_tx(uint32_t dst, uint8_t proto, uint8_t *payload, uint16_t len);
