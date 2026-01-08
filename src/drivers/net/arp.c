#include "drivers/net/arp.h"
#include "drivers/net/ethernet.h"
#include "drivers/terminal.h"
#include "drivers/timer.h"
#include "lib/libc.h"
#include <string.h>

#define ARP_TABLE_SIZE 16

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

typedef struct {
    uint32_t ip;
    uint8_t mac[6];
    uint32_t timestamp;
    int valid;
} arp_entry_t;

static uint32_t local_ip;
static arp_entry_t arp_table[ARP_TABLE_SIZE];

extern void ip_process_arp_queue(uint32_t resolved_ip);

void arp_init(uint32_t ip) {
    local_ip = ip;
    memset(arp_table, 0, sizeof(arp_table));
    term_puts("[ARP] Initialized with IP: ");
    char buf[16];
    itoa(ip, buf, 10);
    term_puts(buf);
    term_puts("\n");
}

int arp_lookup(uint32_t ip, uint8_t *mac) {
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (arp_table[i].valid && arp_table[i].ip == ip) {
            memcpy(mac, arp_table[i].mac, 6);
            term_puts("[ARP] Cache hit for IP: ");
            char buf[16];
            itoa(ip, buf, 10);
            term_puts(buf);
            term_puts("\n");
            return 1;
        }
    }
    term_puts("[ARP] Cache miss for IP: ");
    char buf[16];
    itoa(ip, buf, 10);
    term_puts(buf);
    term_puts("\n");
    return 0;
}

void arp_add_entry(uint32_t ip, uint8_t *mac) {
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (arp_table[i].valid && arp_table[i].ip == ip) {
            memcpy(arp_table[i].mac, mac, 6);
            arp_table[i].timestamp = get_tick_count();
            term_puts("[ARP] Updated entry for IP: ");
            char buf[16];
            itoa(ip, buf, 10);
            term_puts(buf);
            term_puts(" MAC: ");
            for (int j = 0; j < 6; j++) {
                itoa(mac[j], buf, 16);
                term_puts(buf);
                if (j < 5) term_puts(":");
            }
            term_puts("\n");
            return;
        }
    }
    
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (!arp_table[i].valid) {
            arp_table[i].ip = ip;
            memcpy(arp_table[i].mac, mac, 6);
            arp_table[i].timestamp = get_tick_count();
            arp_table[i].valid = 1;
            term_puts("[ARP] Added new entry for IP: ");
            char buf[16];
            itoa(ip, buf, 10);
            term_puts(buf);
            term_puts(" MAC: ");
            for (int j = 0; j < 6; j++) {
                itoa(mac[j], buf, 16);
                term_puts(buf);
                if (j < 5) term_puts(":");
            }
            term_puts("\n");
            ip_process_arp_queue(ip);
            return;
        }
    }
    
    term_puts("[ARP] WARNING: Table full, removing oldest entry\n");
    int oldest = 0;
    for (int i = 1; i < ARP_TABLE_SIZE; i++) {
        if (arp_table[i].timestamp < arp_table[oldest].timestamp) {
            oldest = i;
        }
    }
    arp_table[oldest].ip = ip;
    memcpy(arp_table[oldest].mac, mac, 6);
    arp_table[oldest].timestamp = get_tick_count();
    arp_table[oldest].valid = 1;
    ip_process_arp_queue(ip);
}

void arp_request(uint32_t ip) {
    term_puts("[ARP] Sending request for IP: ");
    char buf[16];
    itoa(ip, buf, 10);
    term_puts(buf);
    term_puts("\n");
    uint8_t arp_buf[sizeof(arp_pkt_t)];
    arp_pkt_t *a = (arp_pkt_t*)arp_buf;
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
    eth_tx(bcast, ETH_TYPE_ARP, arp_buf, sizeof(arp_buf));
}

void arp_rx(uint8_t *pkt) {
    arp_pkt_t *a = (arp_pkt_t*)pkt;
    uint16_t oper = __builtin_bswap16(a->oper);
    if (oper == 1) {
        term_puts("[ARP] Received request\n");
        if (a->tpa == local_ip) {
            term_puts("[ARP] Request is for us, sending reply\n");
            uint8_t reply_buf[sizeof(arp_pkt_t)];
            arp_pkt_t *r = (arp_pkt_t*)reply_buf;
            r->htype = __builtin_bswap16(1);
            r->ptype = __builtin_bswap16(0x0800);
            r->hlen = 6;
            r->plen = 4;
            r->oper = __builtin_bswap16(2);
            memcpy(r->sha, eth_mac(), 6);
            r->spa = local_ip;
            memcpy(r->tha, a->sha, 6);
            r->tpa = a->spa;
            eth_tx(a->sha, ETH_TYPE_ARP, reply_buf, sizeof(reply_buf));
        }
        arp_add_entry(a->spa, a->sha);
    } else if (oper == 2) {
        term_puts("[ARP] Received reply from IP: ");
        char buf[16];
        itoa(a->spa, buf, 10);
        term_puts(buf);
        term_puts("\n");
        arp_add_entry(a->spa, a->sha);
    }
}

void arp_print_table(void) {
    term_puts("\n=== ARP Table ===\n");
    int count = 0;
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (arp_table[i].valid) {
            char buf[16];
            term_puts("IP: ");
            itoa(arp_table[i].ip, buf, 10);
            term_puts(buf);
            term_puts(" -> MAC: ");
            for (int j = 0; j < 6; j++) {
                itoa(arp_table[i].mac[j], buf, 16);
                term_puts(buf);
                if (j < 5) term_puts(":");
            }
            term_puts("\n");
            count++;
        }
    }
    if (count == 0) {
        term_puts("(empty)\n");
    }
    term_puts("=================\n\n");
}

int arp_get_entry_by_index(int index, uint32_t* ip, uint8_t* mac, int* valid) {
    if (index < 0 || index >= ARP_TABLE_SIZE) {
        return -1;
    }
    *ip = arp_table[index].ip;
    memcpy(mac, arp_table[index].mac, 6);
    *valid = arp_table[index].valid;
    return 0;
}
