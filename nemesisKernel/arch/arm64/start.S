.section .text.boot, "ax"
.global _start
.extern kernel_main
.extern __bss_start
.extern __bss_end

_start:
    /* Only run on the primary CPU for now. */
    mrs x0, mpidr_el1
    and x0, x0, #0xff
    cbnz x0, park

    /* Set up a temporary stack below the kernel load address. */
    ldr x1, =stack_top
    mov sp, x1

    /* Clear .bss. */
    ldr x2, =__bss_start
    ldr x3, =__bss_end
    cmp x2, x3
    b.hs 2f
1:
    str xzr, [x2], #8
    cmp x2, x3
    b.lo 1b
2:
    bl kernel_main

park:
    wfe
    b park

.section .bss.stack, "aw", %nobits
.align 16
stack_bottom:
    .space 65536
stack_top:
