#include "drivers/net/icmp.h"
#include "drivers/net/ip.h"
#include "lib/libc.h"
#include "drivers/terminal.h"
#include "drivers/timer.h"

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint16_t id;
    uint16_t seq;
} __attribute__((packed)) icmp_hdr_t;

static uint16_t ping_id = 1;
static uint16_t ping_seq = 0;
static int ping_waiting = 0;
static uint32_t ping_sent_time = 0;

static uint16_t checksum(void *buf, int len) {
    uint32_t sum = 0;
    uint16_t *p = buf;
    while (len > 1) { sum += *p++; len -= 2; }
    if (len) sum += *(uint8_t*)p;
    while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}

void icmp_ping(uint32_t ip) {
    uint8_t buf[64];
    icmp_hdr_t *h = (icmp_hdr_t*)buf;
    h->type = 8;
    h->code = 0;
    h->id = ping_id;
    h->seq = ++ping_seq;
    h->csum = 0;
    h->csum = checksum(buf, sizeof(icmp_hdr_t));
    
    ping_waiting = 1;
    ping_sent_time = get_tick_count();
    
    ip_tx(ip, 1, buf, sizeof(icmp_hdr_t));
}

void icmp_rx(uint8_t *pkt, uint16_t len) {
    if (len < sizeof(icmp_hdr_t))
        return;
    
    icmp_hdr_t *h = (icmp_hdr_t*)pkt;
    
    if (h->type == 0 && ping_waiting) {
        uint32_t rtt = get_tick_count() - ping_sent_time;
        ping_waiting = 0;
        
        char buf[64];
        term_puts("Reply from host: seq=");
        itoa(h->seq, buf, 10);
        term_puts(buf);
        term_puts(" time=");
        itoa(rtt, buf, 10);
        term_puts(buf);
        term_puts("ms\n");
    }
}

int icmp_get_status(void) {
    return ping_waiting;
}

void icmp_reset_status(void) {
    ping_waiting = 0;
}
