// an imperfect module for mapping a trapping and non-trapping 
// heap.

// quick check that <addr> is within the heap at all.
int sbrk_in_heap(uint32_t addr);

// if your allocator needs an sbrk.
//  - note: just passes through to kmalloc.
void *sbrk(long increment);

// initializes the vm system.  must be called  before.
void sbrk_init(void);

// called by checkers to allocate non-trapping memory.
void *notrap_alloc(unsigned n);

// currently doesn't do anything but could!
void notrap_free(void *ptr);
