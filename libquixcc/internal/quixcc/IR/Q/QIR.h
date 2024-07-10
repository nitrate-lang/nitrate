////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_IR_QIR_H__
#define __QUIXCC_IR_QIR_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/IR/IRModule.h>
#include <quixcc/IR/Type.h>
#include <quixcc/parsetree/nodes/AllNodes.h>

#include <string_view>
#include <unordered_map>

namespace libquixcc {
namespace ir {
namespace q {
enum class QType {
  Root,

  /* Types */
  I1,
  I8,
  I16,
  I32,
  I64,
  I128,
  U8,
  U16,
  U32,
  U64,
  U128,
  F32,
  F64,
  Void,
  Ptr,
  Array,
  Vector,
  FType,
  Region,
  Union,
  Opaque,

  /* Functions */
  Block,
  Segment,
  Asm,

  /* OO */
  RegionDef,
  GroupDef,
  UnionDef,

  /* Casting */
  SCast,
  UCast,
  PtrICast,
  IPtrCast,
  Bitcast,

  /* Control Flow */
  Call,
  CallIndirect,
  IfElse,
  While,
  For,
  Loop,
  Break,
  Continue,
  Ret,
  Throw,
  TryCatchFinally,
  Case,
  Switch,

  Ident,

  /* Arithmetic */
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  BitAnd,
  BitOr,
  BitXor,
  BitNot,
  Shl,
  Shr,
  Rotl,
  Rotr,

  /* Comparison */
  Eq,
  Ne,
  Lt,
  Gt,
  Le,
  Ge,

  /* Logical */
  And,
  Or,
  Not,
  Xor,

  /* Variables */
  Local,
  Global,
  Number,
  String,
  Char,
  List,

  /* Memory */
  Assign,
  PostInc,
  PostDec,
  AddressOf,
  Deref,
  Member,
  Index,

  /* Intrinsics */
  IntrinsicCall,
  IntrinsicType,

  EnumMax,
};

class Value : public libquixcc::ir::Node<Q> {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override = 0;
  boost::uuids::uuid hash_impl() const override = 0;
  bool verify_impl() const override = 0;

 public:
  virtual ~Value() = default;

  std::string_view ntype_str() const;

  Token m_start;
};

class RootNode : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  RootNode(std::vector<Value *> children) : children(children) {
    ntype = (int)QType::Root;
  }

 public:
  static RootNode *create(std::vector<Value *> children = {});

  std::vector<Value *> children;
};

enum class QPassType {
  Solver,
  Optimizer,
};

enum class IterOp {
  Do,
  Skip,
  Abort,
};

class QModule : public libquixcc::ir::IRModule<IR::Q, RootNode *> {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  std::string_view ir_dialect_name_impl() const override;
  unsigned ir_dialect_version_impl() const override;
  std::string_view ir_dialect_family_impl() const override;
  std::string_view ir_dialect_description_impl() const override;
  bool verify_impl() const override;
  std::unordered_map<QPassType, std::vector<std::string>> m_passes;
  std::set<std::string> m_tags;

 public:
  QModule(const std::string_view &name) : IRModule<IR::Q, RootNode *>(name) {
    m_passes[QPassType::Solver] = {};
    m_passes[QPassType::Optimizer] = {};
  }
  virtual ~QModule() = default;

  bool from_ptree(quixcc_cc_job_t *job, std::shared_ptr<ParseNode> ast);

  /*========================== METADATA ==========================*/
  void acknowledge_pass(QPassType pass, const std::string &name);
  void unacknowledge_pass(QPassType pass, const std::string &name);
  void add_tag(const std::string &tag);
  void remove_tag(const std::string &tag);
  std::set<std::string> &get_tags() { return m_tags; }

  typedef std::function<void(Value **)> IterFunc;
  typedef std::function<IterOp(Value *)> PredFunc;
  typedef std::function<bool(Value *)> MatchFunc;

  /*========================== ITERATION ==========================*/
  void dft_iter(IterFunc func, PredFunc predicate = nullptr,
                bool post_order = false);

  void bft_iter(IterFunc func, PredFunc predicate = nullptr,
                bool post_order = false);

  /*=========================== SEARCH ============================*/
  Value *dfs_find_first(MatchFunc match, PredFunc predicate = nullptr,
                        bool post_order = false);

  Value *bfs_find_first(MatchFunc match, PredFunc predicate = nullptr,
                        bool post_order = false);

  void dfs_find_all(MatchFunc match, std::vector<Value *> &results,
                    PredFunc predicate = nullptr, bool post_order = false);

  void bfs_find_all(MatchFunc match, std::vector<Value *> &results,
                    PredFunc predicate = nullptr, bool post_order = false);

  void dfs_find_all(MatchFunc match, std::vector<Value **> &results,
                    PredFunc predicate = nullptr, bool post_order = false);

  void bfs_find_all(MatchFunc match, std::vector<Value **> &results,
                    PredFunc predicate = nullptr, bool post_order = false);
};
}  // namespace q
}  // namespace ir
}  // namespace libquixcc

#endif  // __QUIXCC_IR_QIR_H__