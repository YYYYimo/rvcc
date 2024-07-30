    .globl main
main:
    addi sp, sp, -8
    sd fp, 0(sp)
    mv fp, sp
    addi sp, sp, -208
    li a0, 3
    addi sp, sp, -8
    sd a0, 0(sp)
    addi sp, sp, -8
    sd a0, 0(sp)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -8
    ld a0, 0(a0)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -8
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 3
    ld a1, 0(sp)
    addi sp, sp, 8
    sd a0, 0(a1)
    li a0, 4
    addi sp, sp, -8
    sd a0, 0(sp)
    addi sp, sp, -8
    sd a0, 0(sp)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -16
    ld a0, 0(a0)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -16
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 4
    ld a1, 0(sp)
    addi sp, sp, 8
    sd a0, 0(a1)
    li a0, 1
    addi sp, sp, -8
    sd a0, 0(sp)
    addi sp, sp, -8
    sd a0, 0(sp)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -8
    ld a0, 0(a0)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -8
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 1
    ld a1, 0(sp)
    addi sp, sp, 8
    sd a0, 0(a1)
    addi sp, sp, -8
    sd a0, 0(sp)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -16
    ld a0, 0(a0)
    addi sp, sp, -8
    sd a0, 0(sp)
    addi sp, sp, -8
    sd a0, 0(sp)
    ld a1, 0(sp)
    addi sp, sp, 8
    addi a0, fp, -8
    ld a0, 0(a0)
    ld a1, 0(sp)
    addi sp, sp, 8
    add a0, a0, a1
    mv sp, fp
    ld fp, 0(sp)
    addi sp, sp, 8
    ret
