//===-- SimAsmPrinter.cpp - Sim LLVM assembly writer ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format Sim assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SimInstPrinter.h"
#include "Sim.h"
#include "SimInstrInfo.h"
#include "SimTargetMachine.h"
#include "SimSubtarget.h"
#include "TargetInfo/SimTargetInfo.h"


#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class SimAsmPrinter : public AsmPrinter {
public:
  explicit SimAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), STI(TM.getMCSubtargetInfo()) {}

  StringRef getPassName() const override { return "Sim Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;

  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer, const MachineInstr *MI);

  bool runOnMachineFunction(MachineFunction &MF) override;

  // Used in pseudo lowerings
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const {
    return lowerSimMachineOperandToMCOperand(MO, MCOp, *this);
  }

private:
  const MCSubtargetInfo *STI;
};

} // end of anonymous namespace

// Simple pseudo-instructions have their lowering (with expansion to real
// instructions) auto-generated.
#include "SimGenMCPseudoLowering.inc"


void SimAsmPrinter::emitInstruction(const MachineInstr *MI) {
  // Do any auto-generated pseudo lowerings.
  if (emitPseudoExpansionLowering(*OutStreamer, MI))
    return;

  MCInst TmpInst;
  if (!lowerSimMachineInstrToMCInst(MI, TmpInst, *this))
    EmitToStreamer(*OutStreamer, TmpInst);
}

bool SimAsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  // Set the current MCSubtargetInfo to a copy which has the correct
  // feature bits for the current MachineFunction
  MCSubtargetInfo &NewSTI = OutStreamer->getContext().getSubtargetCopy(*TM.getMCSubtargetInfo());
  NewSTI.setFeatureBits(MF.getSubtarget().getFeatureBits());
  STI = &NewSTI;

  SetupMachineFunction(MF);
  emitFunctionBody();
  return false;
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSimAsmPrinter() {
  RegisterAsmPrinter<SimAsmPrinter> X(getTheSimTarget());
}
