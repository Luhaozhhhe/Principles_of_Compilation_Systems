.file "Finbonacci.c"
.option nopic
.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0"
.attribute unaligned_access, 0
.attribute stack_align, 16

.globl a
.globl b

.section .data

.align 2
a:  
    .word 0
b:  
    .word 1

.section .rodata.str1.8,"aMS",@progbits,1
.align 3
.LC0:
.string "%d"
.align 3
.LC1:
.string "%d\n"

.section .text.startup,"ax",@progbits
.align 1
.globl main
.type main, @function

main:
addi sp,sp,-32
sd s0,16(sp)
sd s1,8(sp)
lui s0,%hi(n)
lui a0,%hi(.LC0)
addi a1,s0,%lo(n)
addi a0,a0,%lo(.LC0)
sd ra,24(sp)
call __isoc99_scanf

#output a
lui a0,%hi(.LC1)
addi a0,a0,%lo(.LC1)
lui s2,%hi(a)
addi a1,s2,%lo(a)
lw a1,0(a1)
call printf

#output b
lui a0,%hi(.LC1)
addi a0,a0,%lo(.LC1)
lui s3,%hi(b)
addi a1,s3,%lo(b)
lw a1,0(a1)
call printf

lui s0,%hi(a)
addi s0,s0,%lo(a)
lw s1,0(s0)

lui a1,%hi(b)
addi a1,a1,%lo(b)
lw s2,0(a1)

lui a0,%hi(n)
addi a0,a0,%lo(n)
lw s4,0(a0)
li s0,1

.L1:
    mv s3,s2
    add s2,s2,s1
    lui a0,%hi(.LC1)
    addi a0,a0,%lo(.LC1)
    mv a1,s2
    call printf
    mv s1,s3
    addi s0,s0,1
    blt s0,s4,.L1


ld ra,24(sp)
ld s0,16(sp)
ld s1,8(sp)
li a0,0
addi sp,sp,32
jr ra
.size main, .-main

.globl n
.section .sbss,"aw",@nobits
.align 2
.type n, @object
.size b, 4
n:
  .zero 4

.ident "GCC: () 12.2.0"
.section .note.GNU-stack,"",@progbits
