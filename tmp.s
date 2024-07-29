    .globl main
main:
    li a0, 23
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 12
    ld a1, 0(sp)
    addi sp, sp, 8
    add a0, a0, a1
    li a0, 3
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 99
    ld a1, 0(sp)
    addi sp, sp, 8
    div a0, a0, a1
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 12
    ld a1, 0(sp)
    addi sp, sp, 8
    add a0, a0, a1
    li a0, 66
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 78
    ld a1, 0(sp)
    addi sp, sp, 8
    sub a0, a0, a1
    ret
