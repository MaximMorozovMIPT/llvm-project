//===- SimInstPrinter.cpp - Convert Sim MCInst to assembly syntax -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SimInstPrinter.h"
#include "Sim.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"

#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "asm-printer"


#define GET_INSTRUCTION_NAME
#define PRINT_ALIAS_INSTR
#include "SimGenAsmWriter.inc"

void SimInstPrinter::printOperand(const MCInst *MI, int OpNum, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNum);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << MO.getImm();
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

void SimInstPrinter::printCCOperand(const MCInst *MI, int opNum,
                                    const MCSubtargetInfo &STI,
                                    raw_ostream &O) {
  const MCOperand Operand = MI->getOperand(opNum);
  assert(Operand.isImm() && "Operand must be Immediate");

  int CC = Operand.getImm();
  O << CC;
}

void SimInstPrinter::printBranchOperand(const MCInst *MI, uint64_t Address,
                                        unsigned OpNum, raw_ostream &OS) {
  const MCOperand &MO = MI->getOperand(OpNum);
  if (!MO.isImm()) {
    return printOperand(MI, OpNum, OS);
  }

  if (PrintBranchImmAsAddress) {
    uint32_t Target = Address + MO.getImm();
    OS << formatHex(static_cast<uint64_t>(Target));
  } else {
    OS << MO.getImm();
  }
}

void SimInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const
{
  OS << StringRef(getRegisterName(RegNo)).lower();
}

void SimInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                               StringRef Annot, const MCSubtargetInfo &STI,
                               raw_ostream &O) {
  if (!printAliasInstr(MI, Address, O)) {
    printInstruction(MI, Address, O);
  }
  printAnnotation(O, Annot);
}