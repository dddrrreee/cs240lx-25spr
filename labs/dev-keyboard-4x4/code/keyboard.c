// trivial 4x4 driver.
#include "rpi.h"

enum {
    R4 = 7,
    R3 = 9,
    R2 = 11,
    R1 = 13,
    C1 = 25,
    C2 = 21,
    C3 = 19,
    C4 = 17,
};

const char *key_str[] = {
    "S1", "S2", "S3", "S4",           // row 0
    "S5", "S6", "S7", "S8",           // row 1
    "S9", "S10", "S11", "S12",        // row 2
    "S13", "S14", "S15", "S16",        // row 2
};

static const char *key_to_str(unsigned i, unsigned j) {
    assert(i<4);
    assert(j<4);
    return key_str[i*4+j];
}

typedef struct {
    unsigned row_pins[4]; 
    unsigned col_pins[4];

    // key[i][j] = row i, col j
    unsigned keys[4][4];
} input_4x4_t;

// scan <kb> looking for which keys are held down:  print them.
// return how many.
static int scan_4x4(input_4x4_t *kb) {
    todo("print every key held down");
}

static inline input_4x4_t init_4x4(void) {
    input_4x4_t kb = {
        .row_pins = { R1, R2, R3, R4 },
        .col_pins = { C1, C2, C3, C4 },
    };

    for(int i = 0; i < 4; i++) {
        gpio_set_output(kb.row_pins[i]);
        gpio_set_input(kb.col_pins[i]);
        gpio_set_pulldown(kb.col_pins[i]);
    }
    return kb;
}

void notmain(void) {
    let kb = init_4x4();

    enum { N = 16 };
    while(1) {
        let nchanges = scan_4x4(&kb);
        if(nchanges) {
            output("nchanges=%d\n", nchanges);
        }
    }
}
