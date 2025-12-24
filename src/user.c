#include "syscall.h"
#include "string.h"

#define BUF_SIZE 128

static void strip_newline(char* s) {
    int len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n' || s[len - 1] == '\r') 
        s[len - 1] = 0;
}

static void print_hex_byte(uint8_t b) {
    const char hex[] = "0123456789ABCDEF";
    char out[3];
    out[0] = hex[b >> 4];
    out[1] = hex[b & 0xF];
    out[2] = ' ';
    write(1, out, 3);
}

void user_main(void) {
    char buf[BUF_SIZE];
    int pos = 0;
    
    write(1, "Simple shell ready!\n> ", 22);
    
    while (1) {
        char c;
        int n = read(0, &c, 1);
        if (n <= 0) continue;
        
        if (c == '\b') {
            if (pos > 0) {
                pos--;
                write(1, "\b \b", 3);
            }
            continue;
        }
        
        write(1, &c, 1);
        
        if (c == '\n' || c == '\r') {
            buf[pos] = 0;
            strip_newline(buf);
            
            if (pos > 0) {
                if (strcmp(buf, "help") == 0) {
                    write(1, "Available commands:\n", 20);
                    write(1, "  help       - show this help\n", 30);
                    write(1, "  clear      - clear screen\n", 27);
                    write(1, "  diskread   - read sector 0\n", 29);
                    write(1, "  diskwrite  - write test data\n", 31);
                    
                } else if (strcmp(buf, "clear") == 0) {
                    clear();
                    
                } else if (strcmp(buf, "diskread") == 0) {
                    uint8_t sector[512];
                    write(1, "Reading sector 0...\n", 20);
                    
                    int result = disk_read(0, sector);
                    if (result == 0) {
                        write(1, "Success! First 64 bytes:\n", 25);
                        for (int i = 0; i < 64; i++) {
                            print_hex_byte(sector[i]);
                            if ((i + 1) % 16 == 0)
                                write(1, "\n", 1);
                        }
                    } else {
                        write(1, "Error reading disk\n", 19);
                    }
                    
                } else if (strcmp(buf, "diskwrite") == 0) {
                    uint8_t sector[512];
                    
                    // Заполняем тестовыми данными
                    for (int i = 0; i < 512; i++)
                        sector[i] = i & 0xFF;
                    
                    // Записываем в сектор 1 (не 0, чтобы не испортить MBR)
                    write(1, "Writing test data to sector 1...\n", 34);
                    int result = disk_write(1, sector);
                    
                    if (result == 0) {
                        write(1, "Write successful!\n", 18);
                    } else {
                        write(1, "Error writing disk\n", 19);
                    }
                    
                } else {
                    write(1, "Unknown command\n", 16);
                }
            }
            
            pos = 0;
            write(1, "> ", 2);
            
        } else {
            if (pos < BUF_SIZE - 1) {
                buf[pos] = c;
                pos++;
            }
        }
    }
}
