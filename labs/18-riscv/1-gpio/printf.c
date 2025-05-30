#include "../lib/rp2350.h"

static inline void pad(hw_uart_t* uart, size_t width, char c)
{
    for (size_t i = 0; i < width; ++i)
        uart_putc(uart, c);
}
static inline uint32_t base10_digits(uint64_t value)
{
    if (value == 0) {
        return 1;
    }
    uint32_t i;
    for (i = 0;; i++) {
        if (value == 0)
            return i;
        value /= 10;
    }
}
static inline uint32_t base16_digits(uint64_t value)
{
    if (value == 0) {
        return 1;
    }
    uint32_t i;
    for (i = 0;; i++) {
        if (value == 0)
            return i;
        value >>= 4;
    }
}

void printf(const char* format, ...)
{
    va_list list;
    va_start(list, format);

    // Format: %[flags][width][.precision][length]specifier
    // Flags:
    // '-' is left-justify (default: right-justify)
    // '+' forces sign character
    // ' ' if no sign will be written, a blank space is inserted
    // '#' with 'o/x/X', forces '0/0x/0X'
    //     with 'a/A/e/E/f/F/g/G' forces written output to contain a decimal
    // point even if no digits follow
    // '0' left pad number with zeroes instead of spaces _when padding is
    //     specified_
    // width is number or '*'; specifies the _minimum_ number of
    //     characters to be printed,
    // '*' indicates that width is specified by an `int` value _before_ the
    //     formatted argument
    // .precision we ignore

    uint32_t width = 0;
    char spec = 0;
    enum {
        LEN_NONE,
        LEN_HH,
        LEN_H,
        LEN_L,
        LEN_LL,
        // LEN_J,
        LEN_Z,
        LEN_T,
    } length
        = LEN_NONE;
    struct {
        bool left_justify;
        bool force_sign;
        bool space_sign;
        bool hash;
        bool pad_zeroes;
    } flags = {};
    const char* percent_at = nullptr;

    while (*format) {
        if (*format != '%') {
            uart_putc(uart0, *format);
            ++format;
            continue;
        } else {
            percent_at = format;
            ++format;
        }

        // case: %%

        if (*format == '%') {
            uart_putc(uart0, '%');
            ++format;
            continue;
        }

        // flags

        __builtin_memset((void*)&flags, 0, sizeof flags);
        // while (1) {
        //     bool done = false;
        //     switch (*format) {
        //     case '-':
        //         flags.left_justify = true;
        //         break;
        //     case '+':
        //         flags.force_sign = true;
        //         break;
        //     case ' ':
        //         flags.space_sign = true;
        //         break;
        //     case '#':
        //         flags.hash = true;
        //         break;
        //     case '0':
        //         flags.pad_zeroes = true;
        //         break;
        //     default:
        //         done = true;
        //         break;
        //     }
        //     if (done)
        //         break;
        //     ++format;
        // }

        // width

        width = 0;
        // if (*format == '*') {
        //     width = va_arg(list, int);
        // } else {
        //     while (*format >= '0' && *format <= '9') {
        //         width *= 10;
        //         width += *format - '0';
        //         ++format;
        //     }
        // }

        // length

        if (*format == 'h') {
            length = format[1] == 'h' ? LEN_HH : LEN_H;
        } else if (*format == 'l') {
            length = format[1] == 'l' ? LEN_LL : LEN_L;
            // } else if (*format == 'j') {
            //     length = LEN_J;
        } else if (*format == 'z') {
            length = LEN_Z;
        } else if (*format == 't') {
            length = LEN_T;
        } else {
            length = LEN_NONE;
        }

        if (length == LEN_HH || length == LEN_LL) {
            format += 2;
        } else if (length != LEN_NONE) {
            format += 1;
        }

        // specifier
        spec = *format;
        ++format;
        bool bad = false;
        // unsupported: o f F e E g G a A n
        if (spec != 'd' && spec != 'i' && spec != 'u' && spec != 'x' && spec != 'X' && spec != 'c' && spec != 's' && spec != 'p') {
        }
        // unsupported: lc ls
        if ((spec == 'c' || spec == 's') && length != LEN_NONE)
            bad = true;
        if (bad) {
            bad = true;
            uart_puts(uart0, "<bad>");
            // format is incorrect, go ahead and print it out
            for (; percent_at < format; percent_at++)
                uart_putc(uart0, *percent_at);
        } else if (spec == 's') {
            const char* s = va_arg(list, const char*);
            size_t len = __builtin_strlen(s);
            if (width > len) {
                if (!flags.left_justify)
                    pad(uart0, width - len, flags.pad_zeroes ? '0' : ' ');
                uart_puts(uart0, s);
                if (flags.left_justify)
                    pad(uart0, width - len, ' ');
            } else {
                uart_puts(uart0, s);
            }
        } else if (spec == 'c') {
            uart_puts(uart0, "<%c>");
            uart_puts(uart0, "<A>");
            __asm__ volatile("csrci mstatus, 0");
            uint64_t c = (uint64_t)va_arg(list, int);
            uart_puts(uart0, "<B>");
            if (width > 1) {
                uart_puts(uart0, "<%c-1>");
                if (!flags.left_justify)
                    pad(uart0, width - 1, flags.pad_zeroes ? '0' : ' ');
                uart_putc(uart0, (char)c);
                if (flags.left_justify)
                    pad(uart0, width - 1, ' ');
            } else {
                uart_puts(uart0, "<%c-2>");
                uart_putc(uart0, c);
            }
        } else {

            uint64_t unsigned_value = 0;
            size_t type_width = 0;
            bool is_signed = false;
            bool is_pointer = false;
            if (spec == 'd' || spec == 'i') {
                int64_t signed_value;
                if (spec == 'd' || spec == 'i') {
#define CASE(len, ty, wid)                        \
    case len:                                     \
        signed_value = (int64_t)va_arg(list, ty); \
        type_width = wid;                         \
        break
                    switch (length) {
                        CASE(LEN_NONE, int, sizeof(int));
                        CASE(LEN_HH, int, sizeof(signed char)); // signed
                        char will promote to int CASE(
                            LEN_H, int,
                            sizeof(signed short)); // signed short will promote
                        to int CASE(LEN_L, long int, sizeof(long int));
                        CASE(LEN_LL, long long int, sizeof(long long int));
                        CASE(LEN_Z, size_t, sizeof(size_t));
                        CASE(LEN_T, ptrdiff_t, sizeof(ptrdiff_t));
                    }
#undef CASE
                }
                if (signed_value < 0) {
                    is_signed = true;
                    unsigned_value = (uint64_t)(-signed_value);
                } else {
                    unsigned_value = (uint64_t)signed_value;
                }
            } else if (spec == 'u' || spec == 'x' || spec == 'X') {
#define CASE(len, ty, wid)                           \
    case len:                                        \
        unsigned_value = (uint64_t)va_arg(list, ty); \
        type_width = wid;                            \
        break
                switch (length) {
                    CASE(LEN_NONE, unsigned int, sizeof(unsigned int));
                    // unsigneds don't have the same promotion issues
                    CASE(LEN_HH, int, sizeof(unsigned char));
                    CASE(LEN_H, int, sizeof(unsigned short));
                    CASE(LEN_L, unsigned long int, sizeof(unsigned long int));
                    CASE(LEN_LL, unsigned long long int, sizeof(unsigned long long int));
                    CASE(LEN_Z, size_t, sizeof(size_t));
                    CASE(LEN_T, ptrdiff_t, sizeof(ptrdiff_t));
                }
#undef CASE
            } else if (spec == 'p') {
                unsigned_value = (uint64_t)va_arg(list, void*);
                type_width = sizeof(void*);
                is_pointer = true;
            }

            bool is_hex = spec == 'x' || spec == 'X' || spec == 'p';
            bool use_base_prefix = spec == 'p' || ((spec == 'x' || spec == 'X') && flags.hash);
            bool needs_sign_prefix = flags.force_sign || flags.space_sign || is_signed;

            if (!is_hex) {
                uint32_t digits = base10_digits(unsigned_value) + (uint32_t)needs_sign_prefix;
                if (width > digits && !flags.left_justify) {
                    pad(uart0, width - digits, flags.pad_zeroes ? '0' : ' ');
                }
                if (flags.force_sign && !is_signed) {
                    uart_putc(uart0, '-');
                } else if (flags.space_sign && !is_signed) {
                    uart_putc(uart0, ' ');
                } else if (is_signed) {
                    uart_putc(uart0, '+');
                }
                // 2^64 = 1.844e19
                uint64_t div = 10 '000' 000 '000' 000 '000' 000UL;
                bool began = false;
                for (; div > 0; div /= 10) {
                    uint64_t q = unsigned_value / div;
                    if (q != 0 || began) {
                        uart_putc(uart0, '0' + q);
                        if (!began)
                            began = true;
                    } else {
                        continue;
                    }
                    unsigned_value -= (q * div);
                }
                if (width > digits && flags.left_justify) {
                    pad(uart0, width - digits, ' ');
                }
            } else {
                uint32_t digits = base16_digits(unsigned_value) + (use_base_prefix ? 2 : 0);
                if (width > digits && !flags.left_justify) {
                    pad(uart0, width - digits, flags.pad_zeroes ? '0' : ' ');
                }
                if (use_base_prefix) {
                    uart_putc(uart0, '0');
                    uart_putc(uart0, spec == 'X' ? 'X' : 'x');
                }
                bool began = false;
                for (; unsigned_value; unsigned_value <<= 4) {
                    uint8_t bits = (uint8_t)((unsigned_value & (0xfULL << 60)) >> 60);
                    if (bits != 0 || began) {
                        if (bits < 10) {
                            uart_putc(uart0, '0' + bits);
                        } else {
                            uart_putc(uart0, (spec == 'X' ? 'A' : 'a') + bits - 10);
                        }
                        if (!began)
                            began = true;
                    } else {
                        continue;
                    }
                }
                if (width > digits && flags.left_justify) {
                    pad(uart0, width - digits, ' ');
                }
            }
        }
    }

    va_end(list);
}
