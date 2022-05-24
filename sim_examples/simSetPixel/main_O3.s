	.text
	.file	"simSetPixel.c"
	.globl	main
	.type	main,@function
main:
	ADDi r2 r2 -3
	STi r0 r2 2
	STi r1 r2 1
	STi r3 r2 0
	ADDi r3 r2 3
	MOVhi r4 65535
	ORi r4 r11 0
	MOVli r9 0
	MOVli r10 127
	BL r1 simSetPixel
	BL r1 simFlush
	MOVli r9 0
	LDi r3 r2 0
	LDi r1 r2 1
	LDi r0 r2 2
	BR r1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.ident	"clang version 13.0.1 (git@github.com:MaximMorozovMIPT/llvm-project.git bc4fea7a29b013a2d590958fd259c0e93663a0c2)"
	.section	".note.GNU-stack","",@progbits
