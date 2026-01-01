#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>

void icmp_ping(uint32_t ip);
void icmp_rx(uint8_t *pkt, uint16_t len);
int icmp_get_status(void);
void icmp_reset_status(void);

#endif
