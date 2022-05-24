//===-- SimRegisterInfo.cpp - Sim Register Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sim implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "SimRegisterInfo.h"
#include "Sim.h"
#include "SimMachineFunctionInfo.h"
#include "SimSubtarget.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "SimGenRegisterInfo.inc"

SimRegisterInfo::SimRegisterInfo() : SimGenRegisterInfo(Sim::R1) {}

const MCPhysReg *
SimRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_Sim_SaveList;
}

BitVector SimRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(Sim::R0);
  Reserved.set(Sim::R1);
  Reserved.set(Sim::R2);
  Reserved.set(Sim::R3);
  return Reserved;
}

void SimRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  DebugLoc dl = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  Register FrameReg;

  int Offset = getFrameLowering(MF)->getFrameIndexReference(MF, FrameIndex, FrameReg).getFixed();

  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  assert(isInt<16>(Offset) && "must fit immediate size");

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false, false, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
}

Register SimRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? Sim::FP : Sim::SP;
}

const uint32_t * SimRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const {
  return CSR_Sim_RegMask;
}

const uint32_t * SimRegisterInfo::getRTCallPreservedMask(CallingConv::ID CC) const {
  return CSR_Sim_RegMask;
}