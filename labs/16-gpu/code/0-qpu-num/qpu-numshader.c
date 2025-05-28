#include "qpu-numshader.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
__declspec(align(8))
#elif defined(__GNUC__)
__attribute__((aligned(8)))
#endif
uint32_t qpu_numshader[34] = {
0x15827d80, 0x10020027,
0x15827d80, 0x10020067,
0x150e7d80, 0x100202a7,
0x00101a00, 0xe00208a7,
0x0c027c80, 0x10021c67,
0x159e6fc0, 0x10020c27,
0x159f2fc0, 0x100009e7,
0x11007dc0, 0xd0020867,
0x80904000, 0xe00208a7,
0x0c9e7280, 0x10021c67,
0x15027d80, 0x10020867,
0x119c63c0, 0xd0020867,
0x0c067c40, 0x10021ca7,
0x159f2fc0, 0x100009e7,
0x009e7000, 0x300009e7,
0x00000001, 0xe00209a7,
0x009e7000, 0x100009e7
};
#ifdef __HIGHC__
#pragma Align_to(8, qpu_numshader)
#ifdef __cplusplus
}
#endif
#endif
