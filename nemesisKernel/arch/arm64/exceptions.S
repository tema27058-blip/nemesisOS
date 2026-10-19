.section .text.exceptions, "ax"
.align 11
.global exception_vectors
.global exceptions_init
.extern exception_handler

exception_vectors:
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128

    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128

    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128

    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128
    b exception_entry
    .balign 128

exception_entry:
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    stp x30, xzr, [sp, #-16]!

    mrs x0, esr_el1
    mrs x1, far_el1
    bl exception_handler

1:
    wfe
    b 1b

exceptions_init:
    ldr x0, =exception_vectors
    msr vbar_el1, x0
    isb
    ret
