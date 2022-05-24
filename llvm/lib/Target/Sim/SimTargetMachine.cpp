//===-- SimTargetMachine.cpp - Define TargetMachine for Sim ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Sim specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#include "SimTargetMachine.h"
#include "Sim.h"
#include "SimTargetObjectFile.h"
#include "TargetInfo/SimTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetOptions.h"


using namespace llvm;

#define DEBUG_TYPE "Sim"

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSimTarget() {
  // Register the target.
  RegisterTargetMachine<SimTargetMachine> X(getTheSimTarget());
}

std::string computeDataLayout() {
  std::string Ret;

  // Little endian.
  Ret += "e-m:e";

  // Data mangling.
  // Ret += DataLayout::getManglingComponent(TT);

  // Pointers are 32 bit.
  Ret += "-p:32:32";

  // Make sure that global data has at least 32 bits of alignment by default.
  Ret += "-i1:8:32-i8:8:32-i16:16:32-i32:32:32";

  // 64-bit integers are aligned only to 32 bits.
  Ret += "-i64:32";

  // all-bit floats are aligned only to 32 bits.
  Ret += "-f32:32:32";
  Ret += "-f64:32";

  // We prefer 32 bits of aligned for all globals; see above.
  Ret += "-a:0:32";

  // Integer registers are 32bits.
  Ret += "-n32";

  return Ret;
}

Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  return RM.getValueOr(Reloc::Static);
}

static CodeModel::Model getEffectiveSimCodeModel() {
  return CodeModel::Small;
}

/// Create an Sim architecture model
SimTargetMachine::SimTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Optional<Reloc::Model> RM,
                                     Optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveSimCodeModel(), OL),
      TLOF(std::make_unique<SimTargetObjectFile>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

SimTargetMachine::~SimTargetMachine() = default;

// SimSubtarget *getSubtargetImpl().... was moved to SimTargetMachine.h 

namespace {
/// Sim Code Generator Pass Configuration Options.
class SimPassConfig : public TargetPassConfig {
public:
  SimPassConfig(SimTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  SimTargetMachine &getSimTargetMachine() const {
    return getTM<SimTargetMachine>();
  }

  bool addInstSelector() override;
};
} // namespace

TargetPassConfig *SimTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SimPassConfig(*this, PM);
}

bool SimPassConfig::addInstSelector() {
  addPass(createSimISelDag(getSimTargetMachine(), getOptLevel()));
  return false;
}
