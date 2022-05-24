add_llvm_component_library(LLVMSimDesc
  SimInstPrinter.cpp
  SimMCAsmInfo.cpp
  SimMCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  SimInfo
  Support

  ADD_TO_COMPONENT
  Sim
  )