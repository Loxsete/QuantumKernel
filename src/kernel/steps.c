#include "drivers/terminal.h"
#include "kernel/steps.h"
#include "lib/string.h"


void boot_step(const char *name)
{
    term_puts("-> ");
    term_puts(name);
    int len = strlen(name);
    for (int i = 0; i < 30 - len; i++)
        term_putc(' ');
}

void boot_ok(void)
{
    term_set_color(TERM_COLOR_OK);
    term_puts("[ OK ]\n");
    term_reset_color();
}

void boot_fail(void)
{
    term_set_color(TERM_COLOR_FAIL);
    term_puts("[ FAIL ]\n");
    term_reset_color();
    term_puts("kernel panic: fatal error\n");
    asm volatile("cli");
    for (;;)
        asm volatile("hlt");
}
