#include "drivers/net/ip.h"
#include "drivers/net/ethernet.h"
#include "drivers/net/arp.h"
#include "drivers/net/icmp.h" 
#include "drivers/terminal.h"
#include "drivers/timer.h"
#include "lib/libc.h"

#define ARP_QUEUE_SIZE 8

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

typedef struct {
    uint32_t dst;
    uint8_t proto;
    uint8_t payload[1400];
    uint16_t len;
    uint32_t timestamp;
    int valid;
} queued_packet_t;

static uint32_t local_ip;
static queued_packet_t arp_queue[ARP_QUEUE_SIZE];
static int arp_pending = 0;

static uint16_t checksum(void *buf, int len) {
    uint32_t sum = 0;
    uint16_t *p = buf;
    while (len > 1) { 
        sum += *p++; 
        len -= 2; 
    }
    if (len) sum += *(uint8_t*)p;
    while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}

void ip_init(uint32_t ip) {
    local_ip = ip;
    memset(arp_queue, 0, sizeof(arp_queue));
}

void ip_process_arp_queue(uint32_t resolved_ip) {
    uint8_t mac[6];
    if (!arp_lookup(resolved_ip, mac)) {
        return;
    }
    for (int i = 0; i < ARP_QUEUE_SIZE; i++) {
        if (arp_queue[i].valid && arp_queue[i].dst == resolved_ip) {
            uint8_t buf[1500];
            ip_hdr_t *h = (ip_hdr_t*)buf;
            h->ver_ihl = 0x45;
            h->tos = 0;
            h->len = __builtin_bswap16(sizeof(ip_hdr_t) + arp_queue[i].len);
            h->id = 0;
            h->frag = 0;
            h->ttl = 64;
            h->proto = arp_queue[i].proto;
            h->src = local_ip;
            h->dst = arp_queue[i].dst;
            h->csum = 0;
            h->csum = checksum(h, sizeof(ip_hdr_t));
            memcpy(buf + sizeof(ip_hdr_t), arp_queue[i].payload, arp_queue[i].len);
            eth_tx(mac, ETH_TYPE_IP, buf, sizeof(ip_hdr_t) + arp_queue[i].len);
            arp_queue[i].valid = 0;
            arp_pending--;
        }
    }
}

int ip_tx(uint32_t dst, uint8_t proto, uint8_t *payload, uint16_t len) {
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
        term_puts("[IP] No ARP entry, queueing packet and sending ARP request\n");
        int slot = -1;
        for (int i = 0; i < ARP_QUEUE_SIZE; i++) {
            if (!arp_queue[i].valid) {
                slot = i;
                break;
            }
        }
        if (slot < 0) {
            term_puts("[IP] ERROR: ARP queue full, packet dropped\n");
            return -1;
        }
        arp_queue[slot].dst = dst;
        arp_queue[slot].proto = proto;
        arp_queue[slot].len = len;
        memcpy(arp_queue[slot].payload, payload, len);
        arp_queue[slot].timestamp = get_tick_count();
        arp_queue[slot].valid = 1;
        arp_pending++;
        arp_request(dst);
        return 0;
    }
    eth_tx(mac, ETH_TYPE_IP, buf, sizeof(ip_hdr_t) + len);
    term_puts("[IP] Packet sent successfully\n");
    return 1;
}

void ip_rx(uint8_t *pkt, uint16_t len) {
    ip_hdr_t *h = (ip_hdr_t*)pkt;
    if (len < sizeof(ip_hdr_t))
        return;
    term_puts("[IP] Received packet, proto=");
    char buf[16];
    itoa(h->proto, buf, 10);
    term_puts(buf);
    term_puts("\n");
    if (h->proto == 1) {
        uint16_t payload_len = len - sizeof(ip_hdr_t);
        icmp_rx(pkt + sizeof(ip_hdr_t), payload_len);
    }
}
