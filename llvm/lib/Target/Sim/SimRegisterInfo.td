//===-- SimRegisterInfo.td - Sim Register defs -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Sim register file
//===----------------------------------------------------------------------===//

// For tablegen(... -gen-emitter)  in CMakeLists.txt
class SimReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Sim";
}

// Sim CPU Registers
class SimGPRReg<bits<16> Enc, string n> : SimReg<Enc, n> {
  let Namespace = "Sim";
}

//===----------------------------------------------------------------------===//
//@Registers
//===----------------------------------------------------------------------===//
//@ All registers definition

foreach I = 0-15 in {
  def R#I : SimGPRReg<I, "r"#I>, DwarfRegNum<[I]>;
}

//===----------------------------------------------------------------------===//
//@Register Classes
//===----------------------------------------------------------------------===//

// General purpose registers
def GPR : RegisterClass<"Sim", [i32], 32, (add (sequence "R%u", 0, 15))>;

// The operand is used in selecting DAG nodes to match a register.
def GPROpnd : RegisterOperand<GPR>;