//===-- SimFrameLowering.cpp - Frame lowering for Sim -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Sim.h"
#include "SimFrameLowering.h"
#include "SimInstrInfo.h"
#include "SimMachineFunctionInfo.h"
#include "SimRegisterInfo.h"
#include "SimSubtarget.h"

#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

SimFrameLowering::SimFrameLowering(const SimSubtarget &STI)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(1), 0, Align(1)),
      STI(STI) {}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
bool SimFrameLowering::hasFP(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = STI.getRegisterInfo();

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         RegInfo->hasStackRealignment(MF) || MFI.hasVarSizedObjects() ||
         MFI.isFrameAddressTaken();
}

bool SimFrameLowering::hasBP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  return MFI.hasVarSizedObjects() && TRI->hasStackRealignment(MF);
}

void SimFrameLowering::adjustReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  int64_t Val,
                                  MachineInstr::MIFlag Flag,
                                  Register DestReg,
                                  Register SrcReg ) const {
  // MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
  const SimInstrInfo *TII = STI.getInstrInfo();
  DebugLoc dl;
  if (DestReg == SrcReg && Val == 0)
    return;

  if (isInt<16>(Val)) {
    BuildMI(MBB, MBBI, dl, TII->get(Sim::ADDi), DestReg)
        .addReg(SrcReg)
        .addImm(Val)
        .setMIFlag(Flag);
  } else {
    llvm_unreachable("Invalid Val argument value");
  }
}

void SimFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
    SimMachineFunctionInfo *FuncInfo = MF.getInfo<SimMachineFunctionInfo>();

    MachineFrameInfo &MFI = MF.getFrameInfo();
    const SimRegisterInfo &RegInfo = *static_cast<const SimRegisterInfo *>(STI.getRegisterInfo());
    MachineBasicBlock::iterator MBBI = MBB.begin();
  
    // Debug location must be unknown since the first debug location is used
    // to determine the end of the prologue.
    DebugLoc dl;
  
    bool NeedsStackRealignment = RegInfo.shouldRealignStack(MF);
    if (NeedsStackRealignment && !RegInfo.canRealignStack(MF)) {
      report_fatal_error("Function \"" + Twine(MF.getName()) + "\" required "
                         "stack re-alignment, but LLVM couldn't handle it "
                         "(probably because it has a dynamic alloca).");
    }

    auto StackSize = alignTo(MFI.getStackSize(), getStackAlign());
    StackSize = StackSize / 4; // Byte to word
    // Update stack size with corrected value.
    MFI.setStackSize(StackSize);

    if (!isInt<16>(StackSize)) {
        llvm_unreachable("Stack offs won't fit in USim::LDi");
    }

    // Early exit if there is no need to allocate on the stack
    if (StackSize == 0 && !MFI.adjustsStack()) {
      return;
    }

    // Allocate space on the stack if necessary
    adjustReg(MBB, MBBI, -StackSize, MachineInstr::FrameSetup, Sim::SP, Sim::SP);
    
    const auto &CSI = MFI.getCalleeSavedInfo();

    // The frame pointer is callee-saved, and code has been generated for us to
    // save it to the stack. We need to skip over the storing of callee-saved
    // registers as the frame pointer must be modified after it has been saved
    // to the stack, not before.
    std::advance(MBBI, CSI.size());

    if (!hasFP(MF)) {
      return;
    }

    // FP saved in spillCalleeSavedRegisters

    auto tmp = FuncInfo->getVarArgsSaveSize();
    StackSize -= tmp / 4;
    // Set new FP value
    adjustReg(MBB, MBBI, StackSize, MachineInstr::FrameSetup, Sim::SP, Sim::FP);
}

void SimFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
    const SimRegisterInfo *RI = STI.getRegisterInfo();
    MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
    DebugLoc dl = MBBI->getDebugLoc();

    MachineFrameInfo &MFI = MF.getFrameInfo();

    auto StackSize = alignTo(MFI.getStackSize(), getStackAlign());
    if (StackSize == 0 && !MFI.adjustsStack()) {
      return;
    }

    assert(!(MFI.hasVarSizedObjects() && RI->hasStackRealignment(MF)) && "TBD");

    adjustReg(MBB, MBBI, StackSize / 4, MachineInstr::FrameDestroy, Sim::SP, Sim::SP);

    if (!hasFP(MF)) {
      return;
    }
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions.
MachineBasicBlock::iterator SimFrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  if (!hasReservedCallFrame(MF)) {
    MachineInstr &MI = *I;
    int Size = MI.getOperand(0).getImm();
    if (MI.getOpcode() == Sim::ADJCALLSTACKDOWN) {
      Size = -Size;
    }

    if (Size) {
      adjustReg(MBB, I, Size / 4, MachineInstr::NoFlags, Sim::SP, Sim::SP);
    }
  }
  return MBB.erase(I);
}

// Not preserve stack space within prologue for outgoing variables when the
// function contains variable size objects or there are vector objects accessed
// by the frame pointer.
// Let eliminateCallFramePseudoInstr preserve stack space for it.
bool SimFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
    return !MF.getFrameInfo().hasVarSizedObjects();
}

bool SimFrameLowering::isLeafProc(MachineFunction &MF) const
{
  MachineRegisterInfo &MRI = MF.getRegInfo();
  MachineFrameInfo    &MFI = MF.getFrameInfo();

  // TODO: change R12 to smth else
  return !(MFI.hasCalls()                    // has calls
           || MRI.isPhysRegUsed(Sim::R12)    // Too many registers needed
           || MRI.isPhysRegUsed(Sim::SP)     // %sp is used
           || hasFP(MF));                    // need %fp
}


StackOffset
SimFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                         Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const SimRegisterInfo *RegInfo = STI.getRegisterInfo();
  const SimMachineFunctionInfo *FuncInfo = MF.getInfo<SimMachineFunctionInfo>();

  // TODO: taken from RISCV; need to add VarArgsSaveSize
  int FrameOffset = MFI.getObjectOffset(FI) - getOffsetOfLocalArea() +
                    MFI.getOffsetAdjustment();

  // Addressable stack objects are accessed using neg. offsets from
  // %fp, or positive offsets from %sp.

  const auto &CSI = MFI.getCalleeSavedInfo();
  int MinCSFI = 0;
  int MaxCSFI = -1;
  if (CSI.size()) {
    MinCSFI = CSI[0].getFrameIdx();
    MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
  }

  bool UseFP;
  if (FI >= MinCSFI && FI <= MaxCSFI) {
    UseFP = false;
  } else if (FuncInfo->isLeafProc()) {
    // If there's a leaf proc, all offsets need to be %sp-based,
    // because we haven't caused %fp to actually point to our frame.
    errs() << "FuncInfo->isLeafProc()\n"; 
    UseFP = false;
  } else if (RegInfo->hasStackRealignment(MF)) {
    UseFP = true;
  } else {
    // Finally, default to using %fp.
    UseFP = true;
  }

  FrameOffset = FrameOffset / 4;

  if (UseFP) {
    FrameReg = RegInfo->getFrameRegister(MF);
    return StackOffset::getFixed(FrameOffset);
  } else {
    FrameReg = Sim::SP;
    return StackOffset::getFixed(FrameOffset + MF.getFrameInfo().getStackSize());
  }
}

void SimFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                              BitVector &SavedRegs,
                                              RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);

  // Unconditionally spill RA and FP only if the function uses a frame
  // pointer.
  if (hasFP(MF)) {
    SavedRegs.set(Sim::RA);
    SavedRegs.set(Sim::FP);
  }
  // Mark BP as used if function has dedicated base pointer.
  if (hasBP(MF)) {
    SavedRegs.set(Sim::BP);
  }
}

bool SimFrameLowering::spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                                 MachineBasicBlock::iterator MI,
                                                 ArrayRef<CalleeSavedInfo> CSI,
                                                 const TargetRegisterInfo *TRI) const {
  if (CSI.empty()) {
    return true;
  }

  const TargetInstrInfo &TII = *STI.getInstrInfo();

  for (auto &CS : CSI) {
    // Insert the spill to the stack frame.
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(MBB, MI, Reg, !MBB.isLiveIn(Reg), CS.getFrameIdx(),
                            RC, TRI);
  }

  return true;
}

bool SimFrameLowering::restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                                                   MachineBasicBlock::iterator MI,
                                                   MutableArrayRef<CalleeSavedInfo> CSI,
                                                   const TargetRegisterInfo *TRI) const {
  if (CSI.empty()) {
    return true;
  }

  const TargetInstrInfo &TII = *STI.getInstrInfo();

  // Insert in reverse order.
  // loadRegFromStackSlot can insert multiple instructions.
  for (auto &CS : reverse(CSI)) {
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.loadRegFromStackSlot(MBB, MI, Reg, CS.getFrameIdx(), RC, TRI);
    assert(MI != MBB.begin() && "loadRegFromStackSlot didn't insert any code!");
  }

  return true;
}
