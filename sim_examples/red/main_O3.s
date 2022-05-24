	.text
	.file	"red.c"
	.globl	main
	.type	main,@function
main:
	ADDi r2 r2 -9
	STi r0 r2 8
	STi r1 r2 7
	STi r3 r2 6
	STi r5 r2 5
	STi r6 r2 4
	STi r7 r2 3
	STi r8 r2 2
	ADDi r3 r2 9
	MOVli r9 0
	MOVhi r4 65535
	ORi r4 r4 0
	STi r4 r3 -9
	MOVli r8 256
.LBB0_1:
	STi r9 r3 -8
	MULi r9 r4 257
	LDi r9 r3 -9
	ADD r5 r4 r9
	MOVli r6 0
.LBB0_2:
	MOVli r7 0
.LBB0_3:
	ADDi r7 r9 0
	ADDi r6 r10 0
	ADDi r5 r11 0
	BL r1 simSetPixel
	ADDi r7 r7 1
	B.NE r7 r8 .LBB0_3
	ADDi r6 r6 1
	MOVli r4 128
	B.NE r6 r4 .LBB0_2
	BL r1 simFlush
	LDi r9 r3 -8
	ADDi r9 r9 1
	MOVli r4 255
	B.NE r9 r4 .LBB0_1
	MOVli r9 0
	LDi r8 r2 2
	LDi r7 r2 3
	LDi r6 r2 4
	LDi r5 r2 5
	LDi r3 r2 6
	LDi r1 r2 7
	LDi r0 r2 8
	ADDi r2 r2 2
	BR r1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.ident	"clang version 13.0.1 (git@github.com:MaximMorozovMIPT/llvm-project.git bc4fea7a29b013a2d590958fd259c0e93663a0c2)"
	.section	".note.GNU-stack","",@progbits
