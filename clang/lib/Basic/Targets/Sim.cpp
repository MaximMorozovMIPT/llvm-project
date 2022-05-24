#include "Sim.h"
#include "Targets.h"

using namespace clang;
using namespace clang::targets;

void SimTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  DefineStd(Builder, "sim", Opts);
  Builder.defineMacro("__ELF__");
}