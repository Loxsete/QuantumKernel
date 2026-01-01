#pragma once
#include <stdint.h>
#include "drivers/pci.h"

extern uint8_t mac_addr[6];

void rtl8139_init(pci_device_t *dev);
void rtl8139_irq_handler(void);
void rtl8139_send(void *data, uint16_t len);
