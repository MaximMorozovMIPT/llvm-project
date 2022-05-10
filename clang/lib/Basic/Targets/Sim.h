#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_SIM_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_SIM_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringSwitch.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY SimTargetInfo : public TargetInfo {
public:
  SimTargetInfo(const llvm::Triple &Triple, const TargetOptions &)//
      : TargetInfo(Triple) {
    resetDataLayout("e-m:e-p:32:32-i1:8:32-i8:8:32-i16:16:32-"
                    "i32:32:32-f32:32:32-i64:32-f64:32-a:0:32-n32");
  }

  void getTargetDefines(const LangOptions &Opts,//
                        MacroBuilder &Builder) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override { return None; }//

  BuiltinVaListKind getBuiltinVaListKind() const override {//
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  const char *getClobbers() const override { return ""; }//

  ArrayRef<const char *> getGCCRegNames() const override {//
    static const char *const GCCRegNames[] = {
        "r0", "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7", "r8",
        "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
    return llvm::makeArrayRef(GCCRegNames);
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {//
    static const TargetInfo::GCCRegAlias GCCRegNames[] = {
        {{"g0"}, "r0"},  {{"g1"}, "r1"},  {{"g2"}, "r2"},  {{"g3"}, "r3"},
        {{"g4"}, "r4"},  {{"g5"}, "r5"},  {{"g6"}, "r6"},  {{"g7"}, "r7"},
        {{"g8"}, "r8"},  {{"g9"}, "r9"},  {{"a0"}, "r10"}, {{"a1"}, "r11"},
        {{"a2"}, "r12"}, {{"a3"}, "r13"}, {{"a4"}, "r14"}, {{"a5"}, "r15"},
    };
    return llvm::makeArrayRef(GCCRegNames);
  }

  bool validateAsmConstraint(const char *&Name,//
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

//   bool hasBitIntType() const override { return true; }

//   bool isCLZForZeroUndef() const override { return false; }
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_SIM_H