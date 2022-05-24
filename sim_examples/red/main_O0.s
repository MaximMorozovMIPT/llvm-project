	.text
	.file	"red.c"
	.globl	main
	.type	main,@function
main:
	ADDi r2 r2 -10
	STi r0 r2 9
	STi r1 r2 8
	STi r3 r2 7
	ADDi r3 r2 10
	MOVli r4 0
	STi r4 r3 -4
	STi r4 r3 -5
	STi r4 r3 -6
	STi r4 r3 -7
	STi r4 r3 -5
	B .LBB0_1
.LBB0_1:
	LDi r4 r3 -5
	MOVli r9 254
	B.GT r4 r9 .LBB0_12
	B .LBB0_2
.LBB0_2:
	MOVli r4 0
	STi r4 r3 -7
	B .LBB0_3
.LBB0_3:
	LDi r4 r3 -7
	MOVli r9 127
	B.GT r4 r9 .LBB0_10
	B .LBB0_4
.LBB0_4:
	MOVli r4 0
	STi r4 r3 -6
	B .LBB0_5
.LBB0_5:
	LDi r4 r3 -6
	MOVli r9 255
	B.GT r4 r9 .LBB0_8
	B .LBB0_6
.LBB0_6:
	LDi r9 r3 -6
	STi r9 r3 -10
	LDi r10 r3 -7
	STi r10 r3 -9
	LDi r4 r3 -5
	MULi r4 r4 257
	MOVhi r11 65535
	ORi r11 r11 0
	ADD r11 r4 r11
	STi r11 r3 -8
	LDi r9 r3 -10
	LDi r10 r3 -9
	LDi r11 r3 -8
	BL r1 simSetPixel
	B .LBB0_7
.LBB0_7:
	LDi r4 r3 -6
	ADDi r4 r4 1
	STi r4 r3 -6
	B .LBB0_5
.LBB0_8:
	B .LBB0_9
.LBB0_9:
	LDi r4 r3 -7
	ADDi r4 r4 1
	STi r4 r3 -7
	B .LBB0_3
.LBB0_10:
	BL r1 simFlush
	B .LBB0_11
.LBB0_11:
	LDi r4 r3 -5
	ADDi r4 r4 1
	STi r4 r3 -5
	B .LBB0_1
.LBB0_12:
	MOVli r9 0
	LDi r3 r2 7
	LDi r1 r2 8
	LDi r0 r2 9
	ADDi r2 r2 2
	BR r1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.ident	"clang version 13.0.1 (git@github.com:MaximMorozovMIPT/llvm-project.git bc4fea7a29b013a2d590958fd259c0e93663a0c2)"
	.section	".note.GNU-stack","",@progbits
