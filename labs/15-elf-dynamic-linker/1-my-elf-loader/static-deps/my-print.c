/*
    Stuart: Very minimal version of print functions without dependencies

    The goal is to have as little as possible, and dynamically link the rest.
*/

#include "rpi.h"

int putk(const char *p) {
    for(; *p; p++)
        uart_put8(*p);
    return 1;
}

static int putchar(int c) { 
    uart_put8(c); 
    return c; 
}

static void emit_val(unsigned base, uint32_t u) {
    char num[33], *p = num;

    switch(base) {
    case 2:
        do {
            *p++ = "01"[u % 2];
        } while(u /= 2);
        break;
    case 10:
        do {
            *p++ = "0123456789"[u % 10];
        } while(u /= 10);
        break;
    case 16:
        do {
            *p++ = "0123456789abcdef"[u % 16];
        } while(u /= 16);
        break;
    default: 
        panic("invalid base=%d\n", base);
    }

    // buffered in reverse, so emit backwards
    while(p > &num[0]) {
        p--;
        putchar(*p);
    }
}

int vprintk(const char *fmt, va_list ap) {
    for(; *fmt; fmt++) {
        if(*fmt != '%')
            putchar(*fmt);
        else {
            fmt++; // skip the %

            uint32_t u;
            int v;
            char *s;

            switch(*fmt) {
            case 'b': emit_val(2, va_arg(ap, uint32_t)); break;
            case 'u': emit_val(10, va_arg(ap, uint32_t)); break;
            case 'c': putchar(va_arg(ap, int)); break;

            // we only handle %llx.   
            case 'l':  
                fmt++;
                if(*fmt != 'l')
                    panic("only handling llx format, have: <%s>\n", fmt);
                fmt++;
                if(*fmt != 'x')
                    panic("only handling llx format, have: <%s>\n", fmt);
                putchar('0');
                putchar('x');
                uint64_t x = va_arg(ap, uint64_t);
                uint32_t hi = x>>32;
                uint32_t lo = x;
                if(hi)
                    emit_val(16, hi);
                emit_val(16, lo);
                break;

            // leading 0x
            case 'x':  
            case 'p': 
                putchar('0');
                putchar('x');
                emit_val(16, va_arg(ap, uint32_t));
                break;
            // print '-' if < 0
            case 'd':
                v = va_arg(ap, int);
                if(v < 0) {
                    putchar('-');
                    v = -v;
                }
                emit_val(10, v);
                break;
            // string
            case 's':
                for(s = va_arg(ap, char *); *s; s++) 
                    putchar(*s);
                break;
            default: panic("bogus identifier: <%c>\n", *fmt);
            }
        }
    }
    return 0;
}

int printk(const char *fmt, ...) {
    va_list args;

    int ret;
    va_start(args, fmt);
       ret = vprintk(fmt, args);
    va_end(args);
    return ret;
}

#define panic(msg, args...) do { 					    \
	(printk)("PANIC:%s:%s:%d:" msg "\n",                \
        __FILE__, __FUNCTION__, __LINE__, ##args);      \
	clean_reboot();							            \
} while(0)
