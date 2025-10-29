
#include <cassert>
#include <cstdint>

namespace llvm {
struct Module;
struct Function;
struct Constant;

extern void appendToGlobalCtors(Module &M, Function *F, int Priority,
                                Constant *Data = nullptr);
} // namespace llvm

extern "C" void nitrate_llvm_appendToGlobalCtors(llvm::Module *M,
                                                 llvm::Function *F,
                                                 uint32_t Priority) {
  assert(M != nullptr && "Module must not be null");
  assert(F != nullptr && "Function must not be null");

  llvm::appendToGlobalCtors(*M, F, Priority);
}
