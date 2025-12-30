#include "drivers/pci.h"
#include "drivers/terminal.h"
#include "drivers/io.h"

pci_device_t pci_devices[MAX_PCI_DEVICES];
int pci_device_count = 0;
static void pci_check_function(uint8_t bus, uint8_t device, uint8_t func);
static void pci_enumerate_bus(uint8_t bus);


static uint32_t pci_make_address(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    return (uint32_t)(0x80000000 | (bus << 16) | (device << 11) | (func << 8) | (offset & 0xFC));
}

uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, pci_make_address(bus, device, func, offset));
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, func, offset & ~3);
    return (uint16_t)((dword >> ((offset & 3) * 8)) & 0xFFFF);
}

uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, func, offset & ~3);
    return (uint8_t)((dword >> ((offset & 3) * 8)) & 0xFF);
}

void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t data) {
    outl(PCI_CONFIG_ADDRESS, pci_make_address(bus, device, func, offset));
    outl(PCI_CONFIG_DATA, data);
}

void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint16_t data) {
    uint32_t dword = pci_config_read_dword(bus, device, func, offset & ~3);
    uint32_t shift = (offset & 3) * 8;
    dword = (dword & ~(0xFFFF << shift)) | ((data & 0xFFFF) << shift);
    pci_config_write_dword(bus, device, func, offset & ~3, dword);
}

void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint8_t data) {
    uint32_t dword = pci_config_read_dword(bus, device, func, offset & ~3);
    uint32_t shift = (offset & 3) * 8;
    dword = (dword & ~(0xFF << shift)) | ((data & 0xFF) << shift);
    pci_config_write_dword(bus, device, func, offset & ~3, dword);
}

static void pci_enumerate_bus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        uint16_t vendor = pci_config_read_word(bus, device, 0, 0);
        if (vendor == 0xFFFF) continue;

        pci_check_function(bus, device, 0);

        uint8_t header = pci_config_read_byte(bus, device, 0, 0xE);
        if (header & 0x80) {
            for (uint8_t func = 1; func < 8; func++) {
                if (pci_config_read_word(bus, device, func, 0) != 0xFFFF) {
                    pci_check_function(bus, device, func);
                }
            }
        }
    }
}

static void pci_check_function(uint8_t bus, uint8_t device, uint8_t func) {
    uint16_t vendor = pci_config_read_word(bus, device, func, 0);
    if (vendor == 0xFFFF) return;

    pci_device_t *dev = &pci_devices[pci_device_count++];
    dev->bus = bus;
    dev->device = device;
    dev->function = func;
    dev->vendor_id = vendor;
    dev->device_id = pci_config_read_word(bus, device, func, 2);
    dev->class_code = pci_config_read_byte(bus, device, func, 0xB);
    dev->subclass = pci_config_read_byte(bus, device, func, 0xA);
    dev->prog_if = pci_config_read_byte(bus, device, func, 0x9);
    dev->interrupt_line = pci_config_read_byte(bus, device, func, 0x3C);
    dev->interrupt_pin = pci_config_read_byte(bus, device, func, 0x3D);

    for (int i = 0; i < 6; i++) {
        dev->bar[i] = pci_config_read_dword(bus, device, func, 0x10 + i * 4);
    }

    if (dev->class_code == 0x06 && dev->subclass == 0x04) {
        uint8_t secondary = pci_config_read_byte(bus, device, func, 0x19);
        pci_enumerate_bus(secondary);
    }
}



void pci_enumerate(void) {
    uint8_t header = pci_config_read_byte(0, 0, 0, 0xE);
    if (header & 0x80) {
        for (uint8_t func = 0; func < 8; func++) {
            if (pci_config_read_word(0, 0, func, 0) != 0xFFFF) {
                pci_enumerate_bus(func);
            }
        }
    } else {
        pci_enumerate_bus(0);
    }
}

extern void rtl8139_init(pci_device_t *dev);

pci_driver_t pci_drivers[] = {
    {0x10EC, 0x8139, rtl8139_init},
};

int pci_driver_count = sizeof(pci_drivers) / sizeof(pci_driver_t);

void pci_register_drivers(void) {
    for (int i = 0; i < pci_device_count; i++) {
        pci_device_t *dev = &pci_devices[i];
        for (int j = 0; j < pci_driver_count; j++) {
            if (pci_drivers[j].vendor_id == dev->vendor_id &&
                pci_drivers[j].device_id == dev->device_id) {
                pci_drivers[j].init(dev);
            }
        }
    }
}
