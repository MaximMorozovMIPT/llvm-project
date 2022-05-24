//===- SimMachineFunctionInfo.h - Sim Machine Function Info -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares  Sim specific per-machine-function information.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_SIM_SIMMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_SIM_SIMMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

//SparcMachineFunctionInfo.h
class SimMachineFunctionInfo : public MachineFunctionInfo {
    virtual void anchor();
private:
    Register GlobalBaseReg;

    /// VarArgsFrameOffset - Frame offset to start of varargs area.
    int VarArgsFrameOffset;

    /// Size of the save area used for varargs
    int VarArgsSaveSize = 0;

    /// IsLeafProc - True if the function is a leaf procedure.
    bool IsLeafProc;
public:
    SimMachineFunctionInfo()
      : GlobalBaseReg(0), VarArgsFrameOffset(0), VarArgsSaveSize(0),
        IsLeafProc(false) {}
    explicit SimMachineFunctionInfo(MachineFunction &MF)
      : GlobalBaseReg(0), VarArgsFrameOffset(0), VarArgsSaveSize(0),
        IsLeafProc(false) {}

    Register getGlobalBaseReg() const { return GlobalBaseReg; }
    void setGlobalBaseReg(Register Reg) { GlobalBaseReg = Reg; }

    int getVarArgsFrameOffset() const { return VarArgsFrameOffset; }
    void setVarArgsFrameOffset(int Offset) { VarArgsFrameOffset = Offset; }

    void setVarArgsSaveSize(int Size) { VarArgsSaveSize = Size; }
    int getVarArgsSaveSize() const { return VarArgsSaveSize; };

    void setLeafProc(bool rhs) { IsLeafProc = rhs; }
    bool isLeafProc() const { return IsLeafProc; }
  };
}

#endif // LLVM_LIB_TARGET_SIM_SIMMACHINEFUNCTIONINFO_H
