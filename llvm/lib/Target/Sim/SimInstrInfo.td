//===- SimInstrInfo.td - Target Description for Sim Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sim implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

// ---------------------------------------------------------------------------//
// Selection DAG Nodes.
// ---------------------------------------------------------------------------//

// From riscv
// Target-dependent type requirements.
def SDT_SimCall         : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;

def SDT_SimCallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i32>,
                                           SDTCisVT<1, i32> ]>;

def SDT_SimCallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, i32>,
                                         SDTCisVT<1, i32> ]>;

def SDT_SimBrCC : SDTypeProfile<0, 4, [ SDTCisSameAs<0, 1>,
                                        SDTCisVT<2, OtherVT>,
                                        SDTCisVT<3, OtherVT>]>;
                                         

// These are target-independent nodes, but have target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_SimCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_SimCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def SimRet        : SDNode<"SimISD::RET", SDTNone,
                            [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def SimCall       : SDNode<"SimISD::CALL", SDT_SimCall,
                            [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                             SDNPVariadic]>;

def SimBrCC       : SDNode<"SimISD::BR_CC", SDT_SimBrCC,
                            [SDNPHasChain]>;

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "SimInstrFormats.td"

//===----------------------------------------------------------------------===//
// Sim Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

class ImmAsmOperand<string prefix, int width, string suffix> : AsmOperandClass {
  let Name = prefix # "Imm" # width # suffix;
  let DiagnosticType = !strconcat("Invalid", Name);
}

def SIMM16 : Operand<i32>, ImmLeaf<i32, [{return isInt<16>(Imm);}]> {
  let ParserMatchClass = ImmAsmOperand<"S", 16, "">;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<16>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_SIMM16";
  let OperandNamespace = "SimOp";
}

def UIMM16 : Operand<i32>, ImmLeaf<i32, [{return isUInt<16>(Imm);}]> {
  let ParserMatchClass = ImmAsmOperand<"U", 16, "">;
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isUInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_UIMM16";
  let OperandNamespace = "SimOp";
}

def LI16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((uint64_t)N->getZExtValue() & 0xffff,
                                   SDLoc(N), MVT::i32);
}]>;

def HI16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((uint64_t)N->getZExtValue() >> 16, SDLoc(N),
                                   MVT::i32);
}]>;

def i32li16u : Operand<i32>, PatLeaf<(i32 imm), [{
  return ((N->getZExtValue() & 0xFFFFUL) == N->getZExtValue());
}], LI16> {}

def i32hi16 : Operand<i32>, PatLeaf<(i32 imm), [{
  return ((N->getZExtValue() & 0xFFFF0000UL) == N->getZExtValue());
}], HI16> {}

// Addressing modes.
def ADDRrr : ComplexPattern<iPTR, 1, "SelectADDRrr", [], []>;
def ADDRri : ComplexPattern<iPTR, 1, "SelectADDRri", [frameindex], []>;

// from RISCV
// Constrained operand for the shift operations.
def uimm5 : Operand<i32>, ImmLeaf<i32, [{return isUInt<5>(Imm);}]> {
  let ParserMatchClass = ImmAsmOperand<"U", 5, "">;
  let OperandType = "OPERAND_UIMM5";
  let OperandNamespace = "SimOp";
}

def BTARGET16 : Operand<OtherVT> {
  let ParserMatchClass = ImmAsmOperand<"S", 16, "">;
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<16>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_PCREL";
}

def CTARGET16 : Operand<i32> {
  let ParserMatchClass = ImmAsmOperand<"S", 16, "">;
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<16>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_PCREL";
}

// From riscv
def IsOrAdd: PatFrag<(ops node:$A, node:$B), (or node:$A, node:$B), [{
  return isOrEquivalentToAdd(N);
}]>;

class PatGpr<SDPatternOperator OpNode, SimInst Inst>
    : Pat<(OpNode GPR:$rs1), (Inst GPR:$rs1)>;

class PatGprGpr<SDPatternOperator OpNode, SimInst Inst>
    : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;
    
class PatGprImm<SDPatternOperator OpNode, SimInst Inst, ImmLeaf ImmType>
    : Pat<(i32 (OpNode (i32 GPR:$rs1), ImmType:$imm)),
          (Inst GPR:$rs1, ImmType:$imm)>;

class PatBcc<CondCode Cond, SimInst Inst>
    : Pat<(SimBrCC GPR:$rs1, GPR:$rs2, Cond, bb:$imm16),
          (Inst GPR:$rs1, GPR:$rs2, BTARGET16:$imm16)>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

// The formats follow the categories specified in the our heads and agreements.

// Category: Logical, arithmetic

class SimInstRegImm<bits<8> opcode, string asmstr> : SimInst<opcode,
                    (outs GPR:$rd), (ins GPR:$rs1, GPR:$imm16), asmstr # " $r1 $r2 $imm16", []> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
}

class SimInstRegReg<bits<8> opcode, string asmstr> : SimInst<opcode,
                      (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), asmstr # " $r1 $r2 $r3", []> {
  bits<4> rs2;

  let Inst{15-12} = rs2;
  let Inst{11-0}  = 0;

  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
}

// Category: Flow control.

class CondBranch<bits<8> opcode, string asmstr> : SimInst<opcode,
                 (outs), (ins GPR:$rs1, GPR:$rs2, BTARGET16:$r3_imm), asmstr # " $r1 $r2 $r3_imm", []> {

  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let isTerminator = 1;
  let isBranch = 1;
}

//===----------------------------------------------------------------------===//
// Sim Instructions
// Opcodes are used to support compatibility
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Arithmetic, Logic

def ADD : SimInstRegReg<0x21, "ADD">;  def : PatGprGpr<add,  ADD>;
def SUB : SimInstRegReg<0x22, "SUB">;  def : PatGprGpr<sub,  SUB>;
def MUL : SimInstRegReg<0x23, "MUL">;  def : PatGprGpr<mul,  MUL>;
def DIV : SimInstRegReg<0x24, "DIV">;  def : PatGprGpr<sdiv, DIV>;
def OR  : SimInstRegReg<0x41, "OR">;   def : PatGprGpr<or,   OR>;
def AND : SimInstRegReg<0x42, "AND">;  def : PatGprGpr<and,  AND>;
def XOR : SimInstRegReg<0x43, "XOR">;  def : PatGprGpr<xor,  XOR>;

def ADDi : SimInstRegImm<0x31, "ADDi">; def : PatGprImm<add, ADDi, SIMM16>;
def SUBi : SimInstRegImm<0x32, "SUBi">; def : PatGprImm<sub, SUBi, SIMM16>;
def MULi : SimInstRegImm<0x33, "MULi">; def : PatGprImm<mul, MULi, SIMM16>;
def DIVi : SimInstRegImm<0x34, "DIVi">; def : PatGprImm<sdiv, DIVi, SIMM16>;
def ANDi : SimInstRegImm<0x51, "ANDi">; def : PatGprImm<and, ANDi, SIMM16>;
def ORi  : SimInstRegImm<0x52, "ORi">;  def : PatGprImm<or,  ORi,  SIMM16>;
def XORi : SimInstRegImm<0x53, "XORi">; def : PatGprImm<xor, XORi, SIMM16>;

//===----------------------------------------------------------------------===//
// Moves

def MOV   : SimInst<0x10, (outs GPR:$rd), (ins GPR:$rs), "MOV $rd $rs", []> {
  let r3_imm = 0;
  let isAsCheapAsAMove = 1;
  let isCodeGenOnly = 1;
  let isReMaterializable = 1;
}
def MOVli : SimInst<0x11, (outs GPR:$rd), (ins i32li16u:$imm16),
                       "MOVli $rd $imm16", [(set GPR:$rd, i32li16u:$imm16)]> {
  let r2 = 0;
  let isAsCheapAsAMove = 1;
  let isCodeGenOnly = 1;
  let isReMaterializable = 1;
}
def MOVhi : SimInst<0x12, (outs GPR:$rd), (ins i32li16u:$imm16),
                       "MOVhi $rd $imm16", [(set GPR:$rd, i32li16u:$imm16)]> {
  let r2 = 0;
  let isAsCheapAsAMove = 1;
  let isCodeGenOnly = 1;
  let isReMaterializable = 1;
}

//===----------------------------------------------------------------------===//
// Branches

def B : SimInst<0x02, (outs), (ins BTARGET16:$r3_imm), "B $r3_imm", [(br bb:$r3_imm)]> {
  let r1 = 0;
  let r2 = 0;
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let isTerminator = 1;
  let isBranch = 1;
}

def BL : SimInst<0x03, (outs GPR:$rd), (ins CTARGET16:$imm16), "BL $rd $imm16", []> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let isCall = 1;
  let isTerminator = 1;
  let isBranch = 1;
}

def BR : SimInst<0x05, (outs), (ins GPR:$rs1), "BR $rs1", [(brind GPR:$rs1)]> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let isCall = 1;
  let isTerminator = 1;
  let isBranch = 1;
}

// BLR needed for indirect pseudo-call only
def BLR : SimInst<0x0, (outs GPR:$rd), (ins GPR:$rs1), "BLR $rd $rs1", []> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 0;
  let isCall = 1;
  let isTerminator = 1;
  let isBranch = 1;
}

def BEQ  : CondBranch<0x06, "B.EQ">;  def : PatBcc<SETEQ, BEQ>;
def BNE  : CondBranch<0x07, "B.NE">;  def : PatBcc<SETNE, BNE>;
def BGT  : CondBranch<0x08, "B.GT">;  def : PatBcc<SETGT, BGT>;
def BLE  : CondBranch<0x09, "B.LE">;  def : PatBcc<SETLE, BLE>;
def BLEU : CondBranch<0x13, "B.LEU">; def : PatBcc<SETULE, BLEU>;
def BGTU : CondBranch<0x14, "B.GTU">; def : PatBcc<SETUGT, BGTU>;

//===----------------------------------------------------------------------===//
// Support instr

// No operation
def NOP     : SimInst<0, (outs), (ins), "NOP", []>;

def EXIT    : SimInst<0x01, (outs), (ins), "EXIT", []>;

// TODO: NEG is not implemented now, it is generated by "NEG rd r2 == LDi r1 0; SUB rd r1 r2"
def NEG    : SimInst<0x20, (outs GPR:$r1), (ins GPR:$r2), "NEG $r1 $r2", []> {
  let r3_imm = 0;
}

//===----------------------------------------------------------------------===//
// Loads

def LD : SimInst<0x70, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                       "LD $rd $rs1 $rs2",
                       [(set GPR:$rd, (load (add ADDRrr:$rs1, GPR:$rs2)))]> {
  let hasSideEffects = 0;
  let mayLoad = 1;
  let mayStore = 0;
}

def LDi : SimInst<0x72, (outs GPR:$rd), (ins GPR:$rs1, SIMM16:$imm16),
                       "LDi $rd $rs1 $imm16",
                       [(set GPR:$rd, (load (IsOrAdd ADDRri:$rs1, SIMM16:$imm16)))]> {
  let hasSideEffects = 0;
  let mayLoad = 1;
  let mayStore = 0;
}

// TODO: can it be replaced?
// def LDI_: SimInst<0x0, (outs GPR:$rd), (ins GPR:$rs1, SIMM16:$imm16),
//                         "LDi $rd $rs1 $imm16",
//                         []> {
//   let hasSideEffects = 0;
//   let mayLoad = 1;
//   let mayStore = 0;
// }
def : Pat<(i32 (load ADDRrr:$rs1)), (LDi ADDRrr:$rs1, 0)>;

//===----------------------------------------------------------------------===//
// Stores

def ST : SimInst<0x71, (outs), (ins GPR:$rd, GPR:$rs1, GPR:$rs2),
                       "ST $rd $rs1 $rs2",
                       [(store (i32 GPR:$rd), (add ADDRrr:$rs1, GPR:$rs2))]> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 1;
}

def STi : SimInst<0x73, (outs), (ins GPR:$rd, GPR:$rs1, SIMM16:$imm16),
                       "STi $rd $rs1 $imm16",
                       [(store (i32 GPR:$rd), (IsOrAdd ADDRri:$rs1, SIMM16:$imm16))]> {
  let hasSideEffects = 0;
  let mayLoad = 0;
  let mayStore = 1;
}

def : Pat<(store (i32 GPR:$rd), ADDRri:$rs1),
          (STi GPR:$rd, ADDRri:$rs1, 0)>;

//===----------------------------------------------------------------------===//
// Pseudo

// GP == R2
// GP changed to R0 for compatibility with emulator
let Defs = [R0], Uses = [R0] in {
def ADJCALLSTACKDOWN : SimPseudoInst<(outs), (ins i32imm:$amt, i32imm:$amt2),
                               "# ADJCALLSTACKDOWN $amt $amt2",
                               [(callseq_start timm:$amt, timm:$amt2)]>;
def ADJCALLSTACKUP : SimPseudoInst<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            "# ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// RA == R0
// RA changed to R1 for compatibility with emulator
def PseudoRET : SimPseudoInst<(outs), (ins), "# PseudoRET",
                              [(SimRet)]>,
                              PseudoInstExpansion<(BR R1)> {
  let isBarrier = 1;
  let isReturn = 1;
  let isTerminator = 1;
}

let isCall = 1, Defs = [R1] in
def PseudoCALL : SimPseudoInst<(outs), (ins CTARGET16:$func), "# PseudoCALL",
                               [(SimCall tglobaladdr:$func)]>,
                               PseudoInstExpansion<(BL R1, CTARGET16:$func)>;

// TODO: implement it
let isCall = 1, Defs = [R1] in
def PseudoCALLInd : SimPseudoInst<(outs), (ins GPR:$rs1), "# PseudoCALLInd",
                    [(SimCall GPR:$rs1)]>,
                    PseudoInstExpansion<(BLR R1, GPR:$rs1)>;


// i16 imm
def : Pat<(i32 i32li16u:$imm16), (MOVli imm:$imm16)>;
// i32 imm
def : Pat<(i32 imm:$imm32), (ORi (MOVhi (HI16 imm:$imm32)), (LI16 imm:$imm32))>;
