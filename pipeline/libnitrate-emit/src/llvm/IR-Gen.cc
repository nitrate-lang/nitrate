////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <llvm-18/llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm-18/llvm/ExecutionEngine/MCJIT.h>
#include <llvm-18/llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm-18/llvm/IR/BasicBlock.h>
#include <llvm-18/llvm/IR/Constant.h>
#include <llvm-18/llvm/IR/DataLayout.h>
#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm-18/llvm/IR/Function.h>
#include <llvm-18/llvm/IR/GlobalValue.h>
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <llvm-18/llvm/IR/IRBuilder.h>
#include <llvm-18/llvm/IR/Instructions.h>
#include <llvm-18/llvm/IR/LLVMContext.h>
#include <llvm-18/llvm/IR/LegacyPassManager.h>
#include <llvm-18/llvm/IR/PassManager.h>
#include <llvm-18/llvm/IR/Type.h>
#include <llvm-18/llvm/IR/Value.h>
#include <llvm-18/llvm/IR/Verifier.h>
#include <llvm-18/llvm/InitializePasses.h>
#include <llvm-18/llvm/MC/TargetRegistry.h>
#include <llvm-18/llvm/Passes/PassBuilder.h>
#include <llvm-18/llvm/Support/CodeGen.h>
#include <llvm-18/llvm/Support/ManagedStatic.h>
#include <llvm-18/llvm/Support/MemoryBuffer.h>
#include <llvm-18/llvm/Support/TargetSelect.h>
#include <llvm-18/llvm/Support/raw_os_ostream.h>
#include <llvm-18/llvm/Support/raw_ostream.h>
#include <llvm-18/llvm/Target/TargetMachine.h>
#include <llvm-18/llvm/Target/TargetOptions.h>
#include <llvm-18/llvm/TargetParser/Host.h>
#include <llvm-18/llvm/Transforms/IPO.h>
#include <llvm-18/llvm/Transforms/InstCombine/InstCombine.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Config.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/Module.hh>
#include <optional>
#include <stack>
#include <streambuf>
#include <unordered_map>

/// TODO: Find way to remove the 's.branch_early' edge case

using namespace llvm;
using namespace std;
using namespace ncc::ir;
using namespace ncc;

// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// #if defined(QCORE_DEBUG)
// #define debug(...) \
//   cerr << "[debug]: ln " << __LINE__ << ": " << __VA_ARGS__ << endl
// #else
// #define debug(...)
// #endif

typedef function<bool(IRModule *, qcode_conf_t *, ostream &err,
                      raw_pwrite_stream &out)>
    qcode_adapter_fn;

// using ctx_t = Module;
// using craft_t = IRBuilder<>;
// using val_t = optional<Value *>;
// using ty_t = optional<llvm::Type *>;

// enum class PtrClass {
//   DataPtr,
//   Function,
// };

// class FunctionReturnBlock {
//   AllocaInst *m_value;
//   BasicBlock *m_block;

// public:
//   FunctionReturnBlock(AllocaInst *value, BasicBlock *block)
//       : m_value(value), m_block(block) {}

//   AllocaInst *getValue() const { return m_value; }
//   BasicBlock *getBlock() const { return m_block; }
// };

// class FunctionStackFrame {
//   Function *m_fn;
//   unordered_map<std::string_view, AllocaInst *> m_locals;

// public:
//   FunctionStackFrame(Function *fn) : m_fn(fn) {}

//   Function *getFunction() const { return m_fn; }

//   void addVariable(std::string_view name, AllocaInst *inst) {
//     m_locals[name] = inst;
//   }

//   const auto &getLocalVariables() const { return m_locals; }
// };

// class State {
//   stack<FunctionReturnBlock> m_return_block_stack;
//   stack<GlobalValue::LinkageTypes> m_current_linkage;
//   stack<BasicBlock *> m_break_stack;
//   stack<BasicBlock *> m_skip_stack;
//   stack<FunctionStackFrame> m_stackframe;
//   bool m_inside_fn;

// public:
//   bool branch_early;

// public:
//   State() {
//     m_current_linkage.push(GlobalValue::LinkageTypes::PrivateLinkage);
//     m_inside_fn = false;
//     branch_early = false;
//   }

//   optional<pair<Value *, PtrClass>> find_named_value(
//       ctx_t &m, std::string_view name) const {
//     if (is_inside_function()) {
//       for (let[cur_name, inst] : m_stackframe.top().getLocalVariables()) {
//         if (cur_name == name) {
//           return {{inst, PtrClass::DataPtr}};
//         }
//       }
//     }

//     if (GlobalVariable *global = m.getGlobalVariable(name)) {
//       return {{global, PtrClass::DataPtr}};
//     }

//     if (Function *func = m.getFunction(name)) {
//       return {{func, PtrClass::Function}};
//     }

//     debug("Failed to find named value: " << name);
//     return nullopt;
//   }

//   /////////////////////////////////////////////////////////////////////////////

//   void push_return_block(AllocaInst *value, BasicBlock *block) {
//     m_return_block_stack.push(FunctionReturnBlock(value, block));
//   }

//   const FunctionReturnBlock &get_return_block() const {
//     if (m_return_block_stack.empty()) [[unlikely]] {
//       qcore_panic("return block stack is empty");
//     }

//     return m_return_block_stack.top();
//   }

//   void pop_return_block() {
//     if (m_return_block_stack.empty()) [[unlikely]] {
//       qcore_panic("cannot pop the last return block");
//     }

//     m_return_block_stack.pop();
//   }

//   /////////////////////////////////////////////////////////////////////////////

//   bool is_inside_function() const { return m_inside_fn; }
//   void set_inside_function(bool inside) { m_inside_fn = inside; }

//   /////////////////////////////////////////////////////////////////////////////

//   void push_linkage(GlobalValue::LinkageTypes linkage) {
//     m_current_linkage.push(linkage);
//   }

//   GlobalValue::LinkageTypes get_linkage() const {
//     if (m_current_linkage.empty()) [[unlikely]] {
//       qcore_panic("linkage stack is empty");
//     }

//     return m_current_linkage.top();
//   }

//   void pop_linkage() {
//     if (m_current_linkage.empty()) [[unlikely]] {
//       qcore_panic("cannot pop the last linkage");
//     }

//     m_current_linkage.pop();
//   }

//   /////////////////////////////////////////////////////////////////////////////

//   void push_break_block(BasicBlock *block) { m_break_stack.push(block); }

//   optional<BasicBlock *> get_break_block() const {
//     if (m_break_stack.empty()) {
//       return nullopt;
//     }

//     return m_break_stack.top();
//   }

//   void pop_break_block() {
//     if (m_break_stack.empty()) {
//       qcore_panic("cannot pop the last break block");
//     }

//     m_break_stack.pop();
//   }

//   /////////////////////////////////////////////////////////////////////////////

//   void push_skip_block(BasicBlock *block) { m_skip_stack.push(block); }

//   optional<BasicBlock *> get_skip_block() const {
//     if (m_skip_stack.empty()) {
//       return nullopt;
//     }

//     return m_skip_stack.top();
//   }

//   void pop_skip_block() {
//     if (m_skip_stack.empty()) {
//       qcore_panic("cannot pop the last skip block");
//     }

//     m_skip_stack.pop();
//   }

//   /////////////////////////////////////////////////////////////////////////////

//   void push_stackframe(Function *fn) {
//     m_stackframe.push(FunctionStackFrame(fn));
//   }

//   FunctionStackFrame &get_stackframe() {
//     if (m_stackframe.empty()) {
//       qcore_panic("stackframe is empty");
//     }

//     return m_stackframe.top();
//   }

//   void pop_stackframe() {
//     if (m_stackframe.empty()) {
//       qcore_panic("cannot pop the last stackframe");
//     }

//     m_stackframe.pop();
//   }
// };

class OStreamWriter : public streambuf {
  FILE *m_file;

public:
  OStreamWriter(FILE *file) : m_file(file) {}

  virtual streamsize xsputn(const char *s, streamsize n) override {
    return fwrite(s, 1, n, m_file);
  }

  virtual int overflow(int c) override { return fputc(c, m_file); }

  // Get current position
  virtual streampos seekoff(streamoff off, ios_base::seekdir way,
                            ios_base::openmode) override {
    if (way == ios_base::cur) {
      if (fseek(m_file, off, SEEK_CUR) == -1) {
        return -1;
      }
    } else if (way == ios_base::end) {
      if (fseek(m_file, off, SEEK_END) == -1) {
        return -1;
      }
    } else if (way == ios_base::beg) {
      if (fseek(m_file, off, SEEK_SET) == -1) {
        return -1;
      }
    }

    return ftell(m_file);
  }

  virtual streampos seekpos(streampos sp, ios_base::openmode) override {
    if (fseek(m_file, sp, SEEK_SET) == -1) {
      return -1;
    }

    return ftell(m_file);
  }
};

class OStreamDiscard : public streambuf {
public:
  virtual streamsize xsputn(const char *, streamsize n) override { return n; }
  virtual int overflow(int c) override { return c; }
};

class my_pwrite_ostream : public raw_pwrite_stream {
  ostream &m_os;

public:
  my_pwrite_ostream(ostream &os) : raw_pwrite_stream(true), m_os(os) {}

  void write_impl(const char *ptr, size_t size) override {
    m_os.write(ptr, size);
  }

  void pwrite_impl(const char *ptr, size_t size, uint64_t offset) override {
    auto curpos = current_pos();

    m_os.seekp(offset);
    m_os.write(ptr, size);
    m_os.seekp(curpos);
  }

  uint64_t current_pos() const override {
    auto pos = m_os.tellp();
    qcore_assert(pos != -1, "failed to get current position");

    return pos;
  }
};

// static auto T_gen(craft_t &b, FlowPtr<Expr> N) -> ty_t;
// static auto V_gen(ctx_t &m, craft_t &b, State &s, FlowPtr<Expr> N) -> val_t;

// #define T(N) T_gen(b, N)
// #define V(N) V_gen(m, b, s, N)

// static void make_forward_declaration(ctx_t &m, craft_t &b, State &,
//                                      const Function *N) {
//   vector<llvm::Type *> args;
//   for (auto &arg : N->getParams()) {
//     auto ty = T(arg.first);
//     if (!ty) {
//       debug("Failed to forward declare function: " << N->getName());
//       return;
//     }

//     args.push_back(*ty);
//   }

//   auto ret_ty = T(N->getReturn());
//   if (!ret_ty) {
//     debug("Failed to forward declare function: " << N->getName());
//     return;
//   }

//   auto fn_ty = FunctionType::get(*ret_ty, args, false);
//   m.getOrInsertFunction(N->getName(), fn_ty);

//   debug("Forward declared function: " << N->getName());
// }

// static optional<unique_ptr<Module>> fabricate_llvmir(IRModule *src,
//                                                      qcode_conf_t *, ostream
//                                                      &e, raw_ostream &) {
//   static thread_local unique_ptr<LLVMContext> context;

//   auto root = src->getRoot();
//   if (!root) {
//     e << "error: missing root node" << endl;
//     return nullopt;
//   }

//   if (root->getKind() != IR_eSEQ) {
//     e << "error: expected sequence node as root" << endl;
//     return nullopt;
//   }

//   context = make_unique<LLVMContext>();
//   unique_ptr<IRBuilder<>> b = make_unique<IRBuilder<>>(*context);
//   unique_ptr<Module> m = make_unique<Module>(src->getName(), *context);

//   State s;

//   // Forward declare all functions
//   iterate<dfs_pre>(root, [&](auto, auto N) -> IterOp {
//     if ((*N)->getKind() == IR_eSEQ || (*N)->getKind() == IR_eEXTERN) {
//       return IterOp::Proceed;
//     } else if ((*N)->getKind() != IR_eFUNCTION) {
//       return IterOp::SkipChildren;
//     }

//     make_forward_declaration(*m, *b, s, (*N)->template as<Function>());

//     return IterOp::Proceed;
//   });

//   (void)make_forward_declaration;

//   const Seq *seq = root->as<Seq>();

//   for (auto &node : seq->getItems()) {
//     val_t R = V_gen(*m, *b, s, node);
//     if (!R) {
//       e << "error: failed to lower code" << endl;
//       return nullopt;
//     }
//   }

//   return m;
// }

// static val_t binexpr_do_cast(ctx_t &m, craft_t &b, State &s, Value *L, Op O,
//                              llvm::Type *R, FlowPtr<ncc::ir::Type> LT,
//                              FlowPtr<ncc::ir::Type> RT) {
//   val_t E;

//   if (LT->isSame(RT.get())) {
//     return L;
//   }

//   switch (O) {
//     case Op::BitcastAs: {
//       if (LT->is_pointer() && RT->is_integral()) {
//         E = b.CreatePtrToInt(L, R);
//       } else if (LT->is_integral() && RT->is_pointer()) {
//         E = b.CreateIntToPtr(L, R);
//       } else {
//         E = b.CreateBitCast(L, R);
//       }
//       break;
//     }

//     case Op::CastAs: {
//       llvm::Type *IR_LT = L->getType();
//       /* Handle floating point */
//       if (IR_LT->isFloatingPointTy() && RT->is_signed()) {
//         E = b.CreateFPToSI(L, R);
//       } else if (IR_LT->isFloatingPointTy() && RT->is_unsigned()) {
//         E = b.CreateFPToUI(L, R);
//       } else if (LT->is_signed() && RT->is_floating_point()) {
//         E = b.CreateSIToFP(L, R);
//       } else if (LT->is_unsigned() && RT->is_floating_point()) {
//         E = b.CreateUIToFP(L, R);
//       }

//       /* Integer stuff */
//       else if (LT->is_signed() && RT->is_signed()) {
//         E = b.CreateSExtOrTrunc(L, R);
//       } else if (LT->is_unsigned() && RT->is_signed()) {
//         E = b.CreateZExtOrTrunc(L, R);
//       } else if (LT->is_signed() && RT->is_unsigned()) {
//         E = b.CreateSExtOrTrunc(L, R);
//       } else if (LT->is_unsigned() && RT->is_unsigned()) {
//         E = b.CreateZExtOrTrunc(L, R);
//       }

//       /* Composite casting */
//       else if (LT->is_array() && RT->getKind() == IR_tSTRUCT) {
//         ArrayTy *base = LT->as<ArrayTy>();
//         StructTy *ST = RT->as<StructTy>();

//         if (base->getCount() == ST->getFields().size()) {
//           StructType *new_st_ty = cast<StructType>(R);
//           AllocaInst *new_st = b.CreateAlloca(new_st_ty);

//           for (size_t i = 0; i < ST->getFields().size(); i++) {
//             cout << "Casting element " << i << endl;
//             auto x = base->getElement()->getType();
//             if (!x.has_value()) {
//               debug("Failed to get element type");
//               return nullopt;
//             }

//             val_t F = binexpr_do_cast(m, b, s, b.CreateExtractValue(L, i),
//                                       Op::CastAs,
//                                       new_st_ty->getElementType(i),
//                                       x.value(), ST->getFields()[i]);
//             if (!F) {
//               debug("Failed to cast element " << i);
//               return nullopt;
//             }

//             b.CreateStore(F.value(), b.CreateStructGEP(new_st_ty, new_st,
//             i));
//           }

//           E = b.CreateLoad(new_st->getAllocatedType(), new_st);
//         }
//       } else if (LT->is_array() && RT->is_array()) {
//         ArrayTy *FROM = LT->as<ArrayTy>();
//         ArrayTy *TO = RT->as<ArrayTy>();

//         if (FROM->getCount() == TO->getCount()) {
//           ArrayType *new_arr_ty = cast<ArrayType>(R);
//           AllocaInst *new_arr = b.CreateAlloca(new_arr_ty);

//           for (size_t i = 0; i < FROM->getCount(); i++) {
//             auto x = FROM->getElement()->getType();
//             if (!x.has_value()) {
//               debug("Failed to get element type");
//               return nullopt;
//             }
//             auto y = TO->getElement()->getType();
//             if (!y.has_value()) {
//               debug("Failed to get element type");
//               return nullopt;
//             }
//             val_t F = binexpr_do_cast(m, b, s, b.CreateExtractValue(L, i),
//                                       Op::CastAs,
//                                       new_arr_ty->getElementType(),
//                                       x.value(), y.value());
//             if (!F) {
//               debug("Failed to cast element " << i);
//               return nullopt;
//             }

//             b.CreateStore(F.value(), b.CreateStructGEP(new_arr_ty, new_arr,
//             i));
//           }

//           E = b.CreateLoad(new_arr->getAllocatedType(), new_arr);
//         }
//       } else {
//         cout << "Failed to cast from " << LT->getKindName() << " to "
//              << RT->getKindName() << endl;
//       }
//       break;
//     }

//     default: {
//       qcore_panic("unexpected binary operator");
//     }
//   }

//   return E;
// }

// namespace lower {
//   namespace expr {
//     static val_t for_BINEXPR(ctx_t &m, craft_t &b, State &s, const BinExpr
//     *N) {
// #define PROD_LHS()              \
//   val_t L = V(N->getLHS());     \
//   if (!L) {                     \
//     debug("Failed to get LHS"); \
//     return nullopt;             \
//   }

//       Op O = N->getOp();

//       if (N->getRHS()->isType()) { /* Do casting */
//         PROD_LHS()

//         auto TY = T(N->getRHS()->asType());
//         if (!TY.has_value()) {
//           debug("Failed to get RHS type");
//           return nullopt;
//         }

//         auto L_T = N->getLHS()->getType();
//         if (!L_T.has_value()) {
//           debug("Failed to get LHS type");
//           return nullopt;
//         }

//         auto R_T = N->getRHS()->getType();
//         if (!R_T.has_value()) {
//           debug("Failed to get RHS type");
//           return nullopt;
//         }

//         return binexpr_do_cast(m, b, s, L.value(), O, TY.value(),
//         L_T.value(),
//                                R_T.value());
//       }

//       val_t R = V(N->getRHS());

//       val_t E;

//       switch (O) {
//         case Op::Plus: { /* '+': Addition operator */
//           PROD_LHS()
//           E = b.CreateAdd(L.value(), R.value());
//           break;
//         }

//         case Op::Minus: { /* '-': Subtraction operator */
//           PROD_LHS()
//           E = b.CreateSub(L.value(), R.value());
//           break;
//         }

//         case Op::Times: { /* '*': Multiplication operator */
//           PROD_LHS()
//           E = b.CreateMul(L.value(), R.value());
//           break;
//         }

//         case Op::Slash: { /* '/': Division operator */
//           PROD_LHS()
//           auto x = N->getLHS()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }
//           if (x.value()->is_signed()) {
//             E = b.CreateSDiv(L.value(), R.value());
//           } else if (x.value()->is_unsigned()) {
//             E = b.CreateUDiv(L.value(), R.value());
//           } else {
//             qcore_panic("unexpected type for division");
//           }
//           break;
//         }

//         case Op::Percent: { /* '%': Modulus operator */
//           PROD_LHS()
//           auto x = N->getLHS()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }
//           if (x.value()->is_signed()) {
//             E = b.CreateSRem(L.value(), R.value());
//           } else if (x.value()->is_unsigned()) {
//             E = b.CreateURem(L.value(), R.value());
//           } else {
//             qcore_panic("unexpected type for modulus");
//           }
//           break;
//         }

//         case Op::BitAnd: { /* '&': Bitwise AND operator */
//           PROD_LHS()
//           E = b.CreateAnd(L.value(), R.value());
//           break;
//         }

//         case Op::BitOr: { /* '|': Bitwise OR operator */
//           PROD_LHS()
//           E = b.CreateOr(L.value(), R.value());
//           break;
//         }

//         case Op::BitXor: { /* '^': Bitwise XOR operator */
//           PROD_LHS()
//           E = b.CreateXor(L.value(), R.value());
//           break;
//         }

//         case Op::LogicAnd: { /* '&&': Logical AND operator */
//           PROD_LHS()
//           E = b.CreateLogicalAnd(L.value(), R.value());
//           break;
//         }

//         case Op::LogicOr: { /* '||': Logical OR operator */
//           PROD_LHS()
//           E = b.CreateLogicalOr(L.value(), R.value());
//           break;
//         }

//         case Op::LShift: { /* '<<': Left shift operator */
//           PROD_LHS()
//           E = b.CreateShl(L.value(), R.value());
//           break;
//         }

//         case Op::RShift: { /* '>>': Right shift operator */
//           PROD_LHS()
//           auto x = N->getLHS()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }
//           if (x.value()->is_signed()) {
//             E = b.CreateAShr(L.value(), R.value());
//           } else if (x.value()->is_unsigned()) {
//             E = b.CreateLShr(L.value(), R.value());
//           } else {
//             qcore_panic("unexpected type for shift");
//           }
//           break;
//         }

//         case Op::Set: { /* '=': Assignment operator */
//           if (N->getLHS()->getKind() == IR_eIDENT) {
//             if (auto find = s.find_named_value(
//                     m, N->getLHS()->as<Ident>()->getName())) {
//               if (find->second == PtrClass::DataPtr) {
//                 b.CreateStore(R.value(), find->first);

//                 E = R;
//               }
//             }
//           }
//           break;
//         }

//         case Op::LT: { /* '<': Less than operator */
//           PROD_LHS()
//           auto x = N->getLHS()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }
//           if (x.value()->is_signed()) {
//             E = b.CreateICmpSLT(L.value(), R.value());
//           } else if (x.value()->is_unsigned()) {
//             E = b.CreateICmpULT(L.value(), R.value());
//           } else {
//             qcore_panic("unexpected type for comparison");
//           }
//           break;
//         }

//         case Op::GT: { /* '>': Greater than operator */
//           PROD_LHS()
//           auto x = N->getLHS()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }
//           if (x.value()->is_signed()) {
//             E = b.CreateICmpSGT(L.value(), R.value());
//           } else if (x.value()->is_unsigned()) {
//             E = b.CreateICmpUGT(L.value(), R.value());
//           } else {
//             qcore_panic("unexpected type for comparison");
//           }
//           break;
//         }

//         case Op::LE: { /* '<=': Less than or equal to operator */
//           PROD_LHS()
//           auto x = N->getLHS()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }
//           if (x.value()->is_signed()) {
//             E = b.CreateICmpSLE(L.value(), R.value());
//           } else if (x.value()->is_unsigned()) {
//             E = b.CreateICmpULE(L.value(), R.value());
//           } else {
//             qcore_panic("unexpected type for comparison");
//           }
//           break;
//         }

//         case Op::GE: { /* '>=': Greater than or equal to operator */
//           PROD_LHS()
//           auto x = N->getLHS()->getType();
//           if (x.value()->is_signed()) {
//             E = b.CreateICmpSGE(L.value(), R.value());
//           } else if (x.value()->is_unsigned()) {
//             E = b.CreateICmpUGE(L.value(), R.value());
//           } else {
//             qcore_panic("unexpected type for comparison");
//           }
//           break;
//         }

//         case Op::Eq: { /* '==': Equal to operator */
//           PROD_LHS()
//           E = b.CreateICmpEQ(L.value(), R.value());
//           break;
//         }

//         case Op::NE: { /* '!=': Not equal to operator */
//           PROD_LHS()
//           E = b.CreateICmpNE(L.value(), R.value());
//           break;
//         }

//         default: {
//           qcore_panic("unexpected binary operator");
//         }
//       }

//       return E;
//     }

//     static val_t for_UNEXPR(ctx_t &m, craft_t &b, State &s, const Unary *N) {
//       val_t R;

//       val_t E = V(N->getExpr());
//       if (!E) {
//         debug("Failed to get expression");
//         return nullopt;
//       }

//       switch (N->getOp()) {
//         case Op::Plus: {
//           R = V(N->getExpr());
//           break;
//         }
//         case Op::Minus: {
//           R = b.CreateNeg(E.value());
//           break;
//         }
//         case Op::Times: {
//           /// TODO: Dereference
//           break;
//         }
//         case Op::BitAnd: {
//           if (N->getExpr()->getKind() != IR_eIDENT) {
//             qcore_panic("expected identifier for address_of");
//           }

//           Ident *I = N->getExpr()->as<Ident>();
//           auto find = s.find_named_value(m, I->getName());
//           if (!find) {
//             qcore_panic("failed to find identifier for address_of");
//           }

//           Value *V = find->first;
//           if (!V->getType()->isPointerTy()) {
//             qcore_panic("expected pointer type for address_of");
//           }

//           R = V;
//           break;
//         }
//         case Op::BitNot: {
//           R = b.CreateNot(E.value());
//           break;
//         }
//         case Op::LogicNot: {
//           R = b.CreateICmpEQ(E.value(),
//                              ConstantInt::get(b.getContext(), APInt(1, 0)));
//           break;
//         }
//         case Op::Inc: {
//           /// TODO: Increment increment
//           break;
//         }
//         case Op::Dec: {
//           /// TODO: Decrement decrement
//           break;
//         }
//         case Op::Alignof: {
//           auto x = N->getExpr()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }

//           if (auto align = x.value()->getAlignBytes()) {
//             R = ConstantInt::get(b.getContext(), APInt(64, align.value()));
//           } else {
//             qcore_panic("Failed to get alignment");
//           }

//           break;
//         }
//         case Op::Bitsizeof: {
//           auto x = N->getExpr()->getType();
//           if (!x.has_value()) {
//             debug("Failed to get type");
//             return nullopt;
//           }
//           if (auto size = x.value()->getSizeBits()) {
//             R = ConstantInt::get(b.getContext(), APInt(64, size.value()));
//           } else {
//             qcore_panic("Failed to get size");
//           }
//           break;
//         }

//         default: {
//           qcore_panic("unexpected unary operator");
//         }
//       }

//       return N->isPostfix() ? E : R;
//     }

//     static val_t for_INT(ctx_t &, craft_t &b, State &, const Int *N) {
//       unsigned __int128 lit = N->getValue().convert_to<unsigned __int128>();

//       array<uint64_t, 2> parts;
//       parts[1] = (lit >> 64) & 0xffffffffffffffff;
//       parts[0] = lit & 0xffffffffffffffff;

//       return ConstantInt::get(b.getContext(), APInt(N->getSize(), parts));
//     }

//     static val_t for_FLOAT(ctx_t &, craft_t &b, State &, const Float *N) {
//       return ConstantFP::get(b.getContext(), APFloat(N->getValue()));
//     }

//     static val_t for_LIST(ctx_t &m, craft_t &b, State &s, const List *N) {
//       if (N->size() == 0) {
//         StructType *ST = StructType::get(b.getContext(), {}, true);
//         AllocaInst *AI = b.CreateAlloca(ST);
//         return b.CreateLoad(ST, AI);
//       }

//       vector<Value *> items;
//       items.reserve(N->size());

//       for (let node : *N) {
//         val_t R = V(node);
//         if (!R) {
//           debug("Failed to get item");
//           return nullopt;
//         }

//         items.push_back(R.value());
//       }

//       bool is_homogeneous = all_of(items.begin(), items.end(), [&](Value *V)
//       {
//         return V->getType() == items[0]->getType();
//       });

//       if (is_homogeneous) {  // It's a Basic Array
//         ArrayType *AT = ArrayType::get(items[0]->getType(), items.size());
//         AllocaInst *AI = b.CreateAlloca(AT);

//         for (size_t i = 0; i < items.size(); i++) {
//           b.CreateStore(items[i], b.CreateStructGEP(AT, AI, i));
//         }

//         return b.CreateLoad(AT, AI);
//       } else {  // It's an implicit struct value
//         vector<llvm::Type *> types;
//         types.reserve(items.size());
//         for (auto &item : items) {
//           types.push_back(item->getType());
//         }

//         StructType *ST = StructType::get(b.getContext(), types, true);
//         AllocaInst *AI = b.CreateAlloca(ST);

//         for (size_t i = 0; i < items.size(); i++) {
//           b.CreateStore(items[i], b.CreateStructGEP(ST, AI, i));
//         }

//         return b.CreateLoad(ST, AI);
//       }
//     }

//     static val_t for_SEQ(ctx_t &m, craft_t &b, State &s, const Seq *N) {
//       if (N->getItems().empty()) {
//         return
//         Constant::getNullValue(llvm::Type::getInt32Ty(b.getContext()));
//       }

//       val_t R;

//       for (auto &node : N->getItems()) {
//         R = V(node);
//         if (!R.has_value()) {
//           debug("Failed to get item");
//           return nullopt;
//         }
//       }

//       return R;
//     }

//     static val_t for_INDEX(ctx_t &m, craft_t &b, State &s, const Index *N) {
//       val_t I = V(N->getIndex());
//       if (!I) {
//         debug("Failed to get index");
//         return nullopt;
//       }

//       if (N->getExpr()->getKind() == IR_eIDENT) {
//         Ident *B = N->getExpr()->as<Ident>();
//         auto find = s.find_named_value(m, B->getName());
//         if (!find) {
//           debug("Failed to find named value " << B->getName());
//           return nullopt;
//         }

//         if (find->second != PtrClass::DataPtr) {
//           qcore_panic("expected data pointer");
//         }

//         if (!find->first->getType()->getNonOpaquePointerElementType()) {
//           qcore_panic("unexpected type for index");
//         }

//         llvm::Type *base_ty =
//             find->first->getType()->getNonOpaquePointerElementType();

//         if (base_ty->isArrayTy()) {
//           Value *zero = ConstantInt::get(b.getContext(), APInt(32, 0));
//           Value *indices[] = {zero, I.value()};

//           Value *elem = b.CreateGEP(base_ty, find->first, indices);

//           return b.CreateLoad(base_ty->getArrayElementType(), elem);
//         } else if (base_ty->isPointerTy()) {
//           Value *elem =
//           b.CreateGEP(base_ty->getNonOpaquePointerElementType(),
//                                     find->first, I.value());

//           return b.CreateLoad(base_ty->getNonOpaquePointerElementType(),
//           elem);
//         } else {
//           qcore_panic("unexpected type for index");
//         }
//       } else if (N->getExpr()->getKind() == IR_eLIST) {
//         qcore_implement();
//       } else {
//         qcore_panic("unexpected base expression for index");
//       }
//     }

//     static val_t for_IDENT(ctx_t &m, craft_t &b, State &s, const Ident *N) {
//       if (auto find = s.find_named_value(m, N->getName())) {
//         debug("Found named value " << N->getName());

//         if (find->second == PtrClass::Function) {
//           return find->first;
//         } else if (find->second == PtrClass::DataPtr) {
//           if (find->first->getType()->isPointerTy()) {
//             auto type =
//                 find->first->getType()->getNonOpaquePointerElementType();

//             return b.CreateLoad(type, find->first);
//           }
//         }
//       }

//       debug("Failed to produce ident value " << N->getName());

//       return nullopt;
//     }

//     static val_t for_ASM(ctx_t &, craft_t &, State &, const Asm *) {
//       qcore_implement();
//     }
//   }  // namespace expr

//   namespace symbol {

//     static val_t for_EXTERN(ctx_t &m, craft_t &b, State &s, const Extern *N)
//     {
//       s.push_linkage(GlobalValue::ExternalLinkage);

//       val_t R = V(N->getValue());
//       if (!R) {
//         s.pop_linkage();
//         debug("Failed to get value");
//         return nullopt;
//       }

//       s.pop_linkage();

//       return R;
//     }

//     static val_t for_LOCAL(ctx_t &m, craft_t &b, State &s, const Local *N) {
//       auto x = N->getValue()->getType();
//       if (!x.has_value()) {
//         debug("Failed to get type");
//         return nullopt;
//       }

//       ty_t R_T = T(x.value());
//       if (!R_T) {
//         debug("Failed to get type");
//         return nullopt;
//       }

//       if (s.is_inside_function()) {
//         AllocaInst *local = b.CreateAlloca(R_T.value(), nullptr,
//         N->getName());

//         if (N->getValue()) {
//           val_t R = V(N->getValue());
//           if (!R) {
//             debug("Failed to get value");
//             return nullopt;
//           }
//           b.CreateStore(R.value(), local);
//         } else {
//           b.CreateStore(Constant::getNullValue(R_T.value()), local);
//         }

//         s.get_stackframe().addVariable(N->getName(), local);
//         return local;
//       } else {
//         auto init = Constant::getNullValue(R_T.value());

//         GlobalVariable *global = new GlobalVariable(
//             m, R_T.value(), false, s.get_linkage(), init, N->getName());

//         /// TODO: Set the initializer value during program load???
//         return global;
//       }
//     }

//     static val_t for_FN(ctx_t &m, craft_t &b, State &s, const Function *N) {
//       vector<llvm::Type *> params;

//       { /* Lower parameter types */
//         params.reserve(N->getParams().size());

//         for (auto &param : N->getParams()) {
//           ty_t R = T(param.first);
//           if (!R) {
//             debug("Failed to get parameter type");
//             return nullopt;
//           }

//           params.push_back(R.value());
//         }
//       }

//       llvm::Type *ret_ty;

//       { /* Lower return type */
//         auto x = N->getType();
//         if (!x.has_value()) {
//           debug("Failed to get return type");
//           return nullopt;
//         }
//         ty_t R = T(x.value()->as<FnTy>()->getReturn());
//         if (!R) {
//           debug("Failed to get return type");
//           return nullopt;
//         }

//         ret_ty = R.value();
//       }

//       FunctionType *fn_ty = FunctionType::get(ret_ty, params, false);
//       Value *callee = m.getOrInsertFunction(N->getName(), fn_ty).getCallee();

//       if (!N->getBody().has_value()) {  // It is a declaration
//         return callee;
//       }

//       s.set_inside_function(true);

//       Function *fn = dyn_cast<Function>(callee);

//       s.push_stackframe(fn);

//       { /* Lower function body */
//         BasicBlock *entry, *exit;

//         entry = BasicBlock::Create(b.getContext(), "entry", fn);
//         exit = BasicBlock::Create(b.getContext(), "end", fn);

//         b.SetInsertPoint(entry);

//         AllocaInst *ret_val_alloc = nullptr;
//         if (!ret_ty->isVoidTy()) {
//           ret_val_alloc = b.CreateAlloca(ret_ty, nullptr, "__ret");
//         }
//         s.push_return_block(ret_val_alloc, exit);

//         {
//           b.SetInsertPoint(exit);
//           if (!ret_ty->isVoidTy()) {
//             LoadInst *ret_val =
//                 b.CreateLoad(ret_ty, s.get_return_block().getValue());
//             b.CreateRet(ret_val);
//           } else {
//             b.CreateRetVoid();
//           }
//         }

//         b.SetInsertPoint(entry);

//         for (size_t i = 0; i < N->getParams().size(); i++) {
//           fn->getArg(i)->SetName(N->getParams()[i].second);

//           AllocaInst *param_alloc = b.CreateAlloca(
//               fn->getArg(i)->getType(), nullptr, N->getParams()[i].second);

//           b.CreateStore(fn->getArg(i), param_alloc);
//           s.get_stackframe().addVariable(N->getParams()[i].second,
//           param_alloc);
//         }

//         bool old_branch_early = s.branch_early;

//         for (auto &node : N->getBody().value()->getItems()) {
//           val_t R = V(node);
//           if (!R) {
//             s.pop_return_block();
//             s.pop_stackframe();
//             debug("Failed to get body");
//             return nullopt;
//           }
//         }

//         if (!s.branch_early) {
//           b.CreateBr(exit);
//         }
//         s.branch_early = old_branch_early;

//         s.pop_return_block();
//         s.pop_stackframe();
//       }

//       return fn;
//     }

//   }  // namespace symbol

//   namespace control {
//     static val_t for_CALL(ctx_t &m, craft_t &b, State &s, const Call *N) {
//       if (N->getTarget()->is(IR_eIDENT)) {
//         let func_name = N->getTarget()->getName();

//         if (let find = s.find_named_value(m, func_name)) {
//           if (find->second == PtrClass::Function) { /* Direct call */
//             let func_def = cast<Function>(find->first);

//             let arguments = N->getArgs();
//             vector<Value *> args(arguments.size());

//             /* Lower the function arguments */
//             bool failed = false;
//             transform(arguments.begin(), arguments.end(), args.begin(),
//                       [&](auto node) -> Value * {
//                         if (let R = V(node)) {
//                           return R.value();
//                         } else {
//                           failed = true;
//                           debug("Failed to get argument");

//                           return nullptr;
//                         }
//                       });

//             if (failed) {
//               return nullopt;
//             }

//             /* LLVM will perform validation on the arguments */
//             return b.CreateCall(func_def, args);
//           }
//         }
//       } else { /* Indirect call */
//         if (let T = V(N->getTarget())) {
//           let target = T.value()->getType();

//           /* Ensure the target is a function */
//           if (target->isFunctionTy()) {
//             let function = cast<Function>(T.value());

//             let arguments = N->getArgs();
//             vector<Value *> args(arguments.size());

//             bool failed = false;
//             transform(arguments.begin(), arguments.end(), args.begin(),
//                       [&](auto node) -> Value * {
//                         if (let R = V(node)) {
//                           return R.value();
//                         } else {
//                           failed = true;
//                           debug("Failed to get argument");

//                           return nullptr;
//                         }
//                       });

//             return b.CreateCall(function, args);
//           }
//         }
//       }

//       return nullopt;
//     }

//     static val_t for_RET(ctx_t &m, craft_t &b, State &s, const Ret *N) {
//       if (!N->getExpr()->is(IR_tVOID)) {
//         if (let R = V(N->getExpr())) {
//           b.CreateStore(R.value(), s.get_return_block().getValue());
//         } else {
//           debug("Failed to get return value");

//           return nullopt;
//         }
//       }

//       let br = b.CreateBr(s.get_return_block().getBlock());
//       s.branch_early = true;

//       return br;
//     }

//     static val_t for_BRK(ctx_t &, craft_t &b, State &s, const Brk *) {
//       if (let block = s.get_break_block()) {
//         s.branch_early = true;

//         return b.CreateBr(block.value());
//       } else {
//         return nullopt;
//       }
//     }

//     static val_t for_CONT(ctx_t &, craft_t &b, State &s, const Cont *) {
//       if (let block = s.get_skip_block()) {
//         s.branch_early = true;

//         return b.CreateBr(block.value());
//       } else {
//         return nullopt;
//       }
//     }

//     static val_t for_IF(ctx_t &m, craft_t &b, State &s, const If *N) {
//       if (!s.is_inside_function()) {
//         return nullopt;
//       }

//       let then = BasicBlock::Create(b.getContext(), "",
//                                     s.get_stackframe().getFunction());
//       let els = BasicBlock::Create(b.getContext(), "",
//                                    s.get_stackframe().getFunction());
//       let end = BasicBlock::Create(b.getContext(), "",
//                                    s.get_stackframe().getFunction());

//       let old_branch_early = s.branch_early;
//       s.branch_early = false;

//       if (let R = V(N->getCond())) {
//         b.CreateCondBr(R.value(), then, els);
//         b.SetInsertPoint(then);

//         if (let R_T = V(N->getThen())) {
//           if (!s.branch_early) {
//             b.CreateBr(end);
//           }
//           s.branch_early = false;

//           b.SetInsertPoint(els);
//           if (let R_E = V(N->getElse())) {
//             if (!s.branch_early) {
//               b.CreateBr(end);
//             }
//             s.branch_early = old_branch_early;
//             b.SetInsertPoint(end);

//             return end;
//           }
//         }
//       }

//       s.branch_early = old_branch_early;

//       return nullopt;
//     }

//     static val_t for_WHILE(ctx_t &m, craft_t &b, State &s, const While *N) {
//       if (!s.is_inside_function()) {
//         return nullopt;
//       }

//       let begin = BasicBlock::Create(b.getContext(), "",
//                                      s.get_stackframe().getFunction());
//       let body = BasicBlock::Create(b.getContext(), "",
//                                     s.get_stackframe().getFunction());
//       let end = BasicBlock::Create(b.getContext(), "",
//                                    s.get_stackframe().getFunction());

//       b.CreateBr(begin);
//       b.SetInsertPoint(begin);

//       val_t R;

//       if (let C = V(N->getCond())) {
//         b.CreateCondBr(C.value(), body, end);
//         b.SetInsertPoint(body);

//         let old_branch_early = s.branch_early;
//         s.branch_early = false;

//         s.push_break_block(end);
//         s.push_skip_block(begin);

//         if (let R_B = V(N->getBody())) {
//           if (!s.branch_early) {
//             b.CreateBr(begin);
//           }
//           s.branch_early = true;

//           b.SetInsertPoint(end);

//           R = end;
//         }

//         s.branch_early = old_branch_early;

//         s.pop_skip_block();
//         s.pop_break_block();
//       }

//       return R;
//     }

//     static val_t for_FOR(ctx_t &m, craft_t &b, State &s, const For *N) {
//       if (!s.is_inside_function()) {
//         return nullopt;
//       }

//       let begin = BasicBlock::Create(b.getContext(), "",
//                                      s.get_stackframe().getFunction());
//       let body = BasicBlock::Create(b.getContext(), "",
//                                     s.get_stackframe().getFunction());
//       let step = BasicBlock::Create(b.getContext(), "",
//                                     s.get_stackframe().getFunction());
//       let end = BasicBlock::Create(b.getContext(), "",
//                                    s.get_stackframe().getFunction());

//       val_t R;

//       if (let I = V(N->getInit())) {
//         b.CreateBr(begin);

//         b.SetInsertPoint(begin);
//         if (let C = V(N->getCond())) {
//           b.CreateCondBr(C.value(), body, end);

//           b.SetInsertPoint(step);
//           if (let S = V(N->getStep())) {
//             b.CreateBr(begin);

//             bool old_branch_early = s.branch_early;
//             s.branch_early = false;

//             s.push_break_block(end);
//             s.push_skip_block(step);

//             b.SetInsertPoint(body);
//             if (let B = V(N->getBody())) {
//               if (!s.branch_early) {
//                 b.CreateBr(step);
//               }
//               s.branch_early = true;

//               R = end;
//               b.SetInsertPoint(end);
//             }

//             s.pop_skip_block();
//             s.pop_break_block();

//             s.branch_early = old_branch_early;
//           }
//         }
//       }

//       return R;
//     }

//     static val_t for_CASE(ctx_t &, craft_t &, State &, const Case *) {
//       qcore_panic("code path unreachable");
//     }

//     static bool check_switch_trivial(const Switch *N) {
//       auto C_T = N->getCond()->getType();
//       if (!C_T) {
//         debug("Failed to get condition type");
//         return false;
//       }

//       if (!C_T.value()->is_integral()) {
//         return false;
//       }

//       for (auto &node : N->getCases()) {
//         Case *C = node->as<Case>();
//         auto x = C->getCond()->getType();
//         if (!x) {
//           debug("Failed to get case condition type");
//           return false;
//         }
//         if (!x.value()->isSame(C_T.value())) {
//           return false;
//         }
//       }

//       return true;
//     }

//     static val_t for_SWITCH(ctx_t &m, craft_t &b, State &s, const Switch *N)
//     {
//       val_t R = V(N->getCond());
//       if (!R) {
//         debug("Failed to get condition");
//         return nullopt;
//       }

//       bool is_trivial = check_switch_trivial(N);

//       if (is_trivial) {
//         BasicBlock *end = BasicBlock::Create(b.getContext(), "",
//                                              s.get_stackframe().getFunction());
//         s.push_break_block(end);

//         SwitchInst *SI = b.CreateSwitch(R.value(), end,
//         N->getCases().size());

//         for (auto &node : N->getCases()) {
//           BasicBlock *case_block = BasicBlock::Create(
//               b.getContext(), "", s.get_stackframe().getFunction());
//           b.SetInsertPoint(case_block);
//           case_block->moveBefore(end);

//           Case *C = node->as<Case>();

//           val_t R_C = V(C->getCond());
//           if (!R_C) {
//             debug("Failed to get case condition");
//             return nullopt;
//           }

//           bool branch_early = s.branch_early;
//           val_t R_B = V(C->getBody());
//           if (!R_B) {
//             debug("Failed to get case body");
//             return nullopt;
//           }
//           s.branch_early = branch_early;

//           SI->addCase(cast<ConstantInt>(R_C.value()), case_block);
//         }

//         s.pop_break_block();

//         b.SetInsertPoint(end);

//         return SI;
//       } else {
//         /// TODO: Implement conversion for node

//         qcore_implement();
//       }
//     }

//   }  // namespace control

//   namespace types {
//     namespace prim {
//       static auto for_U1_TY(craft_t &b, const U1Ty *) {
//         return llvm::Type::getInt1Ty(b.getContext());
//       }

//       static auto for_U8_TY(craft_t &b, const U8Ty *) {
//         return llvm::Type::getInt8Ty(b.getContext());
//       }

//       static auto for_U16_TY(craft_t &b, const U16Ty *) {
//         return llvm::Type::getInt16Ty(b.getContext());
//       }

//       static auto for_U32_TY(craft_t &b, const U32Ty *) {
//         return llvm::Type::getInt32Ty(b.getContext());
//       }

//       static auto for_U64_TY(craft_t &b, const U64Ty *) {
//         return llvm::Type::getInt64Ty(b.getContext());
//       }

//       static auto for_U128_TY(craft_t &b, const U128Ty *) {
//         return llvm::Type::getInt128Ty(b.getContext());
//       }

//       static auto for_I8_TY(craft_t &b, const I8Ty *) {
//         return llvm::Type::getInt8Ty(b.getContext());
//       }

//       static auto for_I16_TY(craft_t &b, const I16Ty *) {
//         return llvm::Type::getInt16Ty(b.getContext());
//       }

//       static auto for_I32_TY(craft_t &b, const I32Ty *) {
//         return llvm::Type::getInt32Ty(b.getContext());
//       }

//       static auto for_I64_TY(craft_t &b, const I64Ty *) {
//         return llvm::Type::getInt64Ty(b.getContext());
//       }

//       static auto for_I128_TY(craft_t &b, const I128Ty *) {
//         return llvm::Type::getInt128Ty(b.getContext());
//       }

//       static auto for_F16_TY(craft_t &b, const F16Ty *) {
//         return llvm::Type::getHalfTy(b.getContext());
//       }

//       static auto for_F32_TY(craft_t &b, const F32Ty *) {
//         return llvm::Type::getFloatTy(b.getContext());
//       }

//       static auto for_F64_TY(craft_t &b, const F64Ty *) {
//         return llvm::Type::getDoubleTy(b.getContext());
//       }

//       static auto for_F128_TY(craft_t &b, const F128Ty *) {
//         return llvm::Type::getFP128Ty(b.getContext());
//       }

//       static auto for_VOID_TY(craft_t &b, const VoidTy *) {
//         return llvm::Type::getVoidTy(b.getContext());
//       }

//     }  // namespace prim

//     namespace other {
//       static ty_t for_PTR_TY(craft_t &b, const PtrTy *N) {
//         if (ty_t pointee = T(N->getPointee())) {
//           return PointerType::get(pointee.value(), 0);
//         }

//         return nullopt;
//       }

//       static ty_t for_CONST_TY(craft_t &b, const ConstTy *N) {
//         return T(N->getItem());
//       }

//       static ty_t for_FN_TY(craft_t &b, const FnTy *N) {
//         let params = N->getParams();
//         vector<llvm::Type *> param_types(params.size());
//         bool failed = false;

//         transform(params.begin(), params.end(), param_types.begin(),
//                   [&](auto param) -> llvm::Type * {
//                     if (auto R = T(param)) {
//                       return R.value();
//                     } else {
//                       failed = true;

//                       return nullptr;
//                     }
//                   });

//         if (!failed) {
//           if (auto R = T(N->getReturn())) {
//             bool is_vararg = N->isVariadic();

//             return FunctionType::get(R.value(), std::move(param_types),
//                                      is_vararg);
//           }
//         }

//         return nullopt;
//       }

//       static ty_t for_OPAQUE_TY(craft_t &b, const OpaqueTy *N) {
//         if (!N->getName().empty()) {
//           return StructType::create(b.getContext(), N->getName());
//         }

//         return nullopt;
//       }

//       static ty_t for_STRUCT_TY(craft_t &b, const StructTy *N) {
//         let fields = N->getFields();
//         vector<llvm::Type *> elements(fields.size());
//         bool failed = false;

//         transform(fields.begin(), fields.end(), elements.begin(),
//                   [&](auto field) -> llvm::Type * {
//                     if (auto R = T(field)) {
//                       return R.value();
//                     } else {
//                       failed = true;

//                       return nullptr;
//                     }
//                   });

//         if (failed) {
//           return nullopt;
//         }

//         return StructType::get(b.getContext(), std::move(elements), true);
//       }

//       static ty_t for_UNION_TY(craft_t &, const UnionTy *) {
//         /// TODO: Implement conversion for node
//         qcore_implement();
//       }

//       static ty_t for_ARRAY_TY(craft_t &b, const ArrayTy *N) {
//         if (auto R = T(N->getElement())) {
//           return ArrayType::get(R.value(), N->getCount());
//         }

//         return nullopt;
//       }
//     }  // namespace other
//   }  // namespace types
// }  // namespace lower

// #pragma GCC diagnostic pop

// static auto V_gen(ctx_t &m, craft_t &b, State &s, FlowPtr<Expr> N) -> val_t {
//   static let dispatch = []() constexpr {
// #define FUNCTION(_enum, _func, _type)                                       \
//   R[_enum] = [](ctx_t &m, craft_t &b, State &s, FlowPtr<Expr> N) -> val_t { \
//     return _func(m, b, s, N->as<_type>());                                  \
//   }
//     using func_t = val_t (*)(ctx_t &, craft_t &, State &, FlowPtr<Expr>);

//     array<func_t, IR_LAST + 1> R;
//     R.fill([](ctx_t &, craft_t &, State &, auto n) -> val_t {
//       qcore_panicf("illegal node in input: kind=%s", n->getKindName());
//     });

//     { /* NRGraph recursive llvm-ir builders */
//       using namespace lower::control;
//       using namespace lower::expr;
//       using namespace lower::symbol;

//       FUNCTION(IR_eBIN, for_BINEXPR, BinExpr);
//       FUNCTION(IR_eUNARY, for_UNEXPR, Unary);
//       FUNCTION(IR_eINT, for_INT, Int);
//       FUNCTION(IR_eFLOAT, for_FLOAT, Float);
//       FUNCTION(IR_eLIST, for_LIST, List);
//       FUNCTION(IR_eCALL, for_CALL, Call);
//       FUNCTION(IR_eSEQ, for_SEQ, Seq);
//       FUNCTION(IR_eINDEX, for_INDEX, Index);
//       FUNCTION(IR_eIDENT, for_IDENT, Ident);
//       FUNCTION(IR_eEXTERN, for_EXTERN, Extern);
//       FUNCTION(IR_eLOCAL, for_LOCAL, Local);
//       FUNCTION(IR_eRET, for_RET, Ret);
//       FUNCTION(IR_eBRK, for_BRK, Brk);
//       FUNCTION(IR_eSKIP, for_CONT, Cont);
//       FUNCTION(IR_eIF, for_IF, If);
//       FUNCTION(IR_eWHILE, for_WHILE, While);
//       FUNCTION(IR_eFOR, for_FOR, For);
//       FUNCTION(IR_eCASE, for_CASE, Case);
//       FUNCTION(IR_eSWITCH, for_SWITCH, Switch);
//       FUNCTION(IR_eFUNCTION, for_FN, Fn);
//       FUNCTION(IR_eASM, for_ASM, Asm);

//       R[IR_eIGN] = [](ctx_t &, craft_t &, State &, auto) -> val_t {
//         return nullptr;
//       };

//       R[IR_tTMP] = [](ctx_t &, craft_t &, State &, auto) -> val_t {
//         qcore_panic("unexpected temporary node");
//       };
//     }

// #undef FUNCTION

//     return R;
//   }();

//   return dispatch[N->getKind()](m, b, s, N);
// }

// static auto T_gen(craft_t &b, FlowPtr<Expr> N) -> ty_t {
//   static let dispatch = []() constexpr {
// #define FUNCTION(_enum, _func, _type)                  \
//   R[_enum] = [](craft_t &b, FlowPtr<Expr> N) -> ty_t { \
//     return _func(b, N->as<_type>());                   \
//   }
//     using func_t = ty_t (*)(craft_t &, FlowPtr<Expr>);

//     array<func_t, IR_LAST + 1> R;
//     R.fill([](craft_t &, auto n) -> ty_t {
//       qcore_panicf("illegal node in input: kind=%s", n->getKindName());
//     });

//     { /* NRGraph recursive llvm-ir builders */
//       using namespace lower::types::prim;
//       using namespace lower::types::other;

//       FUNCTION(IR_tU1, for_U1_TY, U1Ty);
//       FUNCTION(IR_tU8, for_U8_TY, U8Ty);
//       FUNCTION(IR_tU16, for_U16_TY, U16Ty);
//       FUNCTION(IR_tU32, for_U32_TY, U32Ty);
//       FUNCTION(IR_tU64, for_U64_TY, U64Ty);
//       FUNCTION(IR_tU128, for_U128_TY, U128Ty);
//       FUNCTION(IR_tI8, for_I8_TY, I8Ty);
//       FUNCTION(IR_tI16, for_I16_TY, I16Ty);
//       FUNCTION(IR_tI32, for_I32_TY, I32Ty);
//       FUNCTION(IR_tI64, for_I64_TY, I64Ty);
//       FUNCTION(IR_tI128, for_I128_TY, I128Ty);
//       FUNCTION(IR_tF16_TY, for_F16_TY, F16Ty);
//       FUNCTION(IR_tF32_TY, for_F32_TY, F32Ty);
//       FUNCTION(IR_tF64_TY, for_F64_TY, F64Ty);
//       FUNCTION(IR_tF128_TY, for_F128_TY, F128Ty);
//       FUNCTION(IR_tVOID, for_VOID_TY, VoidTy);
//       FUNCTION(IR_tPTR, for_PTR_TY, PtrTy);
//       FUNCTION(IR_tCONST, for_CONST_TY, ConstTy);
//       FUNCTION(IR_tOPAQUE, for_OPAQUE_TY, OpaqueTy);
//       FUNCTION(IR_tSTRUCT, for_STRUCT_TY, StructTy);
//       FUNCTION(IR_tUNION, for_UNION_TY, UnionTy);
//       FUNCTION(IR_tARRAY, for_ARRAY_TY, ArrayTy);
//       FUNCTION(IR_tFUNC, for_FN_TY, FnTy);

//       R[IR_eIGN] = [](craft_t &, auto) -> ty_t { return nullptr; };

//       R[IR_tTMP] = [](craft_t &, auto) -> ty_t {
//         qcore_panic("unexpected temporary node");
//       };
//     }

// #undef FUNCTION

//     return R;
//   }();

//   return dispatch[N->getKind()](b, N);
// }

static bool qcode_adapter(IRModule *module, qcode_conf_t *conf, FILE *err,
                          FILE *out, qcode_adapter_fn impl) {
  unique_ptr<streambuf> err_stream_buf, out_stream_buf;

  {
    /* If the error stream is provided, use it. Otherwise, discard the
    output.
     */
    if (err) {
      err_stream_buf = make_unique<OStreamWriter>(err);
    } else {
      err_stream_buf = make_unique<OStreamDiscard>();
    }

    /* If the output stream is provided, use it. Otherwise, discard the
    output.
     */
    if (out) {
      out_stream_buf = make_unique<OStreamWriter>(out);
    } else {
      out_stream_buf = make_unique<OStreamDiscard>();
    }
  }

  ostream err_stream(err_stream_buf.get());
  ostream out_stream(out_stream_buf.get());

  {
    my_pwrite_ostream llvm_adapt(out_stream);
    if (!impl(module, conf, err_stream, llvm_adapt)) {
      err_stream.flush();
      out_stream.flush();
      err &&fflush(err);
      out &&fflush(out);
      return false;
    }
  }

  err_stream.flush();
  out_stream.flush();
  err &&fflush(err);
  out &&fflush(out);

  return true;
}

static optional<unique_ptr<Module>> fabricate_llvmir(IRModule *module,
                                                     qcode_conf_t *conf,
                                                     ostream &err,
                                                     raw_ostream &out) {
  /// TODO: Implement conversion for node
  qcore_implement();
}

NCC_EXPORT bool qcode_ir(IRModule *module, qcode_conf_t *conf, FILE *err,
                         FILE *out) {
  return qcode_adapter(module, conf, err, out,
                       [](IRModule *m, qcode_conf_t *c, ostream &e,
                          raw_pwrite_stream &o) -> bool {
                         auto module = fabricate_llvmir(m, c, e, o);
                         if (!module) {
                           e << "error: failed to fabricate LLVM IR" << endl;
                           return false;
                         }

                         bool failed = verifyModule(*module->get(), &o);

                         module.value()->print(o, nullptr);

                         return !failed;
                       });
}

NCC_EXPORT bool qcode_asm(IRModule *module, qcode_conf_t *conf, FILE *err,
                          FILE *out) {
  return qcode_adapter(
      module, conf, err, out,
      [](IRModule *m, qcode_conf_t *c, ostream &e,
         raw_pwrite_stream &o) -> bool {
        auto module_opt = fabricate_llvmir(m, c, e, o);
        if (!module_opt) {
          e << "error: failed to fabricate LLVM IR" << endl;
          return false;
        }

        auto targetTriple = m->GetTargetInfo().TargetTriple.value_or(
            sys::getDefaultTargetTriple());
        auto CPU = m->GetTargetInfo().CPU.value_or("generic");
        ncc::string Features = m->GetTargetInfo().CPUFeatures.value_or("");
        bool relocPIC = true;

        TargetOptions opt;
        std::string lookupTarget_err;
        auto Target =
            TargetRegistry::lookupTarget(targetTriple.Get(), lookupTarget_err);
        if (!Target) {
          e << "error: failed to lookup target: " << lookupTarget_err << endl;
          return false;
        }

        auto TargetMachine = Target->createTargetMachine(
            targetTriple.Get(), CPU.Get(), Features.Get(), opt,
            relocPIC ? Reloc::PIC_ : Reloc::Static);

        auto &module = *module_opt.value();

        if (verifyModule(module, &o)) {
          e << "error: failed to verify module" << endl;
          return false;
        }

        module.setDataLayout(TargetMachine->createDataLayout());
        module.setTargetTriple(targetTriple.Get());

        ///==========================================================================

        // Create the analysis managers.
        // These must be declared in this order so that they are destroyed in
        // the correct order due to inter-analysis-manager references.
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager MAM;

        // Create the new pass manager builder.
        // Take a look at the PassBuilder constructor parameters for more
        // customization, e.g. specifying a TargetMachine or various debugging
        // options.
        PassBuilder PB;

        // Register all the basic analyses with the managers.
        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        ModulePassManager MPM =
            PB.buildPerModuleDefaultPipeline(OptimizationLevel::O3);

        // Optimize the IR!
        MPM.run(module, MAM);

        ///==========================================================================

        error_code ec;

        legacy::PassManager pass;
        TargetMachine->addPassesToEmitFile(pass, o, nullptr,
                                           CodeGenFileType::AssemblyFile);
        if (!pass.run(module)) {
          e << "error: failed to emit object code" << endl;
          return false;
        }

        return true;
      });
}

NCC_EXPORT bool qcode_obj(IRModule *module, qcode_conf_t *conf, FILE *err,
                          FILE *out) {
  return qcode_adapter(
      module, conf, err, out,
      [](IRModule *m, qcode_conf_t *c, ostream &e,
         raw_pwrite_stream &o) -> bool {
        auto module_opt = fabricate_llvmir(m, c, e, o);
        if (!module_opt) {
          e << "error: failed to fabricate LLVM IR" << endl;
          return false;
        }

        ncc::string targetTriple = m->GetTargetInfo().TargetTriple.value_or(
            sys::getDefaultTargetTriple());
        ncc::string CPU = m->GetTargetInfo().CPU.value_or("generic");
        ncc::string Features = m->GetTargetInfo().CPUFeatures.value_or("");
        bool relocPIC = true;

        TargetOptions opt;
        std::string lookupTarget_err;
        auto Target =
            TargetRegistry::lookupTarget(targetTriple.Get(), lookupTarget_err);
        if (!Target) {
          e << "error: failed to lookup target: " << lookupTarget_err << endl;
          return false;
        }

        auto TargetMachine = Target->createTargetMachine(
            targetTriple.Get(), CPU.Get(), Features.Get(), opt,
            relocPIC ? Reloc::PIC_ : Reloc::Static);

        auto &module = *module_opt.value();

        if (verifyModule(module, &o)) {
          e << "error: failed to verify module" << endl;
          return false;
        }

        module.setDataLayout(TargetMachine->createDataLayout());
        module.setTargetTriple(targetTriple.Get());

        ///==========================================================================

        // Create the analysis managers.
        // These must be declared in this order so that they are destroyed in
        // the correct order due to inter-analysis-manager references.
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager MAM;

        // Create the new pass manager builder.
        // Take a look at the PassBuilder constructor parameters for more
        // customization, e.g. specifying a TargetMachine or various
        // debugging options.
        PassBuilder PB;

        // Register all the basic analyses with the managers.
        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        ModulePassManager MPM =
            PB.buildPerModuleDefaultPipeline(OptimizationLevel::O3);

        // Optimize the IR!
        MPM.run(module, MAM);

        ///==========================================================================

        error_code ec;

        legacy::PassManager pass;
        TargetMachine->addPassesToEmitFile(pass, o, nullptr,
                                           CodeGenFileType::ObjectFile);
        if (!pass.run(module)) {
          e << "error: failed to emit object code" << endl;
          return false;
        }

        return true;
      });
}
