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
#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Config.h>
#include <nitrate-ir/TypeDecl.h>
#include <sys/types.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <optional>
#include <stack>
#include <streambuf>
#include <unordered_map>

/// TODO: Find way to remove the 's.branch_early' edge case

using namespace llvm;
using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#if defined(QCORE_DEBUG)
#define debug(...) \
  cerr << "[debug]: ln " << __LINE__ << ": " << __VA_ARGS__ << endl
#else
#define debug(...)
#endif

#define let const auto &

typedef function<bool(qmodule_t *, qcode_conf_t *, ostream &err,
                      raw_pwrite_stream &out)>
    qcode_adapter_fn;

using ctx_t = Module;
using craft_t = IRBuilder<>;
using val_t = optional<Value *>;
using ty_t = optional<Type *>;

enum class PtrClass {
  DataPtr,
  Function,
};

class FunctionReturnBlock {
  AllocaInst *m_value;
  BasicBlock *m_block;

public:
  FunctionReturnBlock(AllocaInst *value, BasicBlock *block)
      : m_value(value), m_block(block) {}

  AllocaInst *getValue() const { return m_value; }
  BasicBlock *getBlock() const { return m_block; }
};

class FunctionStackFrame {
  Function *m_fn;
  unordered_map<string_view, AllocaInst *> m_locals;

public:
  FunctionStackFrame(Function *fn) : m_fn(fn) {}

  Function *getFunction() const { return m_fn; }

  void addVariable(string_view name, AllocaInst *inst) {
    m_locals[name] = inst;
  }

  const auto &getLocalVariables() const { return m_locals; }
};

class State {
  stack<FunctionReturnBlock> m_return_block_stack;
  stack<GlobalValue::LinkageTypes> m_current_linkage;
  stack<BasicBlock *> m_break_stack;
  stack<BasicBlock *> m_skip_stack;
  stack<FunctionStackFrame> m_stackframe;
  bool m_inside_fn;

public:
  bool branch_early;

public:
  State() {
    m_current_linkage.push(GlobalValue::LinkageTypes::PrivateLinkage);
    m_inside_fn = false;
    branch_early = false;
  }

  optional<pair<Value *, PtrClass>> find_named_value(ctx_t &m,
                                                     string_view name) const {
    if (is_inside_function()) {
      for (let[cur_name, inst] : m_stackframe.top().getLocalVariables()) {
        if (cur_name == name) {
          return {{inst, PtrClass::DataPtr}};
        }
      }
    }

    if (GlobalVariable *global = m.getGlobalVariable(name)) {
      return {{global, PtrClass::DataPtr}};
    }

    if (Function *func = m.getFunction(name)) {
      return {{func, PtrClass::Function}};
    }

    debug("Failed to find named value: " << name);
    return nullopt;
  }

  /////////////////////////////////////////////////////////////////////////////

  void push_return_block(AllocaInst *value, BasicBlock *block) {
    m_return_block_stack.push(FunctionReturnBlock(value, block));
  }

  const FunctionReturnBlock &get_return_block() const {
    if (m_return_block_stack.empty()) [[unlikely]] {
      qcore_panic("return block stack is empty");
    }

    return m_return_block_stack.top();
  }

  void pop_return_block() {
    if (m_return_block_stack.empty()) [[unlikely]] {
      qcore_panic("cannot pop the last return block");
    }

    m_return_block_stack.pop();
  }

  /////////////////////////////////////////////////////////////////////////////

  bool is_inside_function() const { return m_inside_fn; }
  void set_inside_function(bool inside) { m_inside_fn = inside; }

  /////////////////////////////////////////////////////////////////////////////

  void push_linkage(GlobalValue::LinkageTypes linkage) {
    m_current_linkage.push(linkage);
  }

  GlobalValue::LinkageTypes get_linkage() const {
    if (m_current_linkage.empty()) [[unlikely]] {
      qcore_panic("linkage stack is empty");
    }

    return m_current_linkage.top();
  }

  void pop_linkage() {
    if (m_current_linkage.empty()) [[unlikely]] {
      qcore_panic("cannot pop the last linkage");
    }

    m_current_linkage.pop();
  }

  /////////////////////////////////////////////////////////////////////////////

  void push_break_block(BasicBlock *block) { m_break_stack.push(block); }

  optional<BasicBlock *> get_break_block() const {
    if (m_break_stack.empty()) {
      return nullopt;
    }

    return m_break_stack.top();
  }

  void pop_break_block() {
    if (m_break_stack.empty()) {
      qcore_panic("cannot pop the last break block");
    }

    m_break_stack.pop();
  }

  /////////////////////////////////////////////////////////////////////////////

  void push_skip_block(BasicBlock *block) { m_skip_stack.push(block); }

  optional<BasicBlock *> get_skip_block() const {
    if (m_skip_stack.empty()) {
      return nullopt;
    }

    return m_skip_stack.top();
  }

  void pop_skip_block() {
    if (m_skip_stack.empty()) {
      qcore_panic("cannot pop the last skip block");
    }

    m_skip_stack.pop();
  }

  /////////////////////////////////////////////////////////////////////////////

  void push_stackframe(Function *fn) {
    m_stackframe.push(FunctionStackFrame(fn));
  }

  FunctionStackFrame &get_stackframe() {
    if (m_stackframe.empty()) {
      qcore_panic("stackframe is empty");
    }

    return m_stackframe.top();
  }

  void pop_stackframe() {
    if (m_stackframe.empty()) {
      qcore_panic("cannot pop the last stackframe");
    }

    m_stackframe.pop();
  }
};

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

static auto T_gen(craft_t &b, const nr::Expr *N) -> ty_t;
static auto V_gen(ctx_t &m, craft_t &b, State &s, const nr::Expr *N) -> val_t;

#define T(N) T_gen(b, N)
#define V(N) V_gen(m, b, s, N)

static void make_forward_declaration(ctx_t &m, craft_t &b, State &,
                                     const nr::Fn *N) {
  vector<Type *> args;
  for (auto &arg : N->getParams()) {
    auto ty = T(arg.first);
    if (!ty) {
      debug("Failed to forward declare function: " << N->getName());
      return;
    }

    args.push_back(*ty);
  }

  auto ret_ty = T(N->getReturn());
  if (!ret_ty) {
    debug("Failed to forward declare function: " << N->getName());
    return;
  }

  auto fn_ty = FunctionType::get(*ret_ty, args, false);
  m.getOrInsertFunction(N->getName(), fn_ty);

  debug("Forward declared function: " << N->getName());
}

static optional<unique_ptr<Module>> fabricate_llvmir(const qmodule_t *src,
                                                     qcode_conf_t *, ostream &e,
                                                     raw_ostream &) {
  static thread_local unique_ptr<LLVMContext> context;

  let root = src->getRoot();
  if (!root) {
    e << "error: missing root node" << endl;
    return nullopt;
  }

  if (root->getKind() != NR_NODE_SEQ) {
    e << "error: expected sequence node as root" << endl;
    return nullopt;
  }

  context = make_unique<LLVMContext>();
  unique_ptr<IRBuilder<>> b = make_unique<IRBuilder<>>(*context);
  unique_ptr<Module> m = make_unique<Module>(src->getName(), *context);

  State s;

  // Forward declare all functions
  nr::iterate<nr::dfs_pre>(
      root,
      [&](const nr::Expr *, const nr::Expr *const *const N) -> nr::IterOp {
        if ((*N)->getKind() == NR_NODE_SEQ ||
            (*N)->getKind() == NR_NODE_EXTERN) {
          return nr::IterOp::Proceed;
        } else if ((*N)->getKind() != NR_NODE_FN) {
          return nr::IterOp::SkipChildren;
        }

        make_forward_declaration(*m, *b, s, (*N)->as<nr::Fn>());

        return nr::IterOp::Proceed;
      });

  const nr::Seq *seq = root->as<nr::Seq>();

  for (auto &node : seq->getItems()) {
    val_t R = V_gen(*m, *b, s, node);
    if (!R) {
      e << "error: failed to lower code" << endl;
      return nullopt;
    }
  }

  return m;
}

static val_t binexpr_do_cast(ctx_t &m, craft_t &b, State &s, Value *L, nr::Op O,
                             Type *R, nr::Type *LT, nr::Type *RT) {
  val_t E;

  if (LT->isSame(RT)) {
    return L;
  }

  switch (O) {
    case nr::Op::BitcastAs: {
      if (LT->is_pointer() && RT->is_integral()) {
        E = b.CreatePtrToInt(L, R);
      } else if (LT->is_integral() && RT->is_pointer()) {
        E = b.CreateIntToPtr(L, R);
      } else {
        E = b.CreateBitCast(L, R);
      }
      break;
    }

    case nr::Op::CastAs: {
      Type *IR_LT = L->getType();
      /* Handle floating point */
      if (IR_LT->isFloatingPointTy() && RT->is_signed()) {
        E = b.CreateFPToSI(L, R);
      } else if (IR_LT->isFloatingPointTy() && RT->is_unsigned()) {
        E = b.CreateFPToUI(L, R);
      } else if (LT->is_signed() && RT->is_floating_point()) {
        E = b.CreateSIToFP(L, R);
      } else if (LT->is_unsigned() && RT->is_floating_point()) {
        E = b.CreateUIToFP(L, R);
      }

      /* Integer stuff */
      else if (LT->is_signed() && RT->is_signed()) {
        E = b.CreateSExtOrTrunc(L, R);
      } else if (LT->is_unsigned() && RT->is_signed()) {
        E = b.CreateZExtOrTrunc(L, R);
      } else if (LT->is_signed() && RT->is_unsigned()) {
        E = b.CreateSExtOrTrunc(L, R);
      } else if (LT->is_unsigned() && RT->is_unsigned()) {
        E = b.CreateZExtOrTrunc(L, R);
      }

      /* Composite casting */
      else if (LT->is_array() && RT->getKind() == NR_NODE_STRUCT_TY) {
        nr::ArrayTy *base = LT->as<nr::ArrayTy>();
        nr::StructTy *ST = RT->as<nr::StructTy>();

        if (base->getCount() == ST->getFields().size()) {
          StructType *new_st_ty = cast<StructType>(R);
          AllocaInst *new_st = b.CreateAlloca(new_st_ty);

          for (size_t i = 0; i < ST->getFields().size(); i++) {
            cout << "Casting element " << i << endl;
            auto x = base->getElement()->getType();
            if (!x.has_value()) {
              debug("Failed to get element type");
              return nullopt;
            }

            val_t F = binexpr_do_cast(
                m, b, s, b.CreateExtractValue(L, i), nr::Op::CastAs,
                new_st_ty->getElementType(i), x.value(), ST->getFields()[i]);
            if (!F) {
              debug("Failed to cast element " << i);
              return nullopt;
            }

            b.CreateStore(F.value(), b.CreateStructGEP(new_st_ty, new_st, i));
          }

          E = b.CreateLoad(new_st->getAllocatedType(), new_st);
        }
      } else if (LT->is_array() && RT->is_array()) {
        nr::ArrayTy *FROM = LT->as<nr::ArrayTy>();
        nr::ArrayTy *TO = RT->as<nr::ArrayTy>();

        if (FROM->getCount() == TO->getCount()) {
          ArrayType *new_arr_ty = cast<ArrayType>(R);
          AllocaInst *new_arr = b.CreateAlloca(new_arr_ty);

          for (size_t i = 0; i < FROM->getCount(); i++) {
            auto x = FROM->getElement()->getType();
            if (!x.has_value()) {
              debug("Failed to get element type");
              return nullopt;
            }
            auto y = TO->getElement()->getType();
            if (!y.has_value()) {
              debug("Failed to get element type");
              return nullopt;
            }
            val_t F = binexpr_do_cast(
                m, b, s, b.CreateExtractValue(L, i), nr::Op::CastAs,
                new_arr_ty->getElementType(), x.value(), y.value());
            if (!F) {
              debug("Failed to cast element " << i);
              return nullopt;
            }

            b.CreateStore(F.value(), b.CreateStructGEP(new_arr_ty, new_arr, i));
          }

          E = b.CreateLoad(new_arr->getAllocatedType(), new_arr);
        }
      } else {
        cout << "Failed to cast from " << LT->getKindName() << " to "
             << RT->getKindName() << endl;
      }
      break;
    }

    default: {
      qcore_panic("unexpected binary operator");
    }
  }

  return E;
}

namespace lower {
  namespace expr {
    static val_t for_BINEXPR(ctx_t &m, craft_t &b, State &s,
                             const nr::BinExpr *N) {
#define PROD_LHS()              \
  val_t L = V(N->getLHS());     \
  if (!L) {                     \
    debug("Failed to get LHS"); \
    return nullopt;             \
  }

      nr::Op O = N->getOp();

      if (N->getRHS()->isType()) { /* Do casting */
        PROD_LHS()

        auto TY = T(N->getRHS()->asType());
        if (!TY.has_value()) {
          debug("Failed to get RHS type");
          return nullopt;
        }

        auto L_T = N->getLHS()->getType();
        if (!L_T.has_value()) {
          debug("Failed to get LHS type");
          return nullopt;
        }

        auto R_T = N->getRHS()->getType();
        if (!R_T.has_value()) {
          debug("Failed to get RHS type");
          return nullopt;
        }

        return binexpr_do_cast(m, b, s, L.value(), O, TY.value(), L_T.value(),
                               R_T.value());
      }

      val_t R = V(N->getRHS());

      val_t E;

      switch (O) {
        case nr::Op::Plus: { /* '+': Addition operator */
          PROD_LHS()
          E = b.CreateAdd(L.value(), R.value());
          break;
        }

        case nr::Op::Minus: { /* '-': Subtraction operator */
          PROD_LHS()
          E = b.CreateSub(L.value(), R.value());
          break;
        }

        case nr::Op::Times: { /* '*': Multiplication operator */
          PROD_LHS()
          E = b.CreateMul(L.value(), R.value());
          break;
        }

        case nr::Op::Slash: { /* '/': Division operator */
          PROD_LHS()
          auto x = N->getLHS()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (x.value()->is_signed()) {
            E = b.CreateSDiv(L.value(), R.value());
          } else if (x.value()->is_unsigned()) {
            E = b.CreateUDiv(L.value(), R.value());
          } else {
            qcore_panic("unexpected type for division");
          }
          break;
        }

        case nr::Op::Percent: { /* '%': Modulus operator */
          PROD_LHS()
          auto x = N->getLHS()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (x.value()->is_signed()) {
            E = b.CreateSRem(L.value(), R.value());
          } else if (x.value()->is_unsigned()) {
            E = b.CreateURem(L.value(), R.value());
          } else {
            qcore_panic("unexpected type for modulus");
          }
          break;
        }

        case nr::Op::BitAnd: { /* '&': Bitwise AND operator */
          PROD_LHS()
          E = b.CreateAnd(L.value(), R.value());
          break;
        }

        case nr::Op::BitOr: { /* '|': Bitwise OR operator */
          PROD_LHS()
          E = b.CreateOr(L.value(), R.value());
          break;
        }

        case nr::Op::BitXor: { /* '^': Bitwise XOR operator */
          PROD_LHS()
          E = b.CreateXor(L.value(), R.value());
          break;
        }

        case nr::Op::LogicAnd: { /* '&&': Logical AND operator */
          PROD_LHS()
          E = b.CreateLogicalAnd(L.value(), R.value());
          break;
        }

        case nr::Op::LogicOr: { /* '||': Logical OR operator */
          PROD_LHS()
          E = b.CreateLogicalOr(L.value(), R.value());
          break;
        }

        case nr::Op::LShift: { /* '<<': Left shift operator */
          PROD_LHS()
          E = b.CreateShl(L.value(), R.value());
          break;
        }

        case nr::Op::RShift: { /* '>>': Right shift operator */
          PROD_LHS()
          auto x = N->getLHS()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (x.value()->is_signed()) {
            E = b.CreateAShr(L.value(), R.value());
          } else if (x.value()->is_unsigned()) {
            E = b.CreateLShr(L.value(), R.value());
          } else {
            qcore_panic("unexpected type for shift");
          }
          break;
        }

        case nr::Op::Set: { /* '=': Assignment operator */
          if (N->getLHS()->getKind() == NR_NODE_IDENT) {
            if (auto find = s.find_named_value(
                    m, N->getLHS()->as<nr::Ident>()->getName())) {
              if (find->second == PtrClass::DataPtr) {
                b.CreateStore(R.value(), find->first);

                E = R;
              }
            }
          }
          break;
        }

        case nr::Op::LT: { /* '<': Less than operator */
          PROD_LHS()
          auto x = N->getLHS()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (x.value()->is_signed()) {
            E = b.CreateICmpSLT(L.value(), R.value());
          } else if (x.value()->is_unsigned()) {
            E = b.CreateICmpULT(L.value(), R.value());
          } else {
            qcore_panic("unexpected type for comparison");
          }
          break;
        }

        case nr::Op::GT: { /* '>': Greater than operator */
          PROD_LHS()
          auto x = N->getLHS()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (x.value()->is_signed()) {
            E = b.CreateICmpSGT(L.value(), R.value());
          } else if (x.value()->is_unsigned()) {
            E = b.CreateICmpUGT(L.value(), R.value());
          } else {
            qcore_panic("unexpected type for comparison");
          }
          break;
        }

        case nr::Op::LE: { /* '<=': Less than or equal to operator */
          PROD_LHS()
          auto x = N->getLHS()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (x.value()->is_signed()) {
            E = b.CreateICmpSLE(L.value(), R.value());
          } else if (x.value()->is_unsigned()) {
            E = b.CreateICmpULE(L.value(), R.value());
          } else {
            qcore_panic("unexpected type for comparison");
          }
          break;
        }

        case nr::Op::GE: { /* '>=': Greater than or equal to operator */
          PROD_LHS()
          auto x = N->getLHS()->getType();
          if (x.value()->is_signed()) {
            E = b.CreateICmpSGE(L.value(), R.value());
          } else if (x.value()->is_unsigned()) {
            E = b.CreateICmpUGE(L.value(), R.value());
          } else {
            qcore_panic("unexpected type for comparison");
          }
          break;
        }

        case nr::Op::Eq: { /* '==': Equal to operator */
          PROD_LHS()
          E = b.CreateICmpEQ(L.value(), R.value());
          break;
        }

        case nr::Op::NE: { /* '!=': Not equal to operator */
          PROD_LHS()
          E = b.CreateICmpNE(L.value(), R.value());
          break;
        }

        default: {
          qcore_panic("unexpected binary operator");
        }
      }

      return E;
    }

    static val_t for_UNEXPR(ctx_t &m, craft_t &b, State &s,
                            const nr::UnExpr *N) {
      val_t R;

      switch (N->getOp()) {
        case nr::Op::Plus: {
          R = V(N->getExpr());
          break;
        }
        case nr::Op::Minus: {
          if (val_t E = V(N->getExpr())) {
            R = b.CreateNeg(E.value());
          }
          break;
        }
        case nr::Op::Times: {
          /// TODO: Dereference
          break;
        }
        case nr::Op::BitAnd: {
          if (N->getExpr()->getKind() != NR_NODE_IDENT) {
            qcore_panic("expected identifier for address_of");
          }

          nr::Ident *I = N->getExpr()->as<nr::Ident>();
          auto find = s.find_named_value(m, I->getName());
          if (!find) {
            qcore_panic("failed to find identifier for address_of");
          }

          Value *V = find->first;
          if (!V->getType()->isPointerTy()) {
            qcore_panic("expected pointer type for address_of");
          }

          R = V;
          break;
        }
        case nr::Op::BitNot: {
          if (val_t E = V(N->getExpr())) {
            R = b.CreateNot(E.value());
          }
          break;
        }
        case nr::Op::LogicNot: {
          if (val_t E = V(N->getExpr())) {
            R = b.CreateICmpEQ(E.value(),
                               ConstantInt::get(b.getContext(), APInt(1, 0)));
          }
          break;
        }
        case nr::Op::Inc: {
          if (N->getExpr()->getKind() != NR_NODE_IDENT) {
            qcore_panic("expected identifier for increment");
          }

          nr::Ident *I = N->getExpr()->as<nr::Ident>();
          auto find = s.find_named_value(m, I->getName());
          if (!find) {
            qcore_panic("failed to find identifier for increment");
          }

          if (find->second != PtrClass::DataPtr) {
            qcore_panic("expected data pointer");
          }

          Value *V = find->first;

          if (!V->getType()->isPointerTy()) {
            qcore_panic("expected pointer type for increment");
          }

          Value *current =
              b.CreateLoad(V->getType()->getNonOpaquePointerElementType(), V);
          Value *new_val;

          if (current->getType()->isIntegerTy()) {
            auto x = N->getExpr()->getType();
            if (!x.has_value()) {
              debug("Failed to get type");
              return nullopt;
            }
            if (auto size = x.value()->getSizeBits()) {
              new_val = b.CreateAdd(
                  current,
                  ConstantInt::get(b.getContext(), APInt(size.value(), 1)));
            } else {
              qcore_panic("Failed to get size");
            }
          } else if (current->getType()->isFloatingPointTy()) {
            new_val = b.CreateFAdd(
                current, ConstantFP::get(b.getContext(), APFloat(1.0)));
          } else {
            qcore_panic("unexpected type for increment");
          }

          b.CreateStore(new_val, V);

          R = new_val;
          break;
        }
        case nr::Op::Dec: {
          if (N->getExpr()->getKind() != NR_NODE_IDENT) {
            qcore_panic("expected identifier for decrement");
          }

          nr::Ident *I = N->getExpr()->as<nr::Ident>();
          auto find = s.find_named_value(m, I->getName());
          if (!find) {
            qcore_panic("failed to find identifier for decrement");
          }

          if (find->second != PtrClass::DataPtr) {
            qcore_panic("expected data pointer");
          }

          Value *V = find->first;

          if (!V->getType()->isPointerTy()) {
            qcore_panic("expected pointer type for decrement");
          }

          Value *current =
              b.CreateLoad(V->getType()->getNonOpaquePointerElementType(), V);
          Value *new_val;

          if (current->getType()->isIntegerTy()) {
            auto x = N->getExpr()->getType();
            if (!x.has_value()) {
              debug("Failed to get type");
              return nullopt;
            }
            if (auto size = x.value()->getSizeBits()) {
              new_val = b.CreateSub(
                  current,
                  ConstantInt::get(b.getContext(), APInt(size.value(), 1)));
            } else {
              qcore_panic("Failed to get size");
            }

          } else if (current->getType()->isFloatingPointTy()) {
            new_val = b.CreateFSub(
                current, ConstantFP::get(b.getContext(), APFloat(1.0)));
          } else {
            qcore_panic("unexpected type for decrement");
          }

          b.CreateStore(new_val, V);

          R = new_val;
          break;
        }
        case nr::Op::Alignof: {
          auto x = N->getExpr()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (auto align = x.value()->getAlignBytes()) {
            R = ConstantInt::get(b.getContext(), APInt(64, align.value()));
          } else {
            qcore_panic("Failed to get alignment");
          }
          break;
        }
        case nr::Op::Bitsizeof: {
          auto x = N->getExpr()->getType();
          if (!x.has_value()) {
            debug("Failed to get type");
            return nullopt;
          }
          if (auto size = x.value()->getSizeBits()) {
            R = ConstantInt::get(b.getContext(), APInt(64, size.value()));
          } else {
            qcore_panic("Failed to get size");
          }
          break;
        }

        default: {
          qcore_panic("unexpected unary operator");
        }
      }

      return R;
    }

    static val_t for_POST_UNEXPR(ctx_t &m, craft_t &b, State &s,
                                 const nr::PostUnExpr *N) {
      val_t R;

      switch (N->getOp()) {
        case nr::Op::Inc: {
          if (N->getExpr()->getKind() != NR_NODE_IDENT) {
            qcore_panic("expected identifier for increment");
          }

          nr::Ident *I = N->getExpr()->as<nr::Ident>();
          auto find = s.find_named_value(m, I->getName());
          if (!find) {
            qcore_panic("failed to find identifier for increment");
          }

          if (find->second != PtrClass::DataPtr) {
            qcore_panic("expected data pointer");
          }

          Value *V = find->first;

          if (!V->getType()->isPointerTy()) {
            qcore_panic("expected pointer type for increment");
          }

          Value *current =
              b.CreateLoad(V->getType()->getNonOpaquePointerElementType(), V);
          Value *new_val;

          if (current->getType()->isIntegerTy()) {
            auto x = N->getExpr()->getType();
            if (!x.has_value()) {
              debug("Failed to get type");
              return nullopt;
            }
            if (auto size = x.value()->getSizeBits()) {
              new_val = b.CreateAdd(
                  current,
                  ConstantInt::get(b.getContext(), APInt(size.value(), 1)));
            } else {
              qcore_panic("Failed to get size");
            }
          } else if (current->getType()->isFloatingPointTy()) {
            new_val = b.CreateFAdd(
                current, ConstantFP::get(b.getContext(), APFloat(1.0)));
          } else {
            qcore_panic("unexpected type for increment");
          }

          b.CreateStore(new_val, V);

          R = current;
          break;
        }
        case nr::Op::Dec: {
          if (N->getExpr()->getKind() != NR_NODE_IDENT) {
            qcore_panic("expected identifier for decrement");
          }

          nr::Ident *I = N->getExpr()->as<nr::Ident>();
          auto find = s.find_named_value(m, I->getName());
          if (!find) {
            qcore_panic("failed to find identifier for decrement");
          }

          if (find->second != PtrClass::DataPtr) {
            qcore_panic("expected data pointer");
          }

          Value *V = find->first;

          if (!V->getType()->isPointerTy()) {
            qcore_panic("expected pointer type for decrement");
          }

          Value *current =
              b.CreateLoad(V->getType()->getNonOpaquePointerElementType(), V);
          Value *new_val;

          if (current->getType()->isIntegerTy()) {
            auto x = N->getExpr()->getType();
            if (!x.has_value()) {
              debug("Failed to get type");
              return nullopt;
            }
            if (auto size = x.value()->getSizeBits()) {
              new_val = b.CreateSub(
                  current,
                  ConstantInt::get(b.getContext(), APInt(size.value(), 1)));
            } else {
              qcore_panic("Failed to get size");
            }
          } else if (current->getType()->isFloatingPointTy()) {
            new_val = b.CreateFSub(
                current, ConstantFP::get(b.getContext(), APFloat(1.0)));
          } else {
            qcore_panic("unexpected type for decrement");
          }

          b.CreateStore(new_val, V);

          R = current;
          break;
        }

        default: {
          qcore_panic("unexpected post-unary operator");
        }
      }

      return R;
    }

    static val_t for_INT(ctx_t &, craft_t &b, State &, const nr::Int *N) {
      unsigned __int128 lit = N->getValue().convert_to<unsigned __int128>();

      array<uint64_t, 2> parts;
      parts[1] = (lit >> 64) & 0xffffffffffffffff;
      parts[0] = lit & 0xffffffffffffffff;

      return ConstantInt::get(b.getContext(), APInt(N->getSize(), parts));
    }

    static val_t for_FLOAT(ctx_t &, craft_t &b, State &, const nr::Float *N) {
      return ConstantFP::get(b.getContext(), APFloat(N->getValue()));
    }

    static val_t for_LIST(ctx_t &m, craft_t &b, State &s, const nr::List *N) {
      if (N->size() == 0) {
        StructType *ST = StructType::get(b.getContext(), {}, true);
        AllocaInst *AI = b.CreateAlloca(ST);
        return b.CreateLoad(ST, AI);
      }

      vector<Value *> items;
      items.reserve(N->size());

      for (let node : *N) {
        val_t R = V(node);
        if (!R) {
          debug("Failed to get item");
          return nullopt;
        }

        items.push_back(R.value());
      }

      bool is_homogeneous = all_of(items.begin(), items.end(), [&](Value *V) {
        return V->getType() == items[0]->getType();
      });

      if (is_homogeneous) {  // It's a Basic Array
        ArrayType *AT = ArrayType::get(items[0]->getType(), items.size());
        AllocaInst *AI = b.CreateAlloca(AT);

        for (size_t i = 0; i < items.size(); i++) {
          b.CreateStore(items[i], b.CreateStructGEP(AT, AI, i));
        }

        return b.CreateLoad(AT, AI);
      } else {  // It's an implicit struct value
        vector<Type *> types;
        types.reserve(items.size());
        for (auto &item : items) {
          types.push_back(item->getType());
        }

        StructType *ST = StructType::get(b.getContext(), types, true);
        AllocaInst *AI = b.CreateAlloca(ST);

        for (size_t i = 0; i < items.size(); i++) {
          b.CreateStore(items[i], b.CreateStructGEP(ST, AI, i));
        }

        return b.CreateLoad(ST, AI);
      }
    }

    static val_t for_SEQ(ctx_t &m, craft_t &b, State &s, const nr::Seq *N) {
      if (N->getItems().empty()) {
        return Constant::getNullValue(Type::getInt32Ty(b.getContext()));
      }

      val_t R;

      for (auto &node : N->getItems()) {
        R = V(node);
        if (!R.has_value()) {
          debug("Failed to get item");
          return nullopt;
        }
      }

      return R;
    }

    static val_t for_INDEX(ctx_t &m, craft_t &b, State &s, const nr::Index *N) {
      val_t I = V(N->getIndex());
      if (!I) {
        debug("Failed to get index");
        return nullopt;
      }

      if (N->getExpr()->getKind() == NR_NODE_IDENT) {
        nr::Ident *B = N->getExpr()->as<nr::Ident>();
        auto find = s.find_named_value(m, B->getName());
        if (!find) {
          debug("Failed to find named value " << B->getName());
          return nullopt;
        }

        if (find->second != PtrClass::DataPtr) {
          qcore_panic("expected data pointer");
        }

        if (!find->first->getType()->getNonOpaquePointerElementType()) {
          qcore_panic("unexpected type for index");
        }

        Type *base_ty =
            find->first->getType()->getNonOpaquePointerElementType();

        if (base_ty->isArrayTy()) {
          Value *zero = ConstantInt::get(b.getContext(), APInt(32, 0));
          Value *indices[] = {zero, I.value()};

          Value *elem = b.CreateGEP(base_ty, find->first, indices);

          return b.CreateLoad(base_ty->getArrayElementType(), elem);
        } else if (base_ty->isPointerTy()) {
          Value *elem = b.CreateGEP(base_ty->getNonOpaquePointerElementType(),
                                    find->first, I.value());

          return b.CreateLoad(base_ty->getNonOpaquePointerElementType(), elem);
        } else {
          qcore_panic("unexpected type for index");
        }
      } else if (N->getExpr()->getKind() == NR_NODE_LIST) {
        qcore_implement();
      } else {
        qcore_panic("unexpected base expression for index");
      }
    }

    static val_t for_IDENT(ctx_t &m, craft_t &b, State &s, const nr::Ident *N) {
      if (auto find = s.find_named_value(m, N->getName())) {
        debug("Found named value " << N->getName());

        if (find->second == PtrClass::Function) {
          return find->first;
        } else if (find->second == PtrClass::DataPtr) {
          if (find->first->getType()->isPointerTy()) {
            auto type =
                find->first->getType()->getNonOpaquePointerElementType();

            return b.CreateLoad(type, find->first);
          }
        }
      }

      debug("Failed to produce ident value " << N->getName());

      return nullopt;
    }

    static val_t for_ASM(ctx_t &, craft_t &, State &, const nr::Asm *) {
      qcore_implement();
    }
  }  // namespace expr

  namespace symbol {

    static val_t for_EXTERN(ctx_t &m, craft_t &b, State &s,
                            const nr::Extern *N) {
      s.push_linkage(GlobalValue::ExternalLinkage);

      val_t R = V(N->getValue());
      if (!R) {
        s.pop_linkage();
        debug("Failed to get value");
        return nullopt;
      }

      s.pop_linkage();

      return R;
    }

    static val_t for_LOCAL(ctx_t &m, craft_t &b, State &s, const nr::Local *N) {
      auto x = N->getValue()->getType();
      if (!x.has_value()) {
        debug("Failed to get type");
        return nullopt;
      }

      ty_t R_T = T(x.value());
      if (!R_T) {
        debug("Failed to get type");
        return nullopt;
      }

      if (s.is_inside_function()) {
        AllocaInst *local = b.CreateAlloca(R_T.value(), nullptr, N->getName());

        if (N->getValue()) {
          val_t R = V(N->getValue());
          if (!R) {
            debug("Failed to get value");
            return nullopt;
          }
          b.CreateStore(R.value(), local);
        } else {
          b.CreateStore(Constant::getNullValue(R_T.value()), local);
        }

        s.get_stackframe().addVariable(N->getName(), local);
        return local;
      } else {
        auto init = Constant::getNullValue(R_T.value());

        GlobalVariable *global = new GlobalVariable(
            m, R_T.value(), false, s.get_linkage(), init, N->getName());

        /// TODO: Set the initializer value during program load???
        return global;
      }
    }

    static val_t for_FN(ctx_t &m, craft_t &b, State &s, const nr::Fn *N) {
      vector<Type *> params;

      { /* Lower parameter types */
        params.reserve(N->getParams().size());

        for (auto &param : N->getParams()) {
          ty_t R = T(param.first);
          if (!R) {
            debug("Failed to get parameter type");
            return nullopt;
          }

          params.push_back(R.value());
        }
      }

      Type *ret_ty;

      { /* Lower return type */
        auto x = N->getType();
        if (!x.has_value()) {
          debug("Failed to get return type");
          return nullopt;
        }
        ty_t R = T(x.value()->as<nr::FnTy>()->getReturn());
        if (!R) {
          debug("Failed to get return type");
          return nullopt;
        }

        ret_ty = R.value();
      }

      FunctionType *fn_ty = FunctionType::get(ret_ty, params, false);
      Value *callee = m.getOrInsertFunction(N->getName(), fn_ty).getCallee();

      if (!N->getBody().has_value()) {  // It is a declaration
        return callee;
      }

      s.set_inside_function(true);

      Function *fn = dyn_cast<Function>(callee);

      s.push_stackframe(fn);

      { /* Lower function body */
        BasicBlock *entry, *exit;

        entry = BasicBlock::Create(b.getContext(), "entry", fn);
        exit = BasicBlock::Create(b.getContext(), "end", fn);

        b.SetInsertPoint(entry);

        AllocaInst *ret_val_alloc = nullptr;
        if (!ret_ty->isVoidTy()) {
          ret_val_alloc = b.CreateAlloca(ret_ty, nullptr, "__ret");
        }
        s.push_return_block(ret_val_alloc, exit);

        {
          b.SetInsertPoint(exit);
          if (!ret_ty->isVoidTy()) {
            LoadInst *ret_val =
                b.CreateLoad(ret_ty, s.get_return_block().getValue());
            b.CreateRet(ret_val);
          } else {
            b.CreateRetVoid();
          }
        }

        b.SetInsertPoint(entry);

        for (size_t i = 0; i < N->getParams().size(); i++) {
          fn->getArg(i)->setName(N->getParams()[i].second);

          AllocaInst *param_alloc = b.CreateAlloca(
              fn->getArg(i)->getType(), nullptr, N->getParams()[i].second);

          b.CreateStore(fn->getArg(i), param_alloc);
          s.get_stackframe().addVariable(N->getParams()[i].second, param_alloc);
        }

        bool old_branch_early = s.branch_early;

        for (auto &node : N->getBody().value()->getItems()) {
          val_t R = V(node);
          if (!R) {
            s.pop_return_block();
            s.pop_stackframe();
            debug("Failed to get body");
            return nullopt;
          }
        }

        if (!s.branch_early) {
          b.CreateBr(exit);
        }
        s.branch_early = old_branch_early;

        s.pop_return_block();
        s.pop_stackframe();
      }

      return fn;
    }

  }  // namespace symbol

  namespace control {
    static val_t for_CALL(ctx_t &m, craft_t &b, State &s, const nr::Call *N) {
      if (N->getTarget()->is(NR_NODE_FN)) { /* Direct call */
        let func_name = N->getTarget()->getName();

        if (let find = s.find_named_value(m, func_name)) {
          if (find->second == PtrClass::Function) {
            let func_def = cast<Function>(find->first);

            let arguments = N->getArgs();
            vector<Value *> args(arguments.size());

            /* Lower the function arguments */
            bool failed = false;
            transform(arguments.begin(), arguments.end(), args.begin(),
                      [&](auto node) -> Value * {
                        if (let R = V(node)) {
                          return R.value();
                        } else {
                          failed = true;
                          debug("Failed to get argument");

                          return nullptr;
                        }
                      });

            if (failed) {
              return nullopt;
            }

            /* LLVM will perform validation on the arguments */
            return b.CreateCall(func_def, args);
          }
        }
      } else { /* Indirect call */
        if (let T = V(N->getTarget())) {
          let target = T.value()->getType();

          /* Ensure the target is a function */
          if (target->isFunctionTy()) {
            let function = cast<Function>(T.value());

            let arguments = N->getArgs();
            vector<Value *> args(arguments.size());

            bool failed = false;
            transform(arguments.begin(), arguments.end(), args.begin(),
                      [&](auto node) -> Value * {
                        if (let R = V(node)) {
                          return R.value();
                        } else {
                          failed = true;
                          debug("Failed to get argument");

                          return nullptr;
                        }
                      });

            return b.CreateCall(function, args);
          }
        }
      }

      return nullopt;
    }

    static val_t for_RET(ctx_t &m, craft_t &b, State &s, const nr::Ret *N) {
      if (!N->getExpr()->is(NR_NODE_VOID_TY)) {
        if (let R = V(N->getExpr())) {
          b.CreateStore(R.value(), s.get_return_block().getValue());
        } else {
          debug("Failed to get return value");

          return nullopt;
        }
      }

      let br = b.CreateBr(s.get_return_block().getBlock());
      s.branch_early = true;

      return br;
    }

    static val_t for_BRK(ctx_t &, craft_t &b, State &s, const nr::Brk *) {
      if (let block = s.get_break_block()) {
        s.branch_early = true;

        return b.CreateBr(block.value());
      } else {
        return nullopt;
      }
    }

    static val_t for_CONT(ctx_t &, craft_t &b, State &s, const nr::Cont *) {
      if (let block = s.get_skip_block()) {
        s.branch_early = true;

        return b.CreateBr(block.value());
      } else {
        return nullopt;
      }
    }

    static val_t for_IF(ctx_t &m, craft_t &b, State &s, const nr::If *N) {
      if (!s.is_inside_function()) {
        return nullopt;
      }

      let then = BasicBlock::Create(b.getContext(), "",
                                    s.get_stackframe().getFunction());
      let els = BasicBlock::Create(b.getContext(), "",
                                   s.get_stackframe().getFunction());
      let end = BasicBlock::Create(b.getContext(), "",
                                   s.get_stackframe().getFunction());

      let old_branch_early = s.branch_early;
      s.branch_early = false;

      if (let R = V(N->getCond())) {
        b.CreateCondBr(R.value(), then, els);
        b.SetInsertPoint(then);

        if (let R_T = V(N->getThen())) {
          if (!s.branch_early) {
            b.CreateBr(end);
          }
          s.branch_early = false;

          b.SetInsertPoint(els);
          if (let R_E = V(N->getElse())) {
            if (!s.branch_early) {
              b.CreateBr(end);
            }
            s.branch_early = old_branch_early;
            b.SetInsertPoint(end);

            return end;
          }
        }
      }

      s.branch_early = old_branch_early;

      return nullopt;
    }

    static val_t for_WHILE(ctx_t &m, craft_t &b, State &s, const nr::While *N) {
      if (!s.is_inside_function()) {
        return nullopt;
      }

      let begin = BasicBlock::Create(b.getContext(), "",
                                     s.get_stackframe().getFunction());
      let body = BasicBlock::Create(b.getContext(), "",
                                    s.get_stackframe().getFunction());
      let end = BasicBlock::Create(b.getContext(), "",
                                   s.get_stackframe().getFunction());

      let old_branch_early = s.branch_early;
      s.branch_early = false;

      b.CreateBr(begin);
      b.SetInsertPoint(begin);

      val_t R;

      if (let C = V(N->getCond())) {
        b.CreateCondBr(C.value(), body, end);
        b.SetInsertPoint(body);

        s.push_break_block(end);
        s.push_skip_block(begin);

        if (let R_B = V(N->getBody())) {
          if (!s.branch_early) {
            b.CreateBr(begin);
          }
          s.branch_early = true;

          b.SetInsertPoint(end);

          R = end;
        }

        s.pop_skip_block();
        s.pop_break_block();
      }

      s.branch_early = old_branch_early;

      return R;
    }

    static val_t for_FOR(ctx_t &m, craft_t &b, State &s, const nr::For *N) {
      if (!s.is_inside_function()) {
        return nullopt;
      }

      let begin = BasicBlock::Create(b.getContext(), "",
                                     s.get_stackframe().getFunction());
      let body = BasicBlock::Create(b.getContext(), "",
                                    s.get_stackframe().getFunction());
      let step = BasicBlock::Create(b.getContext(), "",
                                    s.get_stackframe().getFunction());
      let end = BasicBlock::Create(b.getContext(), "",
                                   s.get_stackframe().getFunction());

      val_t R;

      if (let I = V(N->getInit())) {
        b.CreateBr(begin);

        b.SetInsertPoint(begin);
        if (let C = V(N->getCond())) {
          b.CreateCondBr(C.value(), body, end);

          b.SetInsertPoint(step);
          if (let S = V(N->getStep())) {
            b.CreateBr(begin);

            bool old_branch_early = s.branch_early;
            s.branch_early = false;

            s.push_break_block(end);
            s.push_skip_block(step);

            b.SetInsertPoint(body);
            if (let B = V(N->getBody())) {
              if (!s.branch_early) {
                b.CreateBr(step);
              }
              s.branch_early = true;

              R = end;
              b.SetInsertPoint(end);
            }

            s.pop_skip_block();
            s.pop_break_block();

            s.branch_early = old_branch_early;
          }
        }
      }

      return R;
    }

    static val_t for_CASE(ctx_t &, craft_t &, State &, const nr::Case *) {
      qcore_panic("code path unreachable");
    }

    static bool check_switch_trivial(const nr::Switch *N) {
      auto C_T = N->getCond()->getType();
      if (!C_T) {
        debug("Failed to get condition type");
        return false;
      }

      if (!C_T.value()->is_integral()) {
        return false;
      }

      for (auto &node : N->getCases()) {
        nr::Case *C = node->as<nr::Case>();
        auto x = C->getCond()->getType();
        if (!x) {
          debug("Failed to get case condition type");
          return false;
        }
        if (!x.value()->isSame(C_T.value())) {
          return false;
        }
      }

      return true;
    }

    static val_t for_SWITCH(ctx_t &m, craft_t &b, State &s,
                            const nr::Switch *N) {
      val_t R = V(N->getCond());
      if (!R) {
        debug("Failed to get condition");
        return nullopt;
      }

      bool is_trivial = check_switch_trivial(N);

      if (is_trivial) {
        BasicBlock *end = BasicBlock::Create(b.getContext(), "",
                                             s.get_stackframe().getFunction());
        s.push_break_block(end);

        SwitchInst *SI = b.CreateSwitch(R.value(), end, N->getCases().size());

        for (auto &node : N->getCases()) {
          BasicBlock *case_block = BasicBlock::Create(
              b.getContext(), "", s.get_stackframe().getFunction());
          b.SetInsertPoint(case_block);
          case_block->moveBefore(end);

          nr::Case *C = node->as<nr::Case>();

          val_t R_C = V(C->getCond());
          if (!R_C) {
            debug("Failed to get case condition");
            return nullopt;
          }

          bool branch_early = s.branch_early;
          val_t R_B = V(C->getBody());
          if (!R_B) {
            debug("Failed to get case body");
            return nullopt;
          }
          s.branch_early = branch_early;

          SI->addCase(cast<ConstantInt>(R_C.value()), case_block);
        }

        s.pop_break_block();

        b.SetInsertPoint(end);

        return SI;
      } else {
        /// TODO: Implement conversion for node

        qcore_implement();
      }
    }

  }  // namespace control

  namespace types {
    namespace prim {
      static auto for_U1_TY(craft_t &b, const nr::U1Ty *) {
        return Type::getInt1Ty(b.getContext());
      }

      static auto for_U8_TY(craft_t &b, const nr::U8Ty *) {
        return Type::getInt8Ty(b.getContext());
      }

      static auto for_U16_TY(craft_t &b, const nr::U16Ty *) {
        return Type::getInt16Ty(b.getContext());
      }

      static auto for_U32_TY(craft_t &b, const nr::U32Ty *) {
        return Type::getInt32Ty(b.getContext());
      }

      static auto for_U64_TY(craft_t &b, const nr::U64Ty *) {
        return Type::getInt64Ty(b.getContext());
      }

      static auto for_U128_TY(craft_t &b, const nr::U128Ty *) {
        return Type::getInt128Ty(b.getContext());
      }

      static auto for_I8_TY(craft_t &b, const nr::I8Ty *) {
        return Type::getInt8Ty(b.getContext());
      }

      static auto for_I16_TY(craft_t &b, const nr::I16Ty *) {
        return Type::getInt16Ty(b.getContext());
      }

      static auto for_I32_TY(craft_t &b, const nr::I32Ty *) {
        return Type::getInt32Ty(b.getContext());
      }

      static auto for_I64_TY(craft_t &b, const nr::I64Ty *) {
        return Type::getInt64Ty(b.getContext());
      }

      static auto for_I128_TY(craft_t &b, const nr::I128Ty *) {
        return Type::getInt128Ty(b.getContext());
      }

      static auto for_F16_TY(craft_t &b, const nr::F16Ty *) {
        return Type::getHalfTy(b.getContext());
      }

      static auto for_F32_TY(craft_t &b, const nr::F32Ty *) {
        return Type::getFloatTy(b.getContext());
      }

      static auto for_F64_TY(craft_t &b, const nr::F64Ty *) {
        return Type::getDoubleTy(b.getContext());
      }

      static auto for_F128_TY(craft_t &b, const nr::F128Ty *) {
        return Type::getFP128Ty(b.getContext());
      }

      static auto for_VOID_TY(craft_t &b, const nr::VoidTy *) {
        return Type::getVoidTy(b.getContext());
      }

    }  // namespace prim

    namespace other {
      static ty_t for_PTR_TY(craft_t &b, const nr::PtrTy *N) {
        if (ty_t pointee = T(N->getPointee())) {
          return PointerType::get(pointee.value(), 0);
        }

        return nullopt;
      }

      static ty_t for_FN_TY(craft_t &b, const nr::FnTy *N) {
        let params = N->getParams();
        vector<Type *> param_types(params.size());
        bool failed = false;

        transform(params.begin(), params.end(), param_types.begin(),
                  [&](auto param) -> Type * {
                    if (auto R = T(param)) {
                      return R.value();
                    } else {
                      failed = true;

                      return nullptr;
                    }
                  });

        if (!failed) {
          if (auto R = T(N->getReturn())) {
            bool is_vararg = N->getAttrs().contains(nr::FnAttr::Variadic);

            return FunctionType::get(R.value(), std::move(param_types),
                                     is_vararg);
          }
        }

        return nullopt;
      }

      static ty_t for_OPAQUE_TY(craft_t &b, const nr::OpaqueTy *N) {
        if (!N->getName().empty()) {
          return StructType::create(b.getContext(), N->getName());
        }

        return nullopt;
      }

      static ty_t for_STRUCT_TY(craft_t &b, const nr::StructTy *N) {
        let fields = N->getFields();
        vector<Type *> elements(fields.size());
        bool failed = false;

        transform(fields.begin(), fields.end(), elements.begin(),
                  [&](auto field) -> Type * {
                    if (auto R = T(field)) {
                      return R.value();
                    } else {
                      failed = true;

                      return nullptr;
                    }
                  });

        if (failed) {
          return nullopt;
        }

        return StructType::get(b.getContext(), std::move(elements), true);
      }

      static ty_t for_UNION_TY(craft_t &, const nr::UnionTy *) {
        /// TODO: Implement conversion for node
        qcore_implement();
      }

      static ty_t for_ARRAY_TY(craft_t &b, const nr::ArrayTy *N) {
        if (auto R = T(N->getElement())) {
          return ArrayType::get(R.value(), N->getCount());
        }

        return nullopt;
      }
    }  // namespace other
  }  // namespace types
}  // namespace lower

#pragma GCC diagnostic pop

static auto V_gen(ctx_t &m, craft_t &b, State &s, const nr::Expr *N) -> val_t {
  static let dispatch = []() constexpr {
#define FUNCTION(_enum, _func, _type)                                         \
  R[_enum] = [](ctx_t &m, craft_t &b, State &s, const nr::Expr *N) -> val_t { \
    return _func(m, b, s, N->as<_type>());                                    \
  }
    using func_t = val_t (*)(ctx_t &, craft_t &, State &, const nr::Expr *);

    array<func_t, NR_NODE_LAST + 1> R;
    R.fill([](ctx_t &, craft_t &, State &, const nr::Expr *n) -> val_t {
      qcore_panicf("illegal node in input: kind=%s", n->getKindName());
    });

    { /* NRGraph recursive llvm-ir builders */
      using namespace lower::control;
      using namespace lower::expr;
      using namespace lower::symbol;

      FUNCTION(NR_NODE_BINEXPR, for_BINEXPR, nr::BinExpr);
      FUNCTION(NR_NODE_UNEXPR, for_UNEXPR, nr::UnExpr);
      FUNCTION(NR_NODE_POST_UNEXPR, for_POST_UNEXPR, nr::PostUnExpr);
      FUNCTION(NR_NODE_INT, for_INT, nr::Int);
      FUNCTION(NR_NODE_FLOAT, for_FLOAT, nr::Float);
      FUNCTION(NR_NODE_LIST, for_LIST, nr::List);
      FUNCTION(NR_NODE_CALL, for_CALL, nr::Call);
      FUNCTION(NR_NODE_SEQ, for_SEQ, nr::Seq);
      FUNCTION(NR_NODE_INDEX, for_INDEX, nr::Index);
      FUNCTION(NR_NODE_IDENT, for_IDENT, nr::Ident);
      FUNCTION(NR_NODE_EXTERN, for_EXTERN, nr::Extern);
      FUNCTION(NR_NODE_LOCAL, for_LOCAL, nr::Local);
      FUNCTION(NR_NODE_RET, for_RET, nr::Ret);
      FUNCTION(NR_NODE_BRK, for_BRK, nr::Brk);
      FUNCTION(NR_NODE_CONT, for_CONT, nr::Cont);
      FUNCTION(NR_NODE_IF, for_IF, nr::If);
      FUNCTION(NR_NODE_WHILE, for_WHILE, nr::While);
      FUNCTION(NR_NODE_FOR, for_FOR, nr::For);
      FUNCTION(NR_NODE_CASE, for_CASE, nr::Case);
      FUNCTION(NR_NODE_SWITCH, for_SWITCH, nr::Switch);
      FUNCTION(NR_NODE_FN, for_FN, nr::Fn);
      FUNCTION(NR_NODE_ASM, for_ASM, nr::Asm);

      R[NR_NODE_IGN] = [](ctx_t &, craft_t &, State &,
                          const nr::Expr *) -> val_t { return nullptr; };

      R[NR_NODE_TMP] = [](ctx_t &, craft_t &, State &,
                          const nr::Expr *) -> val_t {
        qcore_panic("unexpected temporary node");
      };
    }

#undef FUNCTION

    return R;
  }();

  return dispatch[N->getKind()](m, b, s, N);
}

static auto T_gen(craft_t &b, const nr::Expr *N) -> ty_t {
  static let dispatch = []() constexpr {
#define FUNCTION(_enum, _func, _type)                    \
  R[_enum] = [](craft_t &b, const nr::Expr *N) -> ty_t { \
    return _func(b, N->as<_type>());                     \
  }
    using func_t = ty_t (*)(craft_t &, const nr::Expr *);

    array<func_t, NR_NODE_LAST + 1> R;
    R.fill([](craft_t &, const nr::Expr *n) -> ty_t {
      qcore_panicf("illegal node in input: kind=%s", n->getKindName());
    });

    { /* NRGraph recursive llvm-ir builders */
      using namespace lower::types::prim;
      using namespace lower::types::other;

      FUNCTION(NR_NODE_U1_TY, for_U1_TY, nr::U1Ty);
      FUNCTION(NR_NODE_U8_TY, for_U8_TY, nr::U8Ty);
      FUNCTION(NR_NODE_U16_TY, for_U16_TY, nr::U16Ty);
      FUNCTION(NR_NODE_U32_TY, for_U32_TY, nr::U32Ty);
      FUNCTION(NR_NODE_U64_TY, for_U64_TY, nr::U64Ty);
      FUNCTION(NR_NODE_U128_TY, for_U128_TY, nr::U128Ty);
      FUNCTION(NR_NODE_I8_TY, for_I8_TY, nr::I8Ty);
      FUNCTION(NR_NODE_I16_TY, for_I16_TY, nr::I16Ty);
      FUNCTION(NR_NODE_I32_TY, for_I32_TY, nr::I32Ty);
      FUNCTION(NR_NODE_I64_TY, for_I64_TY, nr::I64Ty);
      FUNCTION(NR_NODE_I128_TY, for_I128_TY, nr::I128Ty);
      FUNCTION(NR_NODE_F16_TY, for_F16_TY, nr::F16Ty);
      FUNCTION(NR_NODE_F32_TY, for_F32_TY, nr::F32Ty);
      FUNCTION(NR_NODE_F64_TY, for_F64_TY, nr::F64Ty);
      FUNCTION(NR_NODE_F128_TY, for_F128_TY, nr::F128Ty);
      FUNCTION(NR_NODE_VOID_TY, for_VOID_TY, nr::VoidTy);
      FUNCTION(NR_NODE_PTR_TY, for_PTR_TY, nr::PtrTy);
      FUNCTION(NR_NODE_OPAQUE_TY, for_OPAQUE_TY, nr::OpaqueTy);
      FUNCTION(NR_NODE_STRUCT_TY, for_STRUCT_TY, nr::StructTy);
      FUNCTION(NR_NODE_UNION_TY, for_UNION_TY, nr::UnionTy);
      FUNCTION(NR_NODE_ARRAY_TY, for_ARRAY_TY, nr::ArrayTy);
      FUNCTION(NR_NODE_FN_TY, for_FN_TY, nr::FnTy);

      R[NR_NODE_IGN] = [](craft_t &, const nr::Expr *) -> ty_t {
        return nullptr;
      };

      R[NR_NODE_TMP] = [](craft_t &, const nr::Expr *) -> ty_t {
        qcore_panic("unexpected temporary node");
      };
    }

#undef FUNCTION

    return R;
  }();

  return dispatch[N->getKind()](b, N);
}

static bool qcode_adapter(qmodule_t *module, qcode_conf_t *conf, FILE *err,
                          FILE *out, qcode_adapter_fn impl) {
  unique_ptr<streambuf> err_stream_buf, out_stream_buf;

  {
    /* If the error stream is provided, use it. Otherwise, discard the output.
     */
    if (err) {
      err_stream_buf = make_unique<OStreamWriter>(err);
    } else {
      err_stream_buf = make_unique<OStreamDiscard>();
    }

    /* If the output stream is provided, use it. Otherwise, discard the output.
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

static optional<unique_ptr<Module>> fabricate_llvmir(const qmodule_t *module,
                                                     qcode_conf_t *conf,
                                                     ostream &err,
                                                     raw_ostream &out);

C_EXPORT bool qcode_ir(qmodule_t *module, qcode_conf_t *conf, FILE *err,
                       FILE *out) {
  return qcode_adapter(module, conf, err, out,
                       [](qmodule_t *m, qcode_conf_t *c, ostream &e,
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

C_EXPORT bool qcode_asm(qmodule_t *module, qcode_conf_t *conf, FILE *err,
                        FILE *out) {
  return qcode_adapter(
      module, conf, err, out,
      [](qmodule_t *m, qcode_conf_t *c, ostream &e,
         raw_pwrite_stream &o) -> bool {
        auto module_opt = fabricate_llvmir(m, c, e, o);
        if (!module_opt) {
          e << "error: failed to fabricate LLVM IR" << endl;
          return false;
        }

        string targetTriple = m->getTargetInfo().TargetTriple.value_or(
            sys::getDefaultTargetTriple());
        string CPU = m->getTargetInfo().CPU.value_or("generic");
        string Features = m->getTargetInfo().CPUFeatures.value_or("");
        bool relocPIC = true;

        TargetOptions opt;
        string lookupTarget_err;
        auto Target =
            TargetRegistry::lookupTarget(targetTriple, lookupTarget_err);
        if (!Target) {
          e << "error: failed to lookup target: " << lookupTarget_err << endl;
          return false;
        }

        auto TargetMachine =
            Target->createTargetMachine(targetTriple, CPU, Features, opt,
                                        relocPIC ? Reloc::PIC_ : Reloc::Static);

        auto &module = *module_opt.value();

        if (verifyModule(module, &o)) {
          e << "error: failed to verify module" << endl;
          return false;
        }

        module.setDataLayout(TargetMachine->createDataLayout());
        module.setTargetTriple(targetTriple);

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

C_EXPORT bool qcode_obj(qmodule_t *module, qcode_conf_t *conf, FILE *err,
                        FILE *out) {
  return qcode_adapter(
      module, conf, err, out,
      [](qmodule_t *m, qcode_conf_t *c, ostream &e,
         raw_pwrite_stream &o) -> bool {
        auto module_opt = fabricate_llvmir(m, c, e, o);
        if (!module_opt) {
          e << "error: failed to fabricate LLVM IR" << endl;
          return false;
        }

        string targetTriple = m->getTargetInfo().TargetTriple.value_or(
            sys::getDefaultTargetTriple());
        string CPU = m->getTargetInfo().CPU.value_or("generic");
        string Features = m->getTargetInfo().CPUFeatures.value_or("");
        bool relocPIC = true;

        TargetOptions opt;
        string lookupTarget_err;
        auto Target =
            TargetRegistry::lookupTarget(targetTriple, lookupTarget_err);
        if (!Target) {
          e << "error: failed to lookup target: " << lookupTarget_err << endl;
          return false;
        }

        auto TargetMachine =
            Target->createTargetMachine(targetTriple, CPU, Features, opt,
                                        relocPIC ? Reloc::PIC_ : Reloc::Static);

        auto &module = *module_opt.value();

        if (verifyModule(module, &o)) {
          e << "error: failed to verify module" << endl;
          return false;
        }

        module.setDataLayout(TargetMachine->createDataLayout());
        module.setTargetTriple(targetTriple);

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
                                           CodeGenFileType::ObjectFile);
        if (!pass.run(module)) {
          e << "error: failed to emit object code" << endl;
          return false;
        }

        return true;
      });
}
