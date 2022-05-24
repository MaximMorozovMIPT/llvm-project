//===-- SimMCTargetDesc.h - Sim target descriptions -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCTARGETDESC_H
#define LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {

class Target;

extern Target TheSimTarget;

} // end namespace llvm

// Defines symbolic names for Sim registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "SimGenRegisterInfo.inc"

// Defines symbolic names for the Sim instructions.
#define GET_INSTRINFO_ENUM
#include "SimGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SimGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_SIM_MCTARGETDESC_SIMMCTARGETDESC_H