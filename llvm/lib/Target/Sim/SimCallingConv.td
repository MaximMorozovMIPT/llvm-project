//===-- SimCallingConv.td - Sim Calling Conventions ------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the Sim architectures.
//
//===----------------------------------------------------------------------===//

// Sim 32-bit ELF C Calling convention.
def CC_Sim : CallingConv<[
  // Promote i8/i16 args to i32.
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,

  // Register R13 is used to pass structure return pointer. Why don't we need it?
  // CCIfSRet<CCIfType<[i32], CCAssignToReg<[R13]>>>,  

  // Registers R9 to R12 are used for passing parameters.
  // 2 first arguments cause regs a 16 bit
  CCIfType<[i32,i64], CCAssignToReg<[R9, R10, R11, R12]>>,

  // Other arguments are passed on the stack, at least 4-byte-aligned.
  // <1,1> because of using 4 byte words
  CCIfType<[i32], CCAssignToStack<1, 1>>,
  CCIfType<[i64], CCAssignToStack<2, 1>>,
]>;

// Sim 32-bit ELF C return-value convention.
def RetCC_Sim : CallingConv<[
  // 32-bit values are returned in R2, 64-bit values in pair R2/R3.
  CCIfType<[i32], CCAssignToReg<[R9, R10]>>,
  CCIfType<[i64], CCAssignToReg<[R9, R10, R11, R12]>>,
  // CCIfType<[i32, i64], CCAssignToReg<[R9, R10, R11, R12]>> Is there any difference?

  // Alternatively, they are assigned to the stack in 4-byte aligned units.
  CCIfType<[i32], CCAssignToStack<1, 1>>,
  CCIfType<[i64], CCAssignToStack<2, 1>>,
]>;

// Sim 32-bit ELF C callee saved registers.
def CSR_Sim : CalleeSavedRegs<(add R0, R3, R5, R6, R7, R8)>;