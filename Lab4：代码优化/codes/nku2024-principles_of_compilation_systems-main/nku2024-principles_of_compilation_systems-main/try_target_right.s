	.text
	.attribute	4, 16
	.attribute	5, "rv64i2p0_m2p0_a2p0_c2p0"
	.file	"try_target.out.ll"
	.globl	add
	.p2align	1
	.type	add,@function
add:
	.cfi_startproc
	addi	sp, sp, -16
	.cfi_def_cfa_offset 16
	sw	a0, 8(sp)
	sw	a1, 12(sp)
	lw	a0, 8(sp)
	lw	a1, 12(sp)
	addw	a0, a0, a1
	addi	sp, sp, 16
	ret
.Lfunc_end0:
	.size	add, .Lfunc_end0-add
	.cfi_endproc

	.globl	main
	.p2align	1
	.type	main,@function
main:
	.cfi_startproc
	addi	sp, sp, -16
	.cfi_def_cfa_offset 16
	sd	ra, 8(sp)
	.cfi_offset ra, -8
	li	a0, 5
	li	a1, 6
	call	add@plt
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym add
