	.text
	.file	"main.c"
	.globl	main
	.type	main,@function
main:
	ADDi r2 r2 -1
	STi r3 r2 0
	ADDi r2 r3 1
	MOVli r9 4950
	LDi r3 r2 0
	BR r1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.ident	"clang version 13.0.1 (git@github.com:MaximMorozovMIPT/llvm-project.git 572ed058aafd62e5fd6b9ef6f56e7e9f7d7ae2b5)"
	.section	".note.GNU-stack","",@progbits
