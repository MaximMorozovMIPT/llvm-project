//===-- SimISelLowering.cpp - Sim DAG lowering implementation -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SimTargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "SimISelLowering.h"
#include "SimMachineFunctionInfo.h"
#include "SimRegisterInfo.h"
#include "SimTargetMachine.h"
#include "SimTargetObjectFile.h"
#include "MCTargetDesc/SimInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include <cctype>

using namespace llvm;

#define DEBUG_TYPE "Sim-lower"

#include "SimGenCallingConv.inc"
#include "SimGenRegisterInfo.inc"

//===----------------------------------------------------------------------===//
// TargetLowering Implementation
//===----------------------------------------------------------------------===//

SimTargetLowering::SimTargetLowering(const TargetMachine &TM,
                                       const SimSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  addRegisterClass(MVT::i32, &Sim::GPRRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties(Subtarget.getRegisterInfo());

  // Set up special registers.
  setStackPointerRegisterToSaveRestore(Sim::R2);

  // setOperationAction(ISD::CTLZ, MVT::i32, Custom);
  for (unsigned Opc = 0; Opc < ISD::BUILTIN_OP_END; ++Opc) {
    setOperationAction(Opc, MVT::i32, Expand);
  }

  // Special DAG combiner for bit-field operations.
  setOperationAction(ISD::ADD, MVT::i32, Legal);
  setOperationAction(ISD::SUB, MVT::i32, Legal);
  setOperationAction(ISD::MUL, MVT::i32, Legal);
  setOperationAction(ISD::SDIV, MVT::i32, Legal);

  setOperationAction(ISD::AND, MVT::i32, Legal);
  setOperationAction(ISD::OR, MVT::i32, Legal);
  setOperationAction(ISD::XOR, MVT::i32, Legal);

  setOperationAction(ISD::LOAD, MVT::i32, Legal);
  setOperationAction(ISD::STORE, MVT::i32, Legal);

  setOperationAction(ISD::Constant, MVT::i32, Legal);
  setOperationAction(ISD::UNDEF, MVT::i32, Legal);

  setOperationAction(ISD::BR_CC, MVT::i32, Custom);

  setOperationAction(ISD::FRAMEADDR, MVT::i32, Legal);

  setOperationAction(ISD::SELECT, MVT::i32, Expand);
}

const char *SimTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((SimISD::NodeType)Opcode) {
  case SimISD::FIRST_NUMBER:    break;
  case SimISD::RET:             return "SimISD::RET";
  case SimISD::CALL:            return "SimISD::CALL";
  case SimISD::BR_CC:           return "SimISD::BR_CC";
  }
  return nullptr;
}

// riscv copy
bool SimTargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                              const AddrMode &AM, Type *Ty,
                                              unsigned AS,
                                              Instruction *I) const {
  // No global is ever allowed as a base.
  if (AM.BaseGV)
    return false;

  if (!isInt<16>(AM.BaseOffs))
    return false;

  switch (AM.Scale) {
  case 0: // "r+i" or just "i", depending on HasBaseReg.
    break;
  case 1:
    if (!AM.HasBaseReg) // allow "r+i".
      break;
    return false; // disallow "r+r" or "r+r+i".
  default:
    return false;
  }

  return true;
}

static void translateSetCCForBranch(const SDLoc &DL, SDValue &LHS, SDValue &RHS,
                                    ISD::CondCode &CC, SelectionDAG &DAG) {
  switch (CC) {
  default:
    break;
  case ISD::SETLT:
  case ISD::SETGE:
    CC = ISD::getSetCCSwappedOperands(CC);
    std::swap(LHS, RHS);
    break;
  }
}

SDValue lowerBR_CC(SDValue Op, SelectionDAG &DAG) {
  SDValue CC = Op.getOperand(1);
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Block = Op.getOperand(4);
  SDLoc DL(Op);

  assert(LHS.getValueType() == MVT::i32);

  ISD::CondCode CCVal = cast<CondCodeSDNode>(CC)->get();
  translateSetCCForBranch(DL, LHS, RHS, CCVal, DAG);
  SDValue TargetCC = DAG.getCondCode(CCVal);

  return DAG.getNode(SimISD::BR_CC, DL, Op.getValueType(), Op.getOperand(0), LHS, RHS, TargetCC, Block);
}

SDValue lowerFRAMEADDR(SDValue Op, SelectionDAG &DAG, const SimSubtarget *Subtarget) {
  const SimRegisterInfo &RI = *Subtarget->getRegisterInfo();
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  Register FrameReg = RI.getFrameRegister(MF);
  EVT VT = Op.getValueType();
  SDLoc DL(Op);
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), DL, FrameReg, VT);
  // Only for current frame
  assert(cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue() == 0);
  return FrameAddr;
}

SDValue SimTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
    default: llvm_unreachable("Should not custom lower this!");
    case ISD::FRAMEADDR:          return lowerFRAMEADDR(Op, DAG, &Subtarget);
    case ISD::BR_CC:              return lowerBR_CC(Op, DAG);
  }
}

Register SimTargetLowering::getRegisterByName(const char* RegName, LLT VT,
                                                const MachineFunction &MF) const {
  Register Reg = StringSwitch<Register>(RegName)
    .Case("r0", Sim::R0).Case("r1", Sim::R1).Case("r2", Sim::R2).Case("r3", Sim::R3)
    .Case("r4", Sim::R4).Case("r5", Sim::R5).Case("r6", Sim::R6).Case("r7", Sim::R7)
    .Case("r8", Sim::R8).Case("r9", Sim::R9).Case("r10", Sim::R10).Case("r11", Sim::R11)
    .Case("r12", Sim::R12).Case("r13", Sim::R13).Case("r14", Sim::R14).Case("r15", Sim::R15)
    .Default(0);

  if (Reg) {
    return Reg;
  }

  report_fatal_error("Invalid register name global variable");
}

EVT SimTargetLowering::getSetCCResultType(const DataLayout &, LLVMContext &,
                                          EVT VT) const {
  assert(!VT.isVector() && "can't support vector type");
  return MVT::i32;
}

SDValue SimTargetLowering::PerformDAGCombine(SDNode *Node, DAGCombinerInfo &DCI) const {
  return SDValue();
}

//===----------------------------------------------------------------------===//
// Calling conventions
//===----------------------------------------------------------------------===//

// Convert Val to a ValVT. Should not be called for CCValAssign::Indirect
// values.
static SDValue convertLocVTToValVT(SelectionDAG &DAG, SDValue Val,
                                   const CCValAssign &VA, const SDLoc &DL,
                                   const SimSubtarget &Subtarget) {
  switch (VA.getLocInfo()) {
  default:
    llvm_unreachable("Unexpected CCValAssign::LocInfo");
  case CCValAssign::Full:
    break;
  case CCValAssign::BCvt:
    Val = DAG.getNode(ISD::BITCAST, DL, VA.getValVT(), Val);
    break;
  }
  return Val;
}

// The caller is responsible for loading the full value if the argument is
// passed with CCValAssign::Indirect.
// riscv copy
static SDValue unpackFromRegLoc(SelectionDAG &DAG, SDValue Chain,
                                const CCValAssign &VA, const SDLoc &DL,
                                const SimTargetLowering &TLI) {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  EVT LocVT = VA.getLocVT();
  SDValue Val;
  const TargetRegisterClass *RC = TLI.getRegClassFor(LocVT.getSimpleVT());
  Register VReg = RegInfo.createVirtualRegister(RC);
  RegInfo.addLiveIn(VA.getLocReg(), VReg);
  Val = DAG.getCopyFromReg(Chain, DL, VReg, LocVT);

  if (VA.getLocInfo() == CCValAssign::Indirect)
    return Val;

  return convertLocVTToValVT(DAG, Val, VA, DL, TLI.getSubtarget());
}

// The caller is responsible for loading the full value if the argument is
// passed with CCValAssign::Indirect.
// riscv copy
static SDValue unpackFromMemLoc(SelectionDAG &DAG, SDValue Chain,
                                const CCValAssign &VA, const SDLoc &DL) {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  EVT LocVT = VA.getLocVT();
  EVT ValVT = VA.getValVT();
  EVT PtrVT = MVT::getIntegerVT(DAG.getDataLayout().getPointerSizeInBits(0));
  int FI = MFI.CreateFixedObject(ValVT.getStoreSize(), VA.getLocMemOffset(),
                                 /*IsImmutable=*/true);
  SDValue FIN = DAG.getFrameIndex(FI, PtrVT);
  SDValue Val;

  ISD::LoadExtType ExtType;
  switch (VA.getLocInfo()) {
  default:
    llvm_unreachable("Unexpected CCValAssign::LocInfo");
  case CCValAssign::Full:
  case CCValAssign::Indirect:
  case CCValAssign::BCvt:
    ExtType = ISD::NON_EXTLOAD;
    break;
  }
  Val = DAG.getExtLoad(
      ExtType, DL, LocVT, Chain, FIN,
      MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI), ValVT);
  return Val;
}

// riscv copy
SDValue SimTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  // EVT PtrVT = getPointerTy(DAG.getDataLayout());
  // unsigned StackSlotSize = MVT(MVT::i32).getSizeInBits() / 8;
  // Used with vargs to acumulate store chains.
  std::vector<SDValue> OutChains;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_Sim);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue ArgValue;
    if (VA.isRegLoc())
      ArgValue = unpackFromRegLoc(DAG, Chain, VA, DL, *this);
    else
      ArgValue = unpackFromMemLoc(DAG, Chain, VA, DL);

    if (VA.getLocInfo() == CCValAssign::Indirect) {
        llvm_unreachable("unsupported Indirect calls");

    }
    InVals.push_back(ArgValue);
  }

  if (IsVarArg) {
    llvm_unreachable("LowerFormalArguments for isVarArg???");
  }

  // All stores are grouped in one node to allow the matching between
  // the size of Ins and InVals. This only happens for vararg functions.
  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
  }

  return Chain;
}

static SDValue convertValVTToLocVT(SelectionDAG &DAG, SDValue Val,
                                   const CCValAssign &VA, const SDLoc &DL) {
  EVT LocVT = VA.getLocVT();

  if (VA.getValVT() == MVT::f32)
    llvm_unreachable("");

  switch (VA.getLocInfo()) {
  default:
    llvm_unreachable("Unexpected LocInfo");
  case CCValAssign::Full:
    break;
  case CCValAssign::SExt:
    Val = DAG.getNode(ISD::SIGN_EXTEND, DL, LocVT, Val);
    break;
  case CCValAssign::ZExt:
    Val = DAG.getNode(ISD::ZERO_EXTEND, DL, LocVT, Val);
    break;
  case CCValAssign::BCvt:
    Val = DAG.getNode(ISD::BITCAST, DL, LocVT, Val);
    break;
  }
  return Val;
}

SDValue SimTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool IsVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &DL, SelectionDAG &DAG) const {

  MachineFunction &MF = DAG.getMachineFunction();

  // Assign locations to each returned value.
  SmallVector<CCValAssign, 16> RetLocs;
  CCState RetCCInfo(CallConv, IsVarArg, MF, RetLocs, *DAG.getContext());
  RetCCInfo.AnalyzeReturn(Outs, RetCC_Sim);

  // Quick exit for void returns
  if (RetLocs.empty())
    return DAG.getNode(SimISD::RET, DL, MVT::Other, Chain);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps;
  RetOps.push_back(Chain);
  for (unsigned I = 0, E = RetLocs.size(); I != E; ++I) {
    CCValAssign &VA = RetLocs[I];

    // Make the return register live on exit.
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue Arg = convertValVTToLocVT(DAG, OutVals[I], VA, DL);
    
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Arg, Glue);

    // Guarantee that all emitted copies are stuck together with flags.
    // TODO: this action is redundant for Simulation model - we don't need Glue
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
    
  }

  // Update chain and glue.
  RetOps[0] = Chain;
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(SimISD::RET, DL, MVT::Other, RetOps);
}

SDValue SimTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                             SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &dl = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  assert(!CLI.IsTailCall);
  bool &isTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_Sim);

  // Get the size of the outgoing arguments stack space requirement.
  unsigned ArgsSize = CCInfo.getNextStackOffset();

  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();

  // taken from RISCV
  // Create local copies for byval args.
  SmallVector<SDValue, 8> ByValArgs;
  for (unsigned i = 0, e = Outs.size(); i != e; ++i) {
    ISD::ArgFlagsTy Flags = Outs[i].Flags;
    if (!Flags.isByVal()) {
      continue;
    }

    SDValue Arg = OutVals[i];
    unsigned Size = Flags.getByValSize();
    Align Alignment = Flags.getNonZeroByValAlign();

    if (Size > 0U) {
      int FI = MFI.CreateStackObject(Size, Alignment, false);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue SizeNode = DAG.getConstant(Size, dl, MVT::i32);

      Chain = DAG.getMemcpy(Chain, dl, FIPtr, Arg, SizeNode, Alignment,
                            false,             // isVolatile,
                            (Size <= 32),      // AlwaysInline if size <= 32,
                            isTailCall,        // isTailCall
                            MachinePointerInfo(), MachinePointerInfo());
      ByValArgs.push_back(FIPtr);
    } else {
      SDValue nullVal;
      ByValArgs.push_back(nullVal);
    }
  }

  Chain = DAG.getCALLSEQ_START(Chain, ArgsSize, 0, dl);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  // SDValue StackPtr;
  for (unsigned i = 0, j = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue ArgValue = OutVals[i];
    ISD::ArgFlagsTy Flags = Outs[i].Flags;

    if (VA.needsCustom()) {
      llvm_unreachable("TBD - needsCustom LowerCall");
    }

    // Promote the value if needed.
    // For now, only handle fully promoted and indirect arguments.
    if (VA.getLocInfo() == CCValAssign::Indirect) {
      llvm_unreachable("unsupported Indirect calls");
    } else {
      ArgValue = convertValVTToLocVT(DAG, ArgValue, VA, dl);
    }

    // Use local copy if it is a byval arg.
    if (Flags.isByVal()) {
      ArgValue = ByValArgs[j++];
    }

    if (VA.isRegLoc()) {
      // Queue up the argument copies and emit them at the end.
      if (VA.getLocVT() != MVT::f32) {
        RegsToPass.push_back(std::make_pair(VA.getLocReg(), ArgValue));
      }

      // TODO: implement without bitcast
      ArgValue = DAG.getNode(ISD::BITCAST, dl, MVT::i32, ArgValue);
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), ArgValue));
    } else {
      assert(VA.isMemLoc() && "Argument not register or memory");
      assert(!isTailCall && "Tail call not allowed if stack is used "
                            "for passing parameters");

      // Create a store off the stack pointer for this argument.
      SDValue StackPtr = DAG.getRegister(Sim::SP, MVT::i32);
      SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), dl);
      PtrOff = DAG.getNode(ISD::ADD, dl, MVT::i32, StackPtr, PtrOff);
      MemOpChains.push_back(
          DAG.getStore(Chain, dl, ArgValue, PtrOff, MachinePointerInfo()));
    }
  }


  // Join the stores, which are independent of one another.
  // Make sure the occur before any copies into physregs.
  if (!MemOpChains.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);
  }

  SDValue Glue;
  // Build a sequence of copy-to-reg nodes, chained and glued together.
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, dl, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i32, 0);
  } else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);
  }

  // Returns a chain & a flag for retval copy to use
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));
  }

  // Add a register mask operand representing the call-preserved registers.
  const SimRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask =
      TRI->getRTCallPreservedMask(CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (Glue.getNode()) {
    Ops.push_back(Glue);
  }

  Chain = DAG.getNode(SimISD::CALL, dl, NodeTys, Ops);
  DAG.addNoMergeSiteInfo(Chain.getNode(), CLI.NoMerge);
  Glue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(ArgsSize, dl, true),
                             DAG.getIntPtrConstant(0, dl, true), Glue, dl);
  Glue = Chain.getValue(1);

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RVInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  RVInfo.AnalyzeCallResult(Ins, RetCC_Sim);

  // Copy all of the result registers out of their specified physreg.
  for (auto &VA : RVLocs) {
    // Copy the value out
    SDValue RetValue = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getLocVT(), Glue);
    // Glue the RetValue to the end of the call sequence
    Chain = RetValue.getValue(1);
    Glue = RetValue.getValue(2);

    if (VA.getLocVT() == MVT::i32 && VA.getValVT() == MVT::f64) {
      llvm_unreachable("");
    }

    RetValue = convertValVTToLocVT(DAG, RetValue, VA, dl);
    InVals.push_back(RetValue);
  }

  return Chain;
}

bool SimTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, Context);
  if (!CCInfo.CheckReturn(Outs, RetCC_Sim))
    return false;
  if (CCInfo.getNextStackOffset() != 0 && IsVarArg)
    llvm_unreachable("CanLowerReturn what?"); 
  return true;
}
