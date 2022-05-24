//===-- SimMCTargetDesc.cpp - Sim target descriptions -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SimInfo.h"
#include "SimMCTargetDesc.h"
#include "SimInstPrinter.h"
#include "SimMCAsmInfo.h"
#include "../TargetInfo/SimTargetInfo.h"

#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MachineLocation.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "SimGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SimGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SimGenRegisterInfo.inc"

static MCAsmInfo *createSimMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new SimMCAsmInfo(TT);
  MCRegister SP = MRI.getDwarfRegNum(Sim::R2, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createSimMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSimMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSimMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitSimMCRegisterInfo(X, Sim::R1);
  return X;
}

static MCSubtargetInfo *createSimMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createSimMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createSimMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new SimInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSimTargetMC() {
  // Register the MCAsmInfo.
  TargetRegistry::RegisterMCAsmInfo(getTheSimTarget(), createSimMCAsmInfo);//

  // Register the MCCodeEmitter.
  //TargetRegistry::RegisterMCCodeEmitter(getTheSimTarget(), createSimMCCodeEmitter);

  // Register the MCInstrInfo.
  TargetRegistry::RegisterMCInstrInfo(getTheSimTarget(), createSimMCInstrInfo);//

  // Register the MCRegisterInfo.
  TargetRegistry::RegisterMCRegInfo(getTheSimTarget(), createSimMCRegisterInfo);//

  // Register the MCSubtargetInfo.
  TargetRegistry::RegisterMCSubtargetInfo(getTheSimTarget(), createSimMCSubtargetInfo);//

  // Register the MCAsmBackend.
  //TargetRegistry::RegisterMCAsmBackend(getTheSimTarget(), createSimMCAsmBackend);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheSimTarget(), createSimMCInstPrinter);//
}