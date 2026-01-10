#ifndef POWER_H
#define POWER_H

void acpi_init(void);
int acpi_is_available(void);
void power_reboot(void);
void power_shutdown(void);
void power_halt(void);
void power_suspend(void);

#endif
