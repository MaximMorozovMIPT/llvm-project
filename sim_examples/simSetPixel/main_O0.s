	.text
	.file	"simSetPixel.c"
	.globl	main
	.type	main,@function
main:
	ADDi r2 r2 -7
	STi r0 r2 6
	STi r1 r2 5
	STi r3 r2 4
	ADDi r3 r2 7
	MOVli r9 0
	STi r9 r3 -5
	STi r9 r3 -4
	MOVhi r4 65535
	ORi r4 r11 0
	STi r11 r3 -6
	MOVli r10 127
	STi r10 r3 -7
	LDi r9 r3 -5
	LDi r10 r3 -7
	LDi r11 r3 -6
	BL r1 simSetPixel
	BL r1 simFlush
	LDi r9 r3 -5
	LDi r3 r2 4
	LDi r1 r2 5
	LDi r0 r2 6
	ADDi r2 r2 1
	BR r1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.ident	"clang version 13.0.1 (git@github.com:MaximMorozovMIPT/llvm-project.git bc4fea7a29b013a2d590958fd259c0e93663a0c2)"
	.section	".note.GNU-stack","",@progbits
