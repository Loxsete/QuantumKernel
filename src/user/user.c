#include "syscall/syscall.h"
#include "lib/libc.h"
#include "lib/string.h"
#include "lib/rtc.h"   
#include <stdint.h>

#define BUF_SIZE 256
#define FILE_BUF 512

static void strip_newline(char* s) {
    int l = strlen(s);
    while (l > 0 && (s[l - 1] == '\n' || s[l - 1] == '\r')) {
        s[l - 1] = 0;
        l--;
    }
}

static void puts(const char* s) {
    write(1, s, strlen(s));
}

static void prompt(void) {
    puts("\n> ");
}

static char* next_token(char** s) {
    while (**s == ' ') (*s)++;
    if (**s == 0) return 0;
    char* start = *s;
    while (**s && **s != ' ') (*s)++;
    if (**s) {
        **s = 0;
        (*s)++;
    }
    return start;
}

void user_main(void) {
    char buf[BUF_SIZE];

    puts(
        "\n"
        "Welcome to Quantum Kernel shell\n"
        "Type 'help' for available commands.\n"
    );

    prompt();
    
    while (1) {
        int pos = 0;
        char c;

        while (1) {
            if (read(0, &c, 1) <= 0) continue;

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
                break;
            }

            if (pos < BUF_SIZE - 1)
                buf[pos++] = c;
        }

        strip_newline(buf);
        if (buf[0] == 0) {
            prompt();
            continue;
        }

        char* p = buf;
        char* cmd = next_token(&p);

        if (!strcmp(cmd, "help")) {
            puts(
                "Commands:\n"
                " help\n"
                " clear\n"
                " cat <file>\n"
                " write <file> <text>\n"
                " append <file> <text>\n"
                " rm <file>\n"
                " mkdir <dir>\n"
                " ls\n"
                " cd <dir>\n"
                " pwd\n"
                " seektest <file>\n"
                " sleep <ms>\n"
                " time\n"
                " netinit <ip> <gateway> <netmask>\n"
                " ping <ip>\n"
            );
        }

        else if (!strcmp(cmd, "clear")) {
            clear();
        }

        else if (!strcmp(cmd, "cat")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: cat <file>\n");
            } else {
                int fd = open(path, 0);
                if (fd < 0) {
                    puts("open failed\n");
                } else {
                    char fbuf[FILE_BUF];
                    int n;
                    while ((n = file_read(fd, fbuf, FILE_BUF)) > 0) {
                        write(1, fbuf, n);
                    }
                    close(fd);
                }
            }
        }

        else if (!strcmp(cmd, "write")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: write <file> <text>\n");
            } else {
                while (*p == ' ') p++;
                
                if (*p == 0) {
                    puts("usage: write <file> <text>\n");
                } else {
                    int fd = open(path, 6);
                    if (fd < 0) {
                        puts("open failed\n");
                    } else {
                        file_write(fd, p, strlen(p));  
                        file_write(fd, "\n", 1);
                        close(fd);
                        puts("ok\n");
                    }
                }
            }
        }
        
        else if (!strcmp(cmd, "append")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: append <file> <text>\n");
            } else {
                while (*p == ' ') p++;
                
                if (*p == 0) {
                    puts("usage: append <file> <text>\n");
                } else {
                    int fd = open(path, 22);
                    if (fd < 0) {
                        puts("open failed\n");
                    } else {
                        seek(fd, 0, 2);
                        file_write(fd, p, strlen(p));  
                        file_write(fd, "\n", 1);
                        close(fd);
                        puts("ok\n");
                    }
                }
            }
        }

        else if (!strcmp(cmd, "rm")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: rm <file>\n");
            } else {
                if (unlink(path) == 0)
                    puts("deleted\n");
                else
                    puts("unlink failed\n");
            }
        }

        else if (!strcmp(cmd, "mkdir")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: mkdir <dir>\n");
            } else {
                if (mkdir(path) == 0)
                    puts("created\n");
                else
                    puts("mkdir failed\n");
            }
        }

        else if (!strcmp(cmd, "seektest")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: seektest <file>\n");
            } else {
                int fd = open(path, 0);
                if (fd < 0) {
                    puts("open failed\n");
                } else {
                    char x[8] = {0};
                    seek(fd, 5, 0);
                    file_read(fd, x, 5);
                    puts("data@5: ");
                    puts(x);
                    puts("\n");
                    close(fd);
                }
            }
        }

        else if (!strcmp(cmd, "ls")) {
            typedef struct {
                char name[32];
                uint32_t size;
                uint8_t attr;
                uint32_t first_cluster;
            } file_info_t;
            
            file_info_t info;
            uint32_t cluster;
            
            cluster = get_cwd_cluster_sys();  
            
            puts("Directory listing:\n");
            
            uint32_t index = 0;
            int count = 0;
            
            while (readdir_sys(cluster, &index, &info) == 0) {
                if (info.attr & 0x10) {
                    puts("<DIR> ");
                } else {
                    puts("      ");
                }
                
                puts(info.name);
                puts("\n");
                
                count++;
                
                if (count > 100) {
                    puts("...(too many entries)\n");
                    break;
                }
            }
            
            if (count == 0) {
                puts("(empty directory)\n");
            }
        }
        
        else if (!strcmp(cmd, "cd")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: cd <dir>\n");
            } else {
                if (chdir_sys(path) == 0)
                    puts("ok\n");
                else
                    puts("cd failed\n");
            }
        }
        
        else if (!strcmp(cmd, "pwd")) {
            uint32_t cluster = get_cwd_cluster_sys();
            char buf[16];
            itoa(cluster, buf, 10);
            puts("cluster: ");
            puts(buf);
            puts("\n");
        }

        else if (!strcmp(cmd, "sleep")) {
            char* t = next_token(&p);
            if (t)
                sleep_sys(atoi(t));
        }

        else if (!strcmp(cmd, "time")) {
            rtc_time_t t;
            rtc_time_sys(&t);  
            int tz = timezone_sys();
        
            char buf[64];
            itoa(t.hour, buf, 10);
            puts(buf);
            puts(":");
            itoa(t.min, buf, 10);
            puts(buf);
            puts(":");
            itoa(t.sec, buf, 10);
            puts(buf);
        
            puts(" UTC");
            itoa(tz, buf, 10);
            puts(buf);
            puts("\n");
        }

        else if (!strcmp(cmd, "netinit")) {
            char* ip_str = next_token(&p);
            char* gw_str = next_token(&p);
            char* nm_str = next_token(&p);
            
            if (!ip_str || !gw_str || !nm_str) {
                puts("usage: netinit <ip> <gateway> <netmask>\n");
                puts("example: netinit 3232235777 3232235521 4294967040\n");
            } else {
                uint32_t ip = atoi(ip_str);
                uint32_t gw = atoi(gw_str);
                uint32_t nm = atoi(nm_str);
                
                if (net_init_sys(ip, gw, nm) == 0)
                    puts("network initialized\n");
                else
                    puts("network init failed\n");
            }
        }
        
        else if (!strcmp(cmd, "ping")) {
            char* ip_str = next_token(&p);
            if (!ip_str) {
                puts("usage: ping <ip>\n");
                puts("example: ping 134744072\n");
            } else {
                uint32_t ip = atoi(ip_str);
                
                ping_reset_sys();
                ping_sys(ip);
                puts("Pinging... ");
                
                uint32_t timeout = 0;
                while (ping_status_sys() && timeout < 5000) {
                    sleep_sys(10);
                    timeout += 10;
                }
                
                if (ping_status_sys()) {
                    puts("Request timed out.\n");
                }
            }
        }
		
		
        else {
            puts("unknown command\n");
        }

        prompt();
    }
}
