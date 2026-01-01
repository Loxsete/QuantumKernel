#pragma once
#include <stdint.h>

void arp_init(uint32_t ip);
void arp_request(uint32_t ip);
void arp_rx(uint8_t *pkt);
int arp_lookup(uint32_t ip, uint8_t *mac);
