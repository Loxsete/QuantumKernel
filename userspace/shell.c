

#include "syscall.h"
#include "string.h"
#include "stdio.h"

#define BUF_SIZE 256
#define FILE_BUF 512

static char prompt_buf[128];

static void strip_newline(char* s) {
    int l = strlen(s);
    while (l > 0 && (s[l - 1] == '\n' || s[l - 1] == '\r')) {
        s[l - 1] = 0;
        l--;
    }
}

static void puts(const char* s) {
    write(STDOUT, s, strlen(s));
}

static void prompt(void) {
    if (getcwd(prompt_buf, sizeof(prompt_buf)) > 0) {
        puts(prompt_buf);
    }
    puts("> ");
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

void _start(void) {  
    char buf[BUF_SIZE];
    prompt_buf[0] = '\0';

    puts("\nWelcome to Quantum Kernel shell\n");
    puts("Type 'help' for available commands.\n");

    prompt();
    
    while (1) {
        int pos = 0;
        char c;

        while (1) {
            if (read(STDIN, &c, 1) <= 0) continue;

            if (c == '\b') {
                if (pos > 0) {
                    pos--;
                    write(STDOUT, "\b \b", 3);
                }
                continue;
            }

            write(STDOUT, &c, 1);

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
            puts("Commands:\n"
                 " help, clear, cat, write, append, rm, mkdir\n"
                 " ls, cd, pwd, sleep, time\n"
                 " netinit, ping, arp, netstat, testnet\n"
                 " reboot, poweroff, halt, ps, kill, exec\n");
        }

        else if (!strcmp(cmd, "clear")) {
            clear();
        }

        else if (!strcmp(cmd, "reboot")) {
            puts("Rebooting in 2 seconds...\n");
            sleep_ms(2000);
            reboot();
        }

        else if (!strcmp(cmd, "poweroff") || !strcmp(cmd, "shutdown")) {
            puts("Powering off...\n");
            sleep_ms(1000);
            shutdown();
        }

        else if (!strcmp(cmd, "halt")) {
            puts("System halted.\n");
            halt();
        }

        else if (!strcmp(cmd, "cat")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: cat <file>\n");
            } else {
                int fd = open(path, O_RDONLY);
                if (fd < 0) {
                    puts("open failed\n");
                } else {
                    char fbuf[FILE_BUF];
                    int n;
                    while ((n = file_read(fd, fbuf, FILE_BUF)) > 0) {
                        write(STDOUT, fbuf, n);
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
                    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);
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
                    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND);
                    if (fd < 0) {
                        puts("open failed\n");
                    } else {
                        seek(fd, 0, SEEK_END);
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

        else if (!strcmp(cmd, "ls")) {
            file_info_t info;
            uint32_t cluster = get_cwd_cluster();
            puts("Directory listing:\n");
            uint32_t index = 0;
            int count = 0;
            while (readdir(cluster, &index, &info) == 0) {
                if (info.attr & 0x10) {
                    puts("<DIR> ");
                } else {
                    puts("      ");
                }
                puts(info.name);
                puts("\n");
                count++;
                if (count > 100) break;
            }
            if (count == 0) {
                puts("(empty)\n");
            }
        }
        
        else if (!strcmp(cmd, "cd")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: cd <dir>\n");
            } else {
                if (chdir(path) == 0)
                    puts("ok\n");
                else
                    puts("cd failed\n");
            }
        }
        
        else if (!strcmp(cmd, "pwd")) {
            if (getcwd(prompt_buf, sizeof(prompt_buf)) > 0) {
                puts(prompt_buf);
                puts("\n");
            } else {
                puts("getcwd failed\n");
            }
        }
        
        else if (!strcmp(cmd, "sleep")) {
            char* t = next_token(&p);
            if (t) {
                int ms = 0;
                
                while (*t >= '0' && *t <= '9') {
                    ms = ms * 10 + (*t - '0');
                    t++;
                }
                sleep_ms(ms);
            }
        }

        else if (!strcmp(cmd, "ps")) {
            ps();
        }

        else if (!strcmp(cmd, "kill")) {
            char* ppid = next_token(&p);
            if (!ppid) {
                puts("usage: kill <pid>\n");
            } else {
                int pid = 0;
                while (*ppid >= '0' && *ppid <= '9') {
                    pid = pid * 10 + (*ppid - '0');
                    ppid++;
                }
                if (kill(pid) == 0)
                    puts("killed\n");
                else
                    puts("no such pid\n");
            }
        }

        else if (!strcmp(cmd, "exec")) {
            char* path = next_token(&p);
            if (!path) {
                puts("usage: exec <program>\n");
            } else {
                int result = exec(path);
                if (result != 0) {
                    puts("exec failed\n");
                }
            }
        }

        else {
            puts("unknown command\n");
        }

        prompt();
    }
    
    exit(); 
}
