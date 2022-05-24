//===-- SimFrameLowering.h - Frame lowering for Sim -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SIM_SIMFRAMELOWERING_H
#define LLVM_LIB_TARGET_SIM_SIMFRAMELOWERING_H

#include "Sim.h"
#include "llvm/ADT/IndexedMap.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/TypeSize.h"

namespace llvm {

class SimTargetMachine;
class SimSubtarget;
class MachineFunction;


class SimFrameLowering : public TargetFrameLowering {
public:
  explicit SimFrameLowering(const SimSubtarget &STI); //

  // Override TargetFrameLowering.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;//
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override; //

  bool hasFP(const MachineFunction &MF) const override;//
  bool hasBP(const MachineFunction &MF) const;//

  MachineBasicBlock::iterator eliminateCallFramePseudoInstr(MachineFunction &MF, //
                                                            MachineBasicBlock &MBB,
                                                            MachineBasicBlock::iterator I) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;//

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,//
                                     Register &FrameReg) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,//
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool restoreCalleeSavedRegisters(MachineBasicBlock &MBB,//
                                   MachineBasicBlock::iterator MI,
                                   MutableArrayRef<CalleeSavedInfo> CSI,
                                   const TargetRegisterInfo *TRI) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,//
                            RegScavenger *RS = nullptr) const override;
private:
  void adjustReg(MachineBasicBlock &MBB,
                  MachineBasicBlock::iterator MBBI,
                  int64_t Val,
                  MachineInstr::MIFlag Flag,
                  Register DestReg,
                  Register SrcReg ) const;

  bool isLeafProc(MachineFunction &MF) const;//

  const SimSubtarget &STI;
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_SIM_SIMFRAMELOWERING_H