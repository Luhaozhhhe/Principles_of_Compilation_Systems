	.text
	.globl main
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zba1p0_zbb1p0"
add:
.add_0:
	sd			ra,-208(sp)
	fsd			fs11,-200(sp)
	fsd			fs10,-192(sp)
	fsd			fs9,-184(sp)
	fsd			fs8,-176(sp)
	fsd			fs7,-168(sp)
	fsd			fs6,-160(sp)
	fsd			fs5,-152(sp)
	fsd			fs4,-144(sp)
	fsd			fs3,-136(sp)
	fsd			fs2,-128(sp)
	fsd			fs1,-120(sp)
	fsd			fs0,-112(sp)
	sd			s11,-104(sp)
	sd			s10,-96(sp)
	sd			s9,-88(sp)
	sd			s8,-80(sp)
	sd			s7,-72(sp)
	sd			s6,-64(sp)
	sd			s5,-56(sp)
	sd			s4,-48(sp)
	sd			s3,-40(sp)
	sd			s2,-32(sp)
	sd			s1,-24(sp)
	sd			fp,-16(sp)
	sd			sp,-8(sp)
	addi		sp,sp,-224
	add			t0,a1,x0
	add			t1,a0,x0
	sw			t1,4(sp)
	sw			t0,0(sp)
	lw			t0,4(sp)
	lw			t1,0(sp)
	addw		t0,t0,t1
	add			a0,t0,x0
	addi		sp,sp,224
	ld			sp,-8(sp)
	ld			fp,-16(sp)
	ld			s1,-24(sp)
	ld			s2,-32(sp)
	ld			s3,-40(sp)
	ld			s4,-48(sp)
	ld			s5,-56(sp)
	ld			s6,-64(sp)
	ld			s7,-72(sp)
	ld			s8,-80(sp)
	ld			s9,-88(sp)
	ld			s10,-96(sp)
	ld			s11,-104(sp)
	fld			fs0,-112(sp)
	fld			fs1,-120(sp)
	fld			fs2,-128(sp)
	fld			fs3,-136(sp)
	fld			fs4,-144(sp)
	fld			fs5,-152(sp)
	fld			fs6,-160(sp)
	fld			fs7,-168(sp)
	fld			fs8,-176(sp)
	fld			fs9,-184(sp)
	fld			fs10,-192(sp)
	fld			fs11,-200(sp)
	ld			ra,-208(sp)
	jalr		x0,ra,0
main:
.main_0:
	sd			ra,-208(sp)
	fsd			fs11,-200(sp)
	fsd			fs10,-192(sp)
	fsd			fs9,-184(sp)
	fsd			fs8,-176(sp)
	fsd			fs7,-168(sp)
	fsd			fs6,-160(sp)
	fsd			fs5,-152(sp)
	fsd			fs4,-144(sp)
	fsd			fs3,-136(sp)
	fsd			fs2,-128(sp)
	fsd			fs1,-120(sp)
	fsd			fs0,-112(sp)
	sd			s11,-104(sp)
	sd			s10,-96(sp)
	sd			s9,-88(sp)
	sd			s8,-80(sp)
	sd			s7,-72(sp)
	sd			s6,-64(sp)
	sd			s5,-56(sp)
	sd			s4,-48(sp)
	sd			s3,-40(sp)
	sd			s2,-32(sp)
	sd			s1,-24(sp)
	sd			fp,-16(sp)
	sd			sp,-8(sp)
	addi		sp,sp,-208
	li			t0,5
	li			t1,6
	addi		a0,t0,0
	addi		a1,t1,0
	call		add
	add			t0,x0,a0
	add			a0,t0,x0
	addi		sp,sp,208
	ld			sp,-8(sp)
	ld			fp,-16(sp)
	ld			s1,-24(sp)
	ld			s2,-32(sp)
	ld			s3,-40(sp)
	ld			s4,-48(sp)
	ld			s5,-56(sp)
	ld			s6,-64(sp)
	ld			s7,-72(sp)
	ld			s8,-80(sp)
	ld			s9,-88(sp)
	ld			s10,-96(sp)
	ld			s11,-104(sp)
	fld			fs0,-112(sp)
	fld			fs1,-120(sp)
	fld			fs2,-128(sp)
	fld			fs3,-136(sp)
	fld			fs4,-144(sp)
	fld			fs5,-152(sp)
	fld			fs6,-160(sp)
	fld			fs7,-168(sp)
	fld			fs8,-176(sp)
	fld			fs9,-184(sp)
	fld			fs10,-192(sp)
	fld			fs11,-200(sp)
	ld			ra,-208(sp)
	jalr		x0,ra,0
	.data
