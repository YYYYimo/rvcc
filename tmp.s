    .globl main
main:
    addi sp, sp, -8
    sd fp, 0(sp)
    mv fp, sp
    addi sp, sp, -16
    addi a0, fp, -8
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 0
    ld a1, 0(sp)
    addi sp, sp, 8
    sd a0, 0(a1)
.L.begin.1:
    li a0, 10
    addi sp, sp, -8
    sd a0, 0(sp)
    addi a0, fp, -8
    ld a0, 0(a0)
    ld a1, 0(sp)
    addi sp, sp, 8
    slt a0, a0, a1
    beqz a0, .L.end.1
    addi a0, fp, -8
    addi sp, sp, -8
    sd a0, 0(sp)
    li a0, 1
    addi sp, sp, -8
    sd a0, 0(sp)
    addi a0, fp, -8
    ld a0, 0(a0)
    ld a1, 0(sp)
    addi sp, sp, 8
    add a0, a0, a1
    ld a1, 0(sp)
    addi sp, sp, 8
    sd a0, 0(a1)
    j .L.begin.1
.L.end.1:
    addi a0, fp, -8
    ld a0, 0(a0)
    j .L.return
.L.return:
    mv sp, fp
    ld fp, 0(sp)
    addi sp, sp, 8
    ret
