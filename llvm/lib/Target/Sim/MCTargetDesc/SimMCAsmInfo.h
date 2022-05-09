//====-- SimMCAsmInfo.h - Sim asm properties ---------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCASMINFO_H
#define LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class Triple;

class SimMCAsmInfo : public MCAsmInfoELF {
private:
  void anchor() override;
  
public:
  explicit SimMCAsmInfo(const Triple &TT);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCASMINFO_H