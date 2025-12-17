#include <kernel/drivers/pci.h>
#include <kernel/arch/x86_64/io.h>
#include <kernel/console.h>

namespace Core {

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | 
                      (func << 8) | (offset & 0xFC) | 0x80000000);
    
    IO::outl(PCI_CONFIG_ADDRESS, address);
    return IO::inl(PCI_CONFIG_DATA);
}

void PCI::initialize() {
    int device_count = 0;
    
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            for (int func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_read(bus, slot, func, 0);
                uint16_t vendor = vendor_device & 0xFFFF;
                
                if (vendor != 0xFFFF) {
                    device_count++;
                }
            }
        }
    }
    
    Console::printf("[PCI] Found %d devices\n", device_count);
}

}