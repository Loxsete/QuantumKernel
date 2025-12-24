#ifndef TERMINAL_H
#define TERMINAL_H

void term_init(void);
void term_clear(void);
void term_putc(char c);
void term_puts(const char* s);
void term_process_char(char c);  

#endif
