#include <nemesis/mm.h>

extern u8 __kernel_end[];

static u8 *heap_next;

void pmm_init(void)
{
    usize address = (usize)__kernel_end;
    address = (address + 15UL) & ~15UL;
    heap_next = (u8 *)address;
}

void *pmm_alloc(usize size)
{
    usize address = (usize)heap_next;
    address = (address + 15UL) & ~15UL;

    heap_next = (u8 *)(address + size);
    return (void *)address;
}
