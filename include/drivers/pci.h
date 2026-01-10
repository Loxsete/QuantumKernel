#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define MAX_PCI_DEVICES 256

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint32_t bar[6];
} pci_device_t;

typedef void (*pci_driver_init_t)(pci_device_t *dev);

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    pci_driver_init_t init;
} pci_driver_t;

extern pci_device_t pci_devices[MAX_PCI_DEVICES];
extern int pci_device_count;

uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t data);
void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint16_t data);
void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint8_t data);
uint16_t pci_find_device(uint16_t vendor_id, uint16_t device_id);

void pci_enumerate(void);
void pci_register_drivers(void);

#endif
