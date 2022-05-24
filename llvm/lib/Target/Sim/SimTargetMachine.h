//===-- SimTargetMachine.h - Define TargetMachine for Sim -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Sim specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SIM_SIMTARGETMACHINE_H
#define LLVM_LIB_TARGET_SIM_SIMTARGETMACHINE_H

#include "SimSubtarget.h"
#include "SimInstrInfo.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class SimTargetMachine : public LLVMTargetMachine {
private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<SimSubtarget>> SubtargetMap;
  SimSubtarget Subtarget;

public:
  SimTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                    CodeGenOpt::Level OL, bool JIT);
  ~SimTargetMachine() override;

  const SimSubtarget *getSubtargetImpl(const Function &) const override { return &Subtarget; }
  const SimSubtarget *getSubtargetImpl() const { return &Subtarget; }

  // Override LLVMTargetMachine
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  // TargetTransformInfo getTargetTransformInfo(const Function &F) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SIM_SIMTARGETMACHINE_H
