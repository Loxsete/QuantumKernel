#include "drivers/net/ethernet.h"
#include "drivers/net/arp.h"
#include "drivers/net/ip.h"
#include "lib/libc.h"
#include "drivers/rtl8139.h"

static uint8_t local_mac[6];

void eth_init(uint8_t *mac) {
    memcpy(local_mac, mac, 6);
}

uint8_t *eth_mac(void) {
    return local_mac;
}

void eth_tx(uint8_t *dst, uint16_t type, uint8_t *payload, uint16_t len) {
    uint8_t frame[1514];
    eth_hdr_t *hdr = (eth_hdr_t*)frame;

    memcpy(hdr->dst, dst, 6);
    memcpy(hdr->src, local_mac, 6);
    hdr->type = __builtin_bswap16(type);

    memcpy(frame + sizeof(eth_hdr_t), payload, len);
    rtl8139_send(frame, len + sizeof(eth_hdr_t));
}

void eth_rx(uint8_t *data, uint16_t len) {
    eth_hdr_t *hdr = (eth_hdr_t*)data;
    uint16_t type = __builtin_bswap16(hdr->type);

    if (type == ETH_TYPE_ARP)
        arp_rx(data + sizeof(eth_hdr_t));
    else if (type == ETH_TYPE_IP)
        ip_rx(data + sizeof(eth_hdr_t), len - sizeof(eth_hdr_t));
}
