#include "drivers/net/arp.h"
#include "drivers/net/ethernet.h"
#include <string.h>

#include <string.h>

typedef struct {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];
    uint32_t spa;
    uint8_t tha[6];
    uint32_t tpa;
} __attribute__((packed)) arp_pkt_t;

static uint32_t local_ip;
static uint32_t cached_ip;
static uint8_t cached_mac[6];

void arp_init(uint32_t ip) {
    local_ip = ip;
}

int arp_lookup(uint32_t ip, uint8_t *mac) {
    if (ip == cached_ip) {
        memcpy(mac, cached_mac, 6);
        return 1;
    }
    return 0;
}

void arp_request(uint32_t ip) {
    uint8_t buf[sizeof(arp_pkt_t)];
    arp_pkt_t *a = (arp_pkt_t*)buf;

    a->htype = __builtin_bswap16(1);
    a->ptype = __builtin_bswap16(0x0800);
    a->hlen = 6;
    a->plen = 4;
    a->oper = __builtin_bswap16(1);
    memcpy(a->sha, eth_mac(), 6);
    a->spa = local_ip;
    memset(a->tha, 0, 6);
    a->tpa = ip;

    uint8_t bcast[6] = {255,255,255,255,255,255};
    eth_tx(bcast, ETH_TYPE_ARP, buf, sizeof(buf));
}

void arp_rx(uint8_t *pkt) {
    arp_pkt_t *a = (arp_pkt_t*)pkt;
    if (__builtin_bswap16(a->oper) == 2) {
        cached_ip = a->spa;
        memcpy(cached_mac, a->sha, 6);
    }
}
