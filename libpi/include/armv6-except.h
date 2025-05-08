// b4-20
enum {
    SECTION_XLATE_FAULT = 0b00101,
    SECTION_PERM_FAULT = 0b1101,
    DOMAIN_SECTION_FAULT = 0b1001,
};

// b4-43: get the data abort reason.
uint32_t data_abort_reason(void);

// b4-44: get the data abort addr.
uint32_t data_abort_addr(void) ;

// returns the reason.
uint32_t prefetch_reason(void) ;

// returns faulting addr
uint32_t prefetch_addr(void);

// was fault caused by a load?
unsigned data_fault_from_ld(void);
