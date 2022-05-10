//===-- Sim.h - Top-level interface for Sim representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Sim back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SIM_SIM_H
#define LLVM_LIB_TARGET_SIM_SIM_H

#include "MCTargetDesc/SimMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/AsmPrinter.h"

namespace llvm {
  class SimTargetMachine;
  class FunctionPass;
  bool lowerSimMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                    AsmPrinter &AP);
  bool lowerSimMachineOperandToMCOperand(const MachineOperand &MO,
                                        MCOperand &MCOp, const AsmPrinter &AP);

  FunctionPass *createSimISelDag(SimTargetMachine &TM,
                                CodeGenOpt::Level OptLevel);

  namespace Sim {
  enum {
    // changed for compatibility with emulator
    GP = Sim::R0,
    RA = Sim::R1,
    SP = Sim::R2,
    FP = Sim::R3,
    BP = Sim::R4,
  };
  } // end namespace SIM;

  } // end namespace llvm;

#endif  // LLVM_LIB_TARGET_SIM_SIM_H
