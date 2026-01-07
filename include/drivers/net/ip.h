#ifndef IP_H
#define IP_H

#include <stdint.h>

int ip_tx(uint32_t dst, uint8_t proto, uint8_t *payload, uint16_t len);
void ip_rx(uint8_t *pkt, uint16_t len);
void ip_init(uint32_t ip);
void ip_process_arp_queue(uint32_t resolved_ip);

#define ETH_TYPE_IP 0x0800

#endif
