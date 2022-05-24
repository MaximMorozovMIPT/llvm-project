	.text
	.file	"main.c"
	.globl	main
	.type	main,@function
main:
	ADDi r2 r2 -3
	STi r1 r2 2
	STi r3 r2 1
	ADDi r3 r2 3
	MOVli r9 0
	STi r9 r3 -3
	LDi r3 r2 1
	LDi r1 r2 2
	BR r1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.ident	"clang version 13.0.1 (git@github.com:MaximMorozovMIPT/llvm-project.git bc4fea7a29b013a2d590958fd259c0e93663a0c2)"
	.section	".note.GNU-stack","",@progbits
