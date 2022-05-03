//===-- Sim.td - Describe the Sim Target Machine ----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the Sim target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "SimRegisterInfo.td"
include "SimCallingConv.td"
include "SimInstrInfo.td"

//===----------------------------------------------------------------------===//
// Sim processors supported.
//===----------------------------------------------------------------------===//

def : Processor<"Sim", NoItineraries, []>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def SimInstrInfo : InstrInfo;

def Sim : Target {
  let InstructionSet = SimInstrInfo;
  let AllowRegisterRenaming = 1; // Maybe for future compatibility
}