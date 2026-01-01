#include "drivers/net/ip.h"
#include "drivers/net/ethernet.h"
#include "drivers/net/arp.h"
#include "drivers/net/icmp.h" 
#include "lib/libc.h"

typedef struct {
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t frag;
    uint8_t ttl;
    uint8_t proto;
    uint16_t csum;
    uint32_t src;
    uint32_t dst;
} __attribute__((packed)) ip_hdr_t;

static uint32_t local_ip;

static uint16_t checksum(void *buf, int len) {
    uint32_t sum = 0;
    uint16_t *p = buf;
    while (len > 1) { sum += *p++; len -= 2; }
    if (len) sum += *(uint8_t*)p;
    while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}

void ip_init(uint32_t ip) {
    local_ip = ip;
}

void ip_tx(uint32_t dst, uint8_t proto, uint8_t *payload, uint16_t len) {
    uint8_t buf[1500];
    ip_hdr_t *h = (ip_hdr_t*)buf;

    h->ver_ihl = 0x45;
    h->tos = 0;
    h->len = __builtin_bswap16(sizeof(ip_hdr_t) + len);
    h->id = 0;
    h->frag = 0;
    h->ttl = 64;
    h->proto = proto;
    h->src = local_ip;
    h->dst = dst;
    h->csum = 0;
    h->csum = checksum(h, sizeof(ip_hdr_t));

    memcpy(buf + sizeof(ip_hdr_t), payload, len);

    uint8_t mac[6];
    if (!arp_lookup(dst, mac)) {
        arp_request(dst);
        return;
    }

    eth_tx(mac, ETH_TYPE_IP, buf, sizeof(ip_hdr_t) + len);
}

void ip_rx(uint8_t *pkt, uint16_t len) {
    ip_hdr_t *h = (ip_hdr_t*)pkt;
    
    if (len < sizeof(ip_hdr_t))
        return;
    
    if (h->proto == 1) {
        uint16_t payload_len = len - sizeof(ip_hdr_t);
        icmp_rx(pkt + sizeof(ip_hdr_t), payload_len);
    }
}
