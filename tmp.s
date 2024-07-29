    .globl main
main:
    li a0, 1
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 1
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 4
    ld a1, 0(sp)
    addi sp, sp, 8
    add a0, a0, a1
    ld a1, 0(sp)
    addi sp, sp, 8
    sub a0, a0, a1
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 2
    ld a1, 0(sp)
    addi sp, sp, 8
    mul a0, a0, a1
    ret
