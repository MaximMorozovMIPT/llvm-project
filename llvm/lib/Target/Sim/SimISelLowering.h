//===-- SimISelLowering.h - Sim DAG lowering interface --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Sim uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SIM_SIMISELLOWERING_H
#define LLVM_LIB_TARGET_SIM_SIMISELLOWERING_H

#include "Sim.h"
#include "SimInstrInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SimSubtarget;
class SimTargetMachine;

namespace SimISD {
enum NodeType : unsigned {
    FIRST_NUMBER = ISD::BUILTIN_OP_END,
    RET,
    CALL,
    BR_CC,
};
} // end namespace SimISD

class SimTargetLowering : public TargetLowering {
private:
    const SimSubtarget &Subtarget;

public:
    SimSubtarget const &getSubtarget() const { return Subtarget; }
    explicit SimTargetLowering(const TargetMachine &TM, const SimSubtarget &STI); //supp

    // Override TargetLowering methods.
    const char *getTargetNodeName(unsigned Opcode) const override; //supp

    // Return true if the addressing mode represented by AM is legal for this
    // target, for a load/store of the specified type.
    // riscv copy
    bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM, Type *Ty, //supp
                             unsigned AS,
                             Instruction *I = nullptr) const override;
 
    // riscv copy
    Register getRegisterByName(const char* RegName, LLT VT, const MachineFunction &MF) const override; //supp
    // riscv copy
    EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context, EVT VT) const override; //supp

    SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override; //supp

    SDValue PerformDAGCombine(SDNode *Node, DAGCombinerInfo &DCI) const override; //supp

    // Override required hooks.
    SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, //supp
                                bool IsVarArg,
                                const SmallVectorImpl<ISD::InputArg> &Ins,
                                const SDLoc &DL, SelectionDAG &DAG,
                                SmallVectorImpl<SDValue> &InVals) const override;

    SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, //supp
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                        SelectionDAG &DAG) const override;
    // riscv copy
    bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF, //supp
                      bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context) const override;

    SDValue LowerCall(CallLoweringInfo &CLI, SmallVectorImpl<SDValue> &InVals) const override; //supp

    // riscv copy
    bool mayBeEmittedAsTailCall(const CallInst *CI) const override { //supp
      return false;
    } 
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SIM_SIMISELLOWERING_H