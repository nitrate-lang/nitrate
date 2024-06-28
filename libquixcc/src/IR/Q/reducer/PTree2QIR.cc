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

#include <IR/Q/Asm.h>
#include <IR/Q/Call.h>
#include <IR/Q/Cast.h>
#include <IR/Q/Control.h>
#include <IR/Q/Function.h>
#include <IR/Q/Ident.h>
#include <IR/Q/Math.h>
#include <IR/Q/Memory.h>
#include <IR/Q/OO.h>
#include <IR/Q/QIR.h>
#include <IR/Q/Type.h>
#include <IR/Q/Variable.h>
#include <core/Logger.h>
#include <mangle/Symbol.h>
#include <openssl/rand.h>

#include <any>
#include <optional>
#include <stack>

using namespace libquixcc;
using namespace ir::q;

typedef Value *QValue;

class QResult {
  std::vector<QValue> m_values;

 public:
  QResult() = default;
  QResult(const QValue &value) { m_values.push_back(value); }
  QResult(const std::vector<QValue> &values) : m_values(values) {}
  QResult(const std::initializer_list<QValue> &values) : m_values(values) {}

  operator bool() const {
    if (m_values.empty()) return false;

    return m_values[0] != nullptr;
  }

  auto operator*() const -> const std::vector<QValue> & { return m_values; }
  auto operator->() const -> const std::vector<QValue> & { return m_values; }
  bool empty() const { return m_values.empty(); }
  QValue operator[](size_t i) const { return m_values.at(i); }

  auto begin() const { return m_values.begin(); }
  auto end() const { return m_values.end(); }
};

struct QState {
  std::map<std::string, std::pair<ExportLangType, QResult>> exported;
  std::stack<std::map<std::string, Type *>> local_idents;
  std::map<std::string, Type *> global_idents;
  bool inside_segment;
  ExportLangType lang;
  std::stack<const FunctionDefNode *> function;
  std::map<std::string, const DefNode *> typedefs;
  std::unordered_map<std::string, Expr *> enum_values;

  QState() {
    inside_segment = false;
    lang = ExportLangType::None;
  }
};

///=============================================================================
/// BEGIN: FORWARD DECLARATIONS
static QResult conv(const ParseNode *n, QState &state);
static QResult conv(const ExprStmtNode *n, QState &state);
static QResult conv(const StmtExprNode *n, QState &state);
static QResult conv(const NopStmtNode *n, QState &state);
static QResult conv(const BlockNode *n, QState &state);
static QResult conv(const StmtGroupNode *n, QState &state);
static QResult conv(const StaticCastExprNode *n, QState &state);
static QResult conv(const BitCastExprNode *n, QState &state);
static QResult conv(const SignedUpcastExprNode *n, QState &state);
static QResult conv(const UnsignedUpcastExprNode *n, QState &state);
static QResult conv(const DowncastExprNode *n, QState &state);
static QResult conv(const UnaryExprNode *n, QState &state);
static QResult conv(const PostUnaryExprNode *n, QState &state);
static QResult conv(const BinaryExprNode *n, QState &state);
static QResult conv(const SeqExprNode *n, QState &state);
static QResult conv(const CallExprNode *n, QState &state);
static QResult conv(const ListExprNode *n, QState &state);
static QResult conv(const MemberAccessNode *n, QState &state);
static QResult conv(const IndexNode *n, QState &state);
static QResult conv(const SliceNode *n, QState &state);
static QResult conv(const FStringNode *n, QState &state);
static QResult conv(const ConstUnaryExprNode *n, QState &state);
static QResult conv(const ConstPostUnaryExprNode *n, QState &state);
static QResult conv(const ConstBinaryExprNode *n, QState &state);
static QResult conv(const IdentifierNode *n, QState &state);
static QResult conv(const MutTypeNode *n, QState &state);
static QResult conv(const U8TypeNode *n, QState &state);
static QResult conv(const U16TypeNode *n, QState &state);
static QResult conv(const U32TypeNode *n, QState &state);
static QResult conv(const U64TypeNode *n, QState &state);
static QResult conv(const U128TypeNode *n, QState &state);
static QResult conv(const I8TypeNode *n, QState &state);
static QResult conv(const I16TypeNode *n, QState &state);
static QResult conv(const I32TypeNode *n, QState &state);
static QResult conv(const I64TypeNode *n, QState &state);
static QResult conv(const I128TypeNode *n, QState &state);
static QResult conv(const F32TypeNode *n, QState &state);
static QResult conv(const F64TypeNode *n, QState &state);
static QResult conv(const BoolTypeNode *n, QState &state);
static QResult conv(const VoidTypeNode *n, QState &state);
static QResult conv(const NullTypeNode *n, QState &state);
static QResult conv(const PointerTypeNode *n, QState &state);
static QResult conv(const OpaqueTypeNode *n, QState &state);
static QResult conv(const StringTypeNode *n, QState &state);
static QResult conv(const EnumTypeNode *n, QState &state);
static QResult conv(const StructTypeNode *n, QState &state);
static QResult conv(const GroupTypeNode *n, QState &state);
static QResult conv(const RegionTypeNode *n, QState &state);
static QResult conv(const UnionTypeNode *n, QState &state);
static QResult conv(const ArrayTypeNode *n, QState &state);
static QResult conv(const VectorTypeNode *n, QState &state);
static QResult conv(const ResultTypeNode *n, QState &state);
static QResult conv(const GeneratorTypeNode *n, QState &state);
static QResult conv(const FunctionTypeNode *n, QState &state);
static QResult conv(const IntegerNode *n, QState &state);
static QResult conv(const FloatLiteralNode *n, QState &state);
static QResult conv(const StringNode *n, QState &state);
static QResult conv(const CharNode *n, QState &state);
static QResult conv(const BoolLiteralNode *n, QState &state);
static QResult conv(const NullLiteralNode *n, QState &state);
static QResult conv(const UndefLiteralNode *n, QState &state);
static QResult conv(const TypedefNode *n, QState &state);
static QResult conv(const VarDeclNode *n, QState &state);
static QResult conv(const LetDeclNode *n, QState &state);
static QResult conv(const ConstDeclNode *n, QState &state);
static QResult conv(const FunctionDeclNode *n, QState &state);
static QResult conv(const StructDefNode *n, QState &state);
static QResult conv(const StructFieldNode *n, QState &state);
static QResult conv(const RegionDefNode *n, QState &state);
static QResult conv(const RegionFieldNode *n, QState &state);
static QResult conv(const GroupDefNode *n, QState &state);
static QResult conv(const GroupFieldNode *n, QState &state);
static QResult conv(const UnionDefNode *n, QState &state);
static QResult conv(const UnionFieldNode *n, QState &state);
static QResult conv(const EnumDefNode *n, QState &state);
static QResult conv(const EnumFieldNode *n, QState &state);
static QResult conv(const FunctionDefNode *n, QState &state);
static QResult conv(const FunctionParamNode *n, QState &state);
static QResult conv(const ExportNode *n, QState &state);
static QResult conv(const InlineAsmNode *n, QState &state);
static QResult conv(const ReturnStmtNode *n, QState &state);
static QResult conv(const RetifStmtNode *n, QState &state);
static QResult conv(const RetzStmtNode *n, QState &state);
static QResult conv(const RetvStmtNode *n, QState &state);
static QResult conv(const BreakStmtNode *n, QState &state);
static QResult conv(const ContinueStmtNode *n, QState &state);
static QResult conv(const IfStmtNode *n, QState &state);
static QResult conv(const WhileStmtNode *n, QState &state);
static QResult conv(const ForStmtNode *n, QState &state);
static QResult conv(const FormStmtNode *n, QState &state);
static QResult conv(const ForeachStmtNode *n, QState &state);
static QResult conv(const CaseStmtNode *n, QState &state);
static QResult conv(const SwitchStmtNode *n, QState &state);
/// END: FORWARD DECLARATIONS
///=============================================================================

///=============================================================================
/// BEGIN: COMMON UTILITIES

/// END: COMMON UTILITIES
///=============================================================================

///=============================================================================
/// BEGIN: CONVERSION FUNCTIONS
static QResult conv(const ExprStmtNode *n, QState &state) {
  /* Function: Convert a parse tree expression-statement adapter
   *           into a QIR expression node.
   *
   * Edge Cases:
   *  - If the body of the adapter is null, return null.
   *
   * General Behavior:
   * - Polymporphically convert the body of the adapter into a QIR
   *   expression node.
   **/

  if (!n->m_expr) {
    return nullptr;
  }

  return conv(n->m_expr.get(), state);
}

static QResult conv(const StmtExprNode *n, QState &state) {
  /* Function: Convert a parse tree statement-expression adapter
   *           into a QIR statement node.
   *
   * Edge Cases:
   *  - If the body of the adapter is null, return null.
   *
   * General Behavior:
   * - Polymporphically convert the body of the adapter into a QIR
   *   statement node.
   **/

  if (!n->m_stmt) {
    return nullptr;
  }

  return conv(n->m_stmt.get(), state);
}

static QResult conv(const NopStmtNode *n, QState &state) {
  /* Function: Disregard a no-operation statement
   *           (compiler internal construct).
   *
   * Edge Cases:
   *
   * General Behavior:
   * - Return null.
   **/

  return nullptr;
}

static QResult conv(const BlockNode *n, QState &state) {
  /* Function: Convert a parse tree block into a QIR block.
   *
   * Edge Cases:
   *  - Any null children in the input shall be skipped.
   *  - Any null children derived from the conversion shall be discarded.
   *
   * General Behavior:
   * - Foreach child in the input, convert the child into a QIR node.
   *   Construct a single block from the resulting QIR nodes.
   **/

  std::vector<QValue> sub;
  sub.reserve(n->m_stmts.size());

  for (const auto &stmt : n->m_stmts) {
    /* Skip null input children */
    if (!stmt) {
      continue;
    }

    /* Convert the child into a QIR node */
    auto res = conv(stmt.get(), state);
    if (!res) {
      continue;
    }

    /* Foreach resulting QIR node, add it to the block */
    for (const auto &val : *res) {
      /* Skip null output children */
      if (!val) {
        continue;
      }

      sub.push_back(val);
    }
  }

  /* Construct a block from the resulting QIR nodes */
  return Block::create(sub);
}

static QResult conv(const StmtGroupNode *n, QState &state) {
  /* Function: Convert a vector of parse tree statements directly
   *           into a QIR list (no intermediate block).
   *
   * Edge Cases:
   *  - Any null children in the input shall be skipped.
   *
   * General Behavior:
   * -  Foreach child in the input, convert the child into a QIR node.
   *    Return the list of QIR children, "AS IS".
   **/

  std::vector<QValue> sub;
  sub.reserve(n->m_stmts.size());

  for (const auto &stmt : n->m_stmts) {
    /* Skip null input children */
    if (!stmt) {
      continue;
    }

    auto res = conv(stmt.get(), state);

    /* Skip null output children */
    if (!res) {
      continue;
    }

    for (const auto &val : *res) {
      /* Skip null output children */
      if (!val) {
        continue;
      }

      sub.push_back(val);
    }
  }

  /* Return the list of QIR children, "AS IS" */
  return sub;
}

static QResult conv(const StaticCastExprNode *n, QState &state) {
  /// TODO: cleanup

  /*
  | Type A    | Type B    | Cast Type     |
  |-----------|-----------|---------------|
  | signed    | unsigned  | signed        |
  | signed    | signed    | signed        |
  | unsigned  | signed    | signed        |
  | unsigned  | unsigned  | unsigned      |
  | pointer   | integer   | ptr-to-int    |
  | integer   | pointer   | int-to-ptr    |
  */

  auto expr = conv(n->m_expr.get(), state)[0]->as<Expr>();
  auto to = conv(n->m_type, state)[0]->as<Type>();

  auto from = expr->infer();
  if (to->is(from)) return expr;

  if (to->is_ptr() && from->is_integer())
    return ir::q::IPtrCast::create(to, expr);
  if (to->is_integer() && from->is_ptr()) return ir::q::PtrICast::create(expr);
  if (from->is_signed() || to->is_signed())
    return ir::q::SCast::create(to, expr);
  if (from->is_unsigned() && to->is_unsigned())
    return ir::q::UCast::create(to, expr);
  if (from->is_float() && to->is_float()) return ir::q::SCast::create(to, expr);

  LOG(FATAL) << "error converting from static_cast to primitive casts"
             << std::endl;
  return nullptr;
}

static QResult conv(const BitCastExprNode *n, QState &state) {
  /* Function: Convert a parse tree bit-cast expression into a QIR bit-cast.
   *
   * Edge Cases:
   *  - If the input expression is null, abort.
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * -  One-for-one lowering of the bit-cast expression.
   **/

  if (!n->m_expr) { /* If the input expression is null, abort */
    LOG(ERROR) << "QIR conv: bit-cast expression has no expression"
               << std::endl;
    return nullptr;
  }

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: bit-cast expression has no type" << std::endl;
    return nullptr;
  }

  auto expr = conv(n->m_expr.get(), state);
  auto to = conv(n->m_type, state);

  if (!expr) { /* If the output expression is null, abort */
    LOG(ERROR) << "QIR conv: QIR bit-cast expression == nullptr" << std::endl;
    return nullptr;
  }

  if (!to) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: QIR bit-cast type == nullptr" << std::endl;
    return nullptr;
  }

  return Bitcast::create(to[0]->as<Type>(), expr[0]->as<Expr>());
}

static QResult conv(const SignedUpcastExprNode *n, QState &state) {
  /* Function: Convert a parse tree s-cast expression into a QIR s-cast.
   *
   * Edge Cases:
   *  - If the input expression is null, abort.
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * -  One-for-one lowering of the s-cast expression.
   **/

  if (!n->m_expr) { /* If the input expression is null, abort */
    LOG(ERROR) << "QIR conv: s-cast expression has no expression" << std::endl;
    return nullptr;
  }

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: s-cast expression has no type" << std::endl;
    return nullptr;
  }

  auto expr = conv(n->m_expr.get(), state);
  auto to = conv(n->m_type, state);

  if (!expr) { /* If the output expression is null, abort */
    LOG(ERROR) << "QIR conv: QIR s-cast expression == nullptr" << std::endl;
    return nullptr;
  }

  if (!to) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: QIR s-cast type == nullptr" << std::endl;
    return nullptr;
  }

  return SCast::create(to[0]->as<Type>(), expr[0]->as<Expr>());
}

static QResult conv(const UnsignedUpcastExprNode *n, QState &state) {
  /* Function: Convert a parse tree u-cast expression into a QIR u-cast.
   *
   * Edge Cases:
   *  - If the input expression is null, abort.
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * -  One-for-one lowering of the u-cast expression.
   **/

  if (!n->m_expr) { /* If the input expression is null, abort */
    LOG(ERROR) << "QIR conv: u-cast expression has no expression" << std::endl;
    return nullptr;
  }

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: u-cast expression has no type" << std::endl;
    return nullptr;
  }

  auto expr = conv(n->m_expr.get(), state);
  auto to = conv(n->m_type, state);

  if (!expr) { /* If the output expression is null, abort */
    LOG(ERROR) << "QIR conv: QIR u-cast expression == nullptr" << std::endl;
    return nullptr;
  }

  if (!to) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: QIR u-cast type == nullptr" << std::endl;
    return nullptr;
  }

  return UCast::create(to[0]->as<Type>(), expr[0]->as<Expr>());
}

static QResult conv(const DowncastExprNode *n, QState &state) {
  /* Function: Convert a parse tree dc-cast expression into a QIR u-cast.
   *
   * Edge Cases:
   *  - If the input expression is null, abort.
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * -  One-for-one lowering of the dc-cast expression.
   **/

  if (!n->m_expr) { /* If the input expression is null, abort */
    LOG(ERROR) << "QIR conv: downcast expression has no expression"
               << std::endl;
    return nullptr;
  }

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: downcast expression has no type" << std::endl;
    return nullptr;
  }

  auto expr = conv(n->m_expr.get(), state);
  auto to = conv(n->m_type, state);

  if (!expr) { /* If the output expression is null, abort */
    LOG(ERROR) << "QIR conv: QIR u-cast (downcast) expression == nullptr"
               << std::endl;
    return nullptr;
  }

  if (!to) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: QIR u-cast (downcast) type == nullptr"
               << std::endl;
    return nullptr;
  }

  return UCast::create(to[0]->as<Type>(), expr[0]->as<Expr>());
}

static QResult conv(const UnaryExprNode *n, QState &state) {
  /// TODO: cleanup

  auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();

  switch (n->m_op) {
    case Operator::Plus:
      return e;
    case Operator::Minus:
      return Sub::create(UCast::create(e->infer(), Number::create("0")), e);
    case Operator::LogicalNot:
      return Not::create(e);
    case Operator::BitwiseNot:
      return BitNot::create(e);
    case Operator::Increment:
      return Assign::create(
          e, Add::create(e, UCast::create(e->infer(), Number::create("1"))));
    case Operator::Decrement:
      return Assign::create(
          e, Sub::create(e, UCast::create(e->infer(), Number::create("1"))));
    case Operator::BitwiseAnd:
      return AddressOf::create(e);
    case Operator::Multiply:
      return Deref::create(e);
    default:
      throw std::runtime_error("QIR UnaryExprNode not implemented");
  }
}

static QResult conv(const PostUnaryExprNode *n, QState &state) {
  /// TODO: cleanup

  auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();

  switch (n->m_op) {
    case Operator::Increment:
      return PostInc::create(e);
    case Operator::Decrement:
      return PostDec::create(e);
    default:
      throw std::runtime_error("QIR PostUnaryExprNode not implemented");
  }
}

static Expr *promote(Type *lht, ir::q::Expr *rhs) {
  /// TODO: cleanup

  auto rht = rhs->infer();

  if (lht->is(rht)) return rhs;

  /* The Intrinsic solver will handle this */
  if (lht->is<IntrinsicType>() || rht->is<IntrinsicType>()) return rhs;

  if (lht->is_void() || rht->is_void())
    throw std::runtime_error("cannot promote void type");

  if (lht->is_integer() && rht->is_ptr()) return ir::q::PtrICast::create(rhs);

  if (lht->is_ptr() && rht->is_integer())
    return ir::q::IPtrCast::create(lht, rhs);

  if (lht->is_float() || rht->is_float()) return ir::q::SCast::create(lht, rhs);

  if (lht->is<I1>() && !rht->is<I1>())
    return ir::q::Ne::create(rhs, ir::q::Number::create("0"));

  if (lht->is_signed() || rht->is_signed())
    return ir::q::SCast::create(lht, rhs);

  if (lht->is_unsigned() || rht->is_unsigned())
    return ir::q::UCast::create(lht, rhs);

  if (lht->is_ptr() && rht->is_ptr()) return ir::q::Bitcast::create(lht, rhs);

  if (lht->is_ptr() && rht->is<Array>()) {
    auto l = lht->as<Ptr>();
    auto r = rht->as<Array>();

    if (!l->type->is(r->type))
      throw std::runtime_error("cannot promote type to pointer");

    return ir::q::AddressOf::create(
        ir::q::Index::create(rhs, ir::q::Number::create("0"), l->type));
  }

  return rhs;

  throw std::runtime_error("cannot promote types");
}

static Expr *promote(ir::q::Expr *lhs, ir::q::Expr *rhs) {
  /// TODO: cleanup
  return promote(lhs->infer(), rhs);
}

static void bipromote(ir::q::Expr **lhs, ir::q::Expr **rhs) {
  /// TODO: cleanup

  auto lht = (*lhs)->infer();
  auto rht = (*rhs)->infer();

  if (!lht || !rht) throw std::runtime_error("failed promote types");

  if (lht->is_ptr()) *lhs = ir::q::PtrICast::create(*lhs);

  if (rht->is_ptr()) *rhs = ir::q::PtrICast::create(*rhs);

  lht = (*lhs)->infer();
  rht = (*rhs)->infer();

  if (lht->is(rht)) return;

  if (lht->is_void() || rht->is_void())
    throw std::runtime_error("cannot promote void type");

  if ((*lhs)->is<Ident>()) {
    *rhs = promote(*lhs, *rhs);
    return;
  } else if ((*rhs)->is<Ident>()) {
    *lhs = promote(*rhs, *lhs);
    return;
  }

  if (lht->is_float() || rht->is_float()) {
    /* We know they are not equal*/
    if (lht->is<F32>())
      *lhs = ir::q::SCast::create(ir::q::F64::create(), *lhs);
    else
      *rhs = ir::q::SCast::create(ir::q::F64::create(), *rhs);

    return;
  }

  if (lht->is_signed() || rht->is_signed()) {
    if (lht->size() < rht->size())
      *lhs = ir::q::SCast::create(rht, *lhs);
    else
      *rhs = ir::q::SCast::create(lht, *rhs);
  } else {
    if (lht->size() < rht->size())
      *lhs = ir::q::UCast::create(rht, *lhs);
    else
      *rhs = ir::q::UCast::create(lht, *rhs);
  }

  return;
}

static QResult conv(const BinaryExprNode *n, QState &state) {
  /// TODO: cleanup

  auto lhs = conv(n->m_lhs.get(), state)[0]->as<Expr>();
  auto rhs = conv(n->m_rhs.get(), state)[0]->as<Expr>();

  switch (n->m_op) {
    case Operator::Plus:
      return bipromote(&lhs, &rhs), Add::create(lhs, rhs);
    case Operator::Minus:
      return bipromote(&lhs, &rhs), Sub::create(lhs, rhs);
    case Operator::Multiply:
      return bipromote(&lhs, &rhs), Mul::create(lhs, rhs);
    case Operator::Divide:
      return bipromote(&lhs, &rhs), Div::create(lhs, rhs);
    case Operator::Modulo:
      return bipromote(&lhs, &rhs), Mod::create(lhs, rhs);
    case Operator::BitwiseAnd:
      return bipromote(&lhs, &rhs), BitAnd::create(lhs, rhs);
    case Operator::BitwiseOr:
      return bipromote(&lhs, &rhs), BitOr::create(lhs, rhs);
    case Operator::BitwiseXor:
      return bipromote(&lhs, &rhs), BitXor::create(lhs, rhs);
    case Operator::LeftShift:
      return bipromote(&lhs, &rhs), Shl::create(lhs, rhs);
    case Operator::RightShift:
      return bipromote(&lhs, &rhs), Shr::create(lhs, rhs);
    case Operator::RotateLeft:
      return bipromote(&lhs, &rhs), Rotl::create(lhs, rhs);
    case Operator::RotateRight:
      return bipromote(&lhs, &rhs), Rotr::create(lhs, rhs);
    case Operator::LogicalAnd:
      return bipromote(&lhs, &rhs), And::create(lhs, rhs);
    case Operator::LogicalOr:
      return bipromote(&lhs, &rhs), Or::create(lhs, rhs);
    case Operator::LogicalXor:
      return bipromote(&lhs, &rhs), Xor::create(lhs, rhs);
    case Operator::LessThan:
      return bipromote(&lhs, &rhs), Lt::create(lhs, rhs);
    case Operator::GreaterThan:
      return bipromote(&lhs, &rhs), Gt::create(lhs, rhs);
    case Operator::LessThanEqual:
      return bipromote(&lhs, &rhs), Le::create(lhs, rhs);
    case Operator::GreaterThanEqual:
      return bipromote(&lhs, &rhs), Ge::create(lhs, rhs);
    case Operator::Equal:
      return bipromote(&lhs, &rhs), Eq::create(lhs, rhs);
    case Operator::NotEqual:
      return bipromote(&lhs, &rhs), Ne::create(lhs, rhs);
    case Operator::Assign:
      return Assign::create(lhs, promote(lhs, rhs));
    case Operator::PlusAssign:
      return Assign::create(lhs, Add::create(lhs, promote(lhs, rhs)));
    case Operator::MinusAssign:
      return Assign::create(lhs, Sub::create(lhs, promote(lhs, rhs)));
    case Operator::MultiplyAssign:
      return Assign::create(lhs, Mul::create(lhs, promote(lhs, rhs)));
    case Operator::DivideAssign:
      return Assign::create(lhs, Div::create(lhs, promote(lhs, rhs)));
    case Operator::ModuloAssign:
      return Assign::create(lhs, Mod::create(lhs, promote(lhs, rhs)));
    case Operator::BitwiseOrAssign:
      return Assign::create(lhs, BitOr::create(lhs, promote(lhs, rhs)));
    case Operator::BitwiseAndAssign:
      return Assign::create(lhs, BitAnd::create(lhs, promote(lhs, rhs)));
    case Operator::BitwiseXorAssign:
      return Assign::create(lhs, BitXor::create(lhs, promote(lhs, rhs)));
    case Operator::XorAssign:
      return Assign::create(lhs, Xor::create(lhs, promote(lhs, rhs)));
    case Operator::OrAssign:
      return Assign::create(lhs, Or::create(lhs, promote(lhs, rhs)));
    case Operator::AndAssign:
      return Assign::create(lhs, And::create(lhs, promote(lhs, rhs)));
    case Operator::LeftShiftAssign:
      return Assign::create(lhs, Shl::create(lhs, promote(lhs, rhs)));
    case Operator::RightShiftAssign:
      return Assign::create(lhs, Shr::create(lhs, promote(lhs, rhs)));

    default:
      throw std::runtime_error("QIR BinaryExprNode not implemented");
  }
}

static QResult conv(const SeqExprNode *n, QState &state) {
  throw std::runtime_error("SeqExprNode not implemented");
}

static QResult conv(const CallExprNode *n, QState &state) {
  /// TODO: cleanup

  Global *callee = nullptr;

  if (state.exported.contains(n->m_decl->m_name))
    callee = state.exported[n->m_decl->m_name].second[0]->as<Global>();
  else
    callee = conv(n->m_decl.get(), state)[0]->as<Global>();

  std::vector<Expr *> args;
  Segment *seg = callee->value->as<Segment>();

  size_t i = 0;
  for (auto &arg : n->m_positional_args) {
    auto v = conv(arg.get(), state)[0]->as<Expr>();

    if (seg->params.size() <= i || !seg->params.at(i).second)
      args.push_back(v);
    else
      args.push_back(promote(seg->params[i].second, v));

    i++;
  }

  while (i < callee->value->as<Segment>()->params.size()) {
    auto v =
        conv(n->m_decl->m_params.at(i)->m_value.get(), state)[0]->as<Expr>();
    if (seg->params[i].second)
      args.push_back(promote(seg->params[i].second, v));
    else
      args.push_back(v);
    i++;
  }

  return Call::create(callee, args);
}

static QResult conv(const ListExprNode *n, QState &state) {
  /* Function: Convert a parse tree list expression into a QIR list.
   *
   * Edge Cases:
   *  - If the input list is empty, return an empty list.
   *  - If any element in the input list is null, abort.
   *  - If any element in the output list is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the list expression.
   **/

  std::vector<Expr *> values;
  values.reserve(n->m_elements.size());

  for (const auto &elem : n->m_elements) {
    if (!elem) { /* If any element in the input list is null, abort */
      LOG(ERROR) << "QIR conv: list expression has null element" << std::endl;
      return nullptr;
    }

    auto res = conv(elem.get(), state);
    if (!res) { /* If any element in the output list is null, abort */
      LOG(ERROR) << "QIR conv: list expression has null element" << std::endl;
      return nullptr;
    }

    values.push_back(res[0]->as<Expr>());
  }

  /* One-for-one lowering of the list expression */
  return List::create(values);
}

static QResult conv(const MemberAccessNode *n, QState &state) {
  /// TODO: cleanup

  auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();
  auto t = e->infer()->as<Type>();

  while (t->is_ptr()) {
    t = t->as<Ptr>()->type;
  }

  switch ((ir::q::QType)t->ntype) {
    case ir::q::QType::Region: {
      auto x = t->as<Region>();

      if (!state.typedefs.contains(x->name))
        throw std::runtime_error("QIR translation: MemberAccessNode not found");

      if (state.typedefs[x->name]->is<RegionDefNode>()) {
        auto def = state.typedefs[x->name]->as<RegionDefNode>();
        for (size_t i = 0; i < def->m_fields.size(); i++) {
          if (def->m_fields[i]->m_name == n->m_field) {
            auto t = conv(def->m_fields[i]->m_type, state)[0]->as<Type>();
            return Member::create(e, i, t);
          }
        }
      } else {
        auto def = state.typedefs[x->name]->as<StructDefNode>();
        for (size_t i = 0; i < def->m_fields.size(); i++) {
          if (def->m_fields[i]->m_name == n->m_field) {
            auto t = conv(def->m_fields[i]->m_type, state)[0]->as<Type>();
            return Member::create(e, i, t);
          }
        }
      }

      throw std::runtime_error("QIR translation: MemberAccessNode not found");
    }
    case ir::q::QType::Union: {
      auto x = t->as<Union>();

      if (!state.typedefs.contains(x->name))
        throw std::runtime_error("QIR translation: MemberAccessNode not found");

      auto def = state.typedefs[x->name]->as<UnionDefNode>();
      for (size_t i = 0; i < def->m_fields.size(); i++) {
        if (def->m_fields[i]->m_name == n->m_field) {
          auto t = conv(def->m_fields[i]->m_type, state)[0]->as<Type>();
          return Member::create(e, i, t);
        }
      }

      throw std::runtime_error("QIR translation: MemberAccessNode not found");
    }
    default:
      throw std::runtime_error("MemberAccessNode not implemented");
  }
}

static QResult conv(const IndexNode *n, QState &state) {
  /// TODO: cleanup

  auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();
  auto i = conv(n->m_index.get(), state)[0]->as<Expr>();
  auto t = e->infer()->as<Type>();

  if (t->is_ptr()) return Index::create(e, i, t->as<Ptr>()->type);

  return Index::create(e, i, t->as<Array>()->type);
}

static QResult conv(const SliceNode *n, QState &state) {
  /// TODO: cleanup

  /// TODO: Implement SliceNode
  throw std::runtime_error("SliceNode not implemented");
}

static QResult conv(const FStringNode *n, QState &state) {
  /// TODO: cleanup

  enum State {
    init,
    check,
  } fstate = init;

  if (n->template_string.empty()) {
    return String::create("");
  }

  std::vector<String *> literal_parts;
  std::string literal;

  for (char c : n->template_string) {
    switch (fstate) {
      case init: {
        if (c == '{') {
          fstate = check;
          break;
        }

        literal += c;
        break;
      }
      case check: {
        if (c == '{') {
          literal += '{';
          break;
        }

        literal_parts.push_back(String::create(literal));
        literal.clear();
        fstate = init;
        break;
      }
    }
  }

  if (!literal.empty()) {
    literal_parts.push_back(String::create(literal));
  }

  if (literal_parts.empty()) {
    return String::create("");
  }

  Expr *result = literal_parts[0];

  for (auto arg : n->args) {
    auto val = conv(arg.get(), state)[0]->as<Expr>();
    auto stringified = IntrinsicCall::create(QIntrinsic::ToString, {val});

    result = Add::create(result, stringified);
  }

  return result;
}

static QResult conv(const ConstUnaryExprNode *n, QState &state) {
  /// TODO: cleanup

  auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();

  switch (n->m_op) {
    case Operator::Plus:
      return e;
    case Operator::Minus:
      return Sub::create(Number::create("0"), e);
    case Operator::LogicalNot:
      return Not::create(e);
    case Operator::BitwiseNot:
      return BitNot::create(e);
    default:
      throw std::runtime_error("QIR ConstUnaryExprNode not implemented");
  }
}

static QResult conv(const ConstPostUnaryExprNode *n, QState &state) {
  /// TODO: cleanup

  auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();

  switch (n->m_op) {
    case Operator::Increment:
      return PostInc::create(e);
    case Operator::Decrement:
      return PostDec::create(e);
    default:
      throw std::runtime_error("QIR ConstPostUnaryExprNode not implemented");
  }
}

static QResult conv(const ConstBinaryExprNode *n, QState &state) {
  /// TODO: cleanup

  auto lhs = conv(n->m_lhs.get(), state)[0]->as<Expr>();
  auto rhs = conv(n->m_rhs.get(), state)[0]->as<Expr>();

  switch (n->m_op) {
    case Operator::Plus:
      return bipromote(&lhs, &rhs), Add::create(lhs, rhs);
    case Operator::Minus:
      return bipromote(&lhs, &rhs), Sub::create(lhs, rhs);
    case Operator::Multiply:
      return bipromote(&lhs, &rhs), Mul::create(lhs, rhs);
    case Operator::Divide:
      return bipromote(&lhs, &rhs), Div::create(lhs, rhs);
    case Operator::Modulo:
      return bipromote(&lhs, &rhs), Mod::create(lhs, rhs);
    case Operator::BitwiseAnd:
      return bipromote(&lhs, &rhs), BitAnd::create(lhs, rhs);
    case Operator::BitwiseOr:
      return bipromote(&lhs, &rhs), BitOr::create(lhs, rhs);
    case Operator::BitwiseXor:
      return bipromote(&lhs, &rhs), BitXor::create(lhs, rhs);
    case Operator::LeftShift:
      return bipromote(&lhs, &rhs), Shl::create(lhs, rhs);
    case Operator::RightShift:
      return bipromote(&lhs, &rhs), Shr::create(lhs, rhs);
    case Operator::LogicalAnd:
      return bipromote(&lhs, &rhs), And::create(lhs, rhs);
    case Operator::LogicalOr:
      return bipromote(&lhs, &rhs), Or::create(lhs, rhs);
    case Operator::LogicalXor:
      return Xor::create(lhs, rhs);
    case Operator::LessThan:
      return bipromote(&lhs, &rhs), Lt::create(lhs, rhs);
    case Operator::GreaterThan:
      return bipromote(&lhs, &rhs), Gt::create(lhs, rhs);
    case Operator::LessThanEqual:
      return bipromote(&lhs, &rhs), Le::create(lhs, rhs);
    case Operator::GreaterThanEqual:
      return bipromote(&lhs, &rhs), Ge::create(lhs, rhs);
    case Operator::Equal:
      return bipromote(&lhs, &rhs), Eq::create(lhs, rhs);
    case Operator::NotEqual:
      return bipromote(&lhs, &rhs), Ne::create(lhs, rhs);

    default:
      throw std::runtime_error("QIR ConstBinaryExprNode not implemented");
  }
}

static QResult conv(const IdentifierNode *n, QState &state) {
  /// TODO: cleanup

  if (state.inside_segment) {
    if (state.local_idents.top().contains(n->m_name))
      return Ident::create(n->m_name, state.local_idents.top()[n->m_name]);
  }

  if (state.enum_values.contains(n->m_name)) {
    return state.enum_values[n->m_name];
  }

  if (!state.global_idents.contains(n->m_name)) {
    throw std::runtime_error("QIR translation: IdentifierNode not found: " +
                             n->m_name);
  }

  return Ident::create(n->m_name, state.global_idents[n->m_name]);
}

static QResult conv(const MutTypeNode *n, QState &state) {
  /* Function: Convert a parse tree mutable type node into a QIR type.
   *
   * Edge Cases:
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * - Disregard mutability, it only has semantic significance.
   **/

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: mutable type has no type" << std::endl;
    return nullptr;
  }

  auto t = conv(n->m_type, state);

  if (!t) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: mutable type == nullptr" << std::endl;
    return nullptr;
  }

  return t[0]->as<Type>();
}

static QResult conv(const U8TypeNode *n, QState &state) {
  /* Function: Convert a parse tree U8 type node into a QIR U8 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the U8 type node.
   **/

  return U8::create();
}

static QResult conv(const U16TypeNode *n, QState &state) {
  /* Function: Convert a parse tree U16 type node into a QIR U16 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the U16 type node.
   **/

  return U16::create();
}

static QResult conv(const U32TypeNode *n, QState &state) {
  /* Function: Convert a parse tree U32 type node into a QIR U32 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the U32 type node.
   **/

  return U32::create();
}

static QResult conv(const U64TypeNode *n, QState &state) {
  /* Function: Convert a parse tree U64 type node into a QIR U64 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the U64 type node.
   **/

  return U64::create();
}

static QResult conv(const U128TypeNode *n, QState &state) {
  /* Function: Convert a parse tree U128 type node into a QIR U128 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the U128 type node.
   **/

  return U128::create();
}

static QResult conv(const I8TypeNode *n, QState &state) {
  /* Function: Convert a parse tree I8 type node into a QIR I8 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the I8 type node.
   **/

  return I8::create();
}

static QResult conv(const I16TypeNode *n, QState &state) {
  /* Function: Convert a parse tree I16 type node into a QIR I16 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the I16 type node.
   **/

  return I16::create();
}

static QResult conv(const I32TypeNode *n, QState &state) {
  /* Function: Convert a parse tree I32 type node into a QIR I32 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the I32 type node.
   **/

  return I32::create();
}

static QResult conv(const I64TypeNode *n, QState &state) {
  /* Function: Convert a parse tree I64 type node into a QIR I64 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the I64 type node.
   **/

  return I64::create();
}

static QResult conv(const I128TypeNode *n, QState &state) {
  /* Function: Convert a parse tree I128 type node into a QIR I128 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the I128 type node.
   **/

  return I128::create();
}

static QResult conv(const F32TypeNode *n, QState &state) {
  /* Function: Convert a parse tree F32 type node into a QIR F32 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the F32 type node.
   **/

  return F32::create();
}

static QResult conv(const F64TypeNode *n, QState &state) {
  /* Function: Convert a parse tree F64 type node into a QIR F64 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the F64 type node.
   **/

  return F64::create();
}

static QResult conv(const BoolTypeNode *n, QState &state) {
  /* Function: Convert a parse tree bool type node into a QIR i1 type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the bool type node.
   **/

  return I1::create();
}

static QResult conv(const VoidTypeNode *n, QState &state) {
  /* Function: Convert a parse tree void type node into a QIR void type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the void type node.
   **/

  return Void::create();
}

static QResult conv(const NullTypeNode *n, QState &state) {
  /* Function: Convert a parse tree null type node into a QIR void type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - The NullTypeNode is used to denote the concept of `noreturn`.
   *   In reality, this is a void type with special semantics.
   **/

  return Void::create();
}

static QResult conv(const PointerTypeNode *n, QState &state) {
  /* Function: Convert a parse tree pointer type node into a QIR pointer type.
   *
   * Edge Cases:
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the pointer type node.
   **/

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: pointer type has no type" << std::endl;
    return nullptr;
  }

  auto t = conv(n->m_type, state);
  if (!t) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: pointer type == nullptr" << std::endl;
    return nullptr;
  }

  return Ptr::create(t[0]->as<Type>());
}

static QResult conv(const OpaqueTypeNode *n, QState &state) {
  /* Function: Convert a parse tree opaque type node into a QIR opaque type.
   *
   * Edge Cases:
   *  - If the name is empty, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the opaque type node.
   **/

  if (n->m_name.empty()) { /* If the name is empty, abort */
    LOG(ERROR) << "QIR conv: opaque type has no name" << std::endl;
    return nullptr;
  }

  return Opaque::create(n->m_name);
}

static QResult conv(const StringTypeNode *n, QState &state) {
  /* Function: Convert a parse tree string type node into a QIR string type.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - Convert into QIR Intrinsic[String]
   **/

  return IntrinsicType::create(QIntrinsicType::String);
}

static QResult conv(const EnumTypeNode *n, QState &state) {
  /* Function: Convert a parse tree enum type node into underlying QIR type.
   *
   * Edge Cases:
   *  - If the input member_type is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the enum type node.
   **/

  if (!n->m_member_type) { /* If the input member_type is null, abort */
    LOG(ERROR) << "QIR conv: enum type has no member type" << std::endl;
    return nullptr;
  }

  auto t = conv(n->m_member_type, state);
  if (!t) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: enum type == nullptr" << std::endl;
    return nullptr;
  }

  return t[0]->as<Type>();
}

static QResult conv(const StructTypeNode *n, QState &state) {
  /* Function: Convert a parse tree struct type node into a QIR region
   *           (packed) type.
   *
   * Edge Cases:
   *  - If any of the input fields are null, abort.
   *  - If any of the QIR fields are null, abort.
   *
   * General Behavior:
   * -
   **/

  std::vector<Type *> fields;
  fields.reserve(n->m_fields.size());

  for (const auto &field : n->m_fields) {
    if (!field) { /* If the input field is null, abort */
      LOG(ERROR) << "QIR conv: struct field is null" << std::endl;
      return nullptr;
    }

    auto ft = conv(field, state);
    if (!ft) { /* If the output field is null, abort */
      LOG(ERROR) << "QIR conv: struct field == nullptr" << std::endl;
      return nullptr;
    }

    fields.push_back(ft[0]->as<Type>());
  }

  return Region::create(n->m_name, fields, false, true);
}

static QResult conv(const GroupTypeNode *n, QState &state) {
  /* Function: Convert a parse tree group type node into a QIR group type.
   *
   * Edge Cases:
   *  - If any of the input fields are null, abort.
   *  - If any of the QIR fields are null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the group type node.
   **/

  std::vector<Type *> fields;
  fields.reserve(n->m_fields.size());

  for (const auto &field : n->m_fields) {
    if (!field) { /* If the input field is null, abort */
      LOG(ERROR) << "QIR conv: group field is null" << std::endl;
      return nullptr;
    }

    auto ft = conv(field, state);
    if (!ft) { /* If the output field is null, abort */
      LOG(ERROR) << "QIR conv: group field == nullptr" << std::endl;
      return nullptr;
    }

    fields.push_back(ft[0]->as<Type>());
  }

  return Region::create(n->m_name, fields, false, false);
}

static QResult conv(const RegionTypeNode *n, QState &state) {
  /* Function: Convert a parse tree region type node into a QIR region
   *           (packed) type.
   *
   * Edge Cases:
   *  - If any of the input fields are null, abort.
   *  - If any of the QIR fields are null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the region type node.
   **/

  std::vector<Type *> fields;
  fields.reserve(n->m_fields.size());

  for (const auto &field : n->m_fields) {
    if (!field) { /* If the input field is null, abort */
      LOG(ERROR) << "QIR conv: region field is null" << std::endl;
      return nullptr;
    }

    auto ft = conv(field, state);
    if (!ft) { /* If the output field is null, abort */
      LOG(ERROR) << "QIR conv: region field == nullptr" << std::endl;
      return nullptr;
    }

    fields.push_back(ft[0]->as<Type>());
  }

  return Region::create(n->m_name, fields, true, true);
}

static QResult conv(const UnionTypeNode *n, QState &state) {
  /* Function: Convert a parse tree union type node into a QIR union type.
   *
   * Edge Cases:
   *  - If any of the input fields are null, abort.
   * - If any of the QIR fields are null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the union type node.
   **/

  std::vector<Type *> fields;
  fields.reserve(n->m_fields.size());

  for (const auto &field : n->m_fields) {
    if (!field) { /* If the input field is null, abort */
      LOG(ERROR) << "QIR conv: union field is null" << std::endl;
      return nullptr;
    }

    auto ft = conv(field, state);
    if (!ft) { /* If the output field is null, abort */
      LOG(ERROR) << "QIR conv: union field == nullptr" << std::endl;
      return nullptr;
    }

    fields.push_back(ft[0]->as<Type>());
  }

  return Union::create(n->m_name, fields);
}

static QResult conv(const ArrayTypeNode *n, QState &state) {
  /* Function: Convert a parse tree array type node into a QIR array type.
   *
   * Edge Cases:
   *  - If the input size is null, abort.
   *  - If the size is not an integer, abort.
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the array type node.
   **/

  if (!n->m_size) { /* If the input size is null, abort */
    LOG(ERROR) << "QIR conv: array type has no size" << std::endl;
    return nullptr;
  }

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: array type has no type" << std::endl;
    return nullptr;
  }

  auto s = conv(n->m_size.get(), state);
  if (!s) { /* If the output size is null, abort */
    LOG(ERROR) << "QIR conv: array size == nullptr" << std::endl;
    return nullptr;
  }

  auto t = conv(n->m_type, state);
  if (!t) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: array type == nullptr" << std::endl;
    return nullptr;
  }

  const auto &size_str = s[0]->as<Number>()->value;

  if (!std::all_of(size_str.begin(), size_str.end(), ::isdigit)) {
    LOG(ERROR) << "QIR conv: array size is not an integer" << std::endl;
    return nullptr;
  }

  uint64_t size;
  try {
    size = std::stoull(size_str);
  } catch (const std::out_of_range &e) {
    LOG(ERROR) << "QIR conv: array size is out of range" << std::endl;
    return nullptr;
  }

  return Array::create(t[0]->as<Type>(), size);
}

static QResult conv(const VectorTypeNode *n, QState &state) {
  /* Function: Convert a parse tree vector type node into a QIR vector type.
   *
   * Edge Cases:
   *  - If the input type is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the vector type node.
   **/

  if (!n->m_type) { /* If the input type is null, abort */
    LOG(ERROR) << "QIR conv: vector type has no type" << std::endl;
    return nullptr;
  }

  auto t = conv(n->m_type, state);
  if (!t) { /* If the output type is null, abort */
    LOG(ERROR) << "QIR conv: vector type == nullptr" << std::endl;
    return nullptr;
  }

  return Vector::create(t[0]->as<Type>());
}

static QResult conv(const ResultTypeNode *n, QState &state) {
  /// TODO: cleanup

  LOG(FATAL) << "ResultTypeNode not implemented" << std::endl;

  return nullptr;
}

static QResult conv(const GeneratorTypeNode *n, QState &state) {
  /// TODO: cleanup

  LOG(FATAL) << "GeneratorTypeNode not implemented" << std::endl;

  return nullptr;
}

static QResult conv(const FunctionTypeNode *n, QState &state) {
  /// TODO: cleanup

  std::vector<Type *> params;
  for (auto &param : n->m_params)
    params.push_back(conv(param.second, state)[0]->as<Type>());

  return FType::create(params, conv(n->m_return_type, state)[0]->as<Type>(),
                       n->m_variadic, n->m_pure, n->m_thread_safe, n->m_foreign,
                       n->m_noexcept);
}

static QResult conv(const IntegerNode *n, QState &state) {
  /* Function: Convert a parse tree integer literal node into a QIR integer
   *           literal.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the integer literal node.
   **/

  return Number::create(n->m_val);
}

static QResult conv(const FloatLiteralNode *n, QState &state) {
  /* Function: Convert a parse tree float literal node into a QIR float
   *           literal.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the float literal node.
   **/

  return Number::create(n->m_val);
}

static QResult conv(const StringNode *n, QState &state) {
  /* Function: Convert a parse tree string literal node into a QIR string
   *           literal.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the string literal node.
   **/

  return String::create(n->m_val);
}

static QResult conv(const CharNode *n, QState &state) {
  /* Function: Convert a parse tree char literal node into a QIR char literal.
   *
   * Edge Cases:
   *  - CharNode is UTF-???.
   *
   * General Behavior:
   * - One-for-one lowering of the char literal node.
   **/

  /// TODO: Decree the size of the char type

  return Char::create(n->m_val);
}

static QResult conv(const BoolLiteralNode *n, QState &state) {
  /* Function: Convert a parse tree boolean literal node into a QIR boolean
   *           literal.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the boolean literal node.
   **/

  if (n->m_val == true) {
    return UCast::create(I1::create(), Number::create("1"));
  }

  return UCast::create(I1::create(), Number::create("0"));
}

static QResult conv(const NullLiteralNode *n, QState &state) {
  /* Function: Convert a parse tree null literal node into a QIR intrinsic null.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - Convert into intrinsic null.
   **/

  return UCast::create(IntrinsicType::create(QIntrinsicType::Null),
                       Number::create("0"));
}

/// WARNING: BEGIN CHATGPT-3.5 GENERATED CODE
typedef __uint128_t uint128_t;

std::string uint128ToString(uint128_t value) {
  if (value == 0) {
    return "0";
  }

  std::stringstream ss;
  std::string result;

  while (value > 0) {
    uint64_t part = value % 10;
    result = std::to_string(part) + result;
    value /= 10;
  }

  return result;
}
/// WARNING: END CHATGPT-3.5 GENERATED CODE

static QResult conv(const UndefLiteralNode *n, QState &state) {
  /* Function: Convert a parse tree undefined literal into random unsigned
   * 128-bit integer literal.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - Generate a cryptographically secure random number.
   **/

  union {
    __uint128_t i;
    uint8_t b[16];
  } __attribute__((packed)) b;

  if (RAND_bytes(b.b, sizeof(b.b)) != 1) {
    LOG(ERROR) << "QIR conv: RAND_bytes failed" << std::endl;
    return nullptr;
  }

  /* Convert to decimal */
  std::string val = uint128ToString(b.i);

  return Number::create(val);
}

static QResult conv(const TypedefNode *n, QState &state) {
  /* Function: Disregard typedefs.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - Disregard typedefs, they have already been processed.
   **/

  return nullptr;
}

static QResult conv(const VarDeclNode *n, QState &state) {
  /// TODO: cleanup

  /// TODO: Implement VarDeclNode
  throw std::runtime_error("QIR translation: VarDeclNode not implemented");
}

static QResult create_defaults(ir::q::Value *var, TypeNode *type,
                               QState &state) {
  /// TODO: cleanup

  switch (type->ntype) {
    case NodeType::StructTypeNode: {
      auto s = type->as<StructTypeNode>();
      if (!state.typedefs.contains(s->m_name))
        throw std::runtime_error("QIR translation: structdef not found");

      std::vector<QValue> res;
      for (size_t i = 0;
           i < state.typedefs[s->m_name]->as<StructDefNode>()->m_fields.size();
           i++) {
        auto field =
            state.typedefs[s->m_name]->as<StructDefNode>()->m_fields[i];
        if (!field->m_value) continue;

        auto v = conv(field->m_value.get(), state)[0]->as<Expr>();
        auto t = conv(field->m_type, state)[0]->as<Type>();

        res.push_back(Assign::create(Member::create(var, i, t), promote(t, v)));
      }

      return res;
    }
    case NodeType::RegionTypeNode: {
      auto r = type->as<RegionTypeNode>();
      if (!state.typedefs.contains(r->m_name))
        throw std::runtime_error("QIR translation: regiondef not found");

      std::vector<QValue> res;
      for (size_t i = 0;
           i < state.typedefs[r->m_name]->as<RegionDefNode>()->m_fields.size();
           i++) {
        auto field =
            state.typedefs[r->m_name]->as<RegionDefNode>()->m_fields[i];
        if (!field->m_value) continue;

        auto v = conv(field->m_value.get(), state)[0]->as<Expr>();
        auto t = conv(field->m_type, state)[0]->as<Type>();

        res.push_back(Assign::create(Member::create(var, i, t), promote(t, v)));
      }

      return res;
    }
    case NodeType::UnionTypeNode: {
      /// TODO: think about this one

      auto u = type->as<UnionTypeNode>();
      if (!state.typedefs.contains(u->m_name))
        throw std::runtime_error("QIR translation: uniondef not found");

      std::vector<QValue> res;
      for (size_t i = 0;
           i < state.typedefs[u->m_name]->as<UnionDefNode>()->m_fields.size();
           i++) {
        auto field = state.typedefs[u->m_name]->as<UnionDefNode>()->m_fields[i];
        if (!field->m_value) continue;

        auto v = conv(field->m_value.get(), state)[0]->as<Expr>();
        auto t = conv(field->m_type, state)[0]->as<Type>();

        res.push_back(Assign::create(Member::create(var, i, t), promote(t, v)));
      }

      return res;
    }
    default:
      throw std::runtime_error(
          "QIR translation: create_defaults not implemented");
  }
}

static bool is_composite(const TypeNode *n) {
  /// TODO: cleanup

  switch (n->ntype) {
    case NodeType::StructTypeNode:
    case NodeType::GroupTypeNode:
    case NodeType::RegionTypeNode:
    case NodeType::UnionTypeNode:
      return true;
    default:
      return false;
  }
}

static QResult conv(const LetDeclNode *n, QState &state) {
  /// TODO: cleanup

  if (state.inside_segment) {
    Expr *init = nullptr;

    if (n->m_init) init = conv(n->m_init.get(), state)[0]->as<Expr>();

    Type *type = nullptr;
    if (n->m_type)
      type = conv(n->m_type, state)[0]->as<Type>();
    else if (init)
      type = init->infer();
    else
      throw std::runtime_error("QIR translation: LetDeclNode is malformed");

    auto l = Local::create(n->m_name, type, init);
    state.local_idents.top()[n->m_name] = l->type;
    std::vector<QValue> res = {l};
    auto ident = Ident::create(n->m_name, l->type);

    if (l->type == nullptr && n->m_type && is_composite(n->m_type)) {
      auto defaults = create_defaults(ident, n->m_type, state);
      for (auto &d : defaults) res.push_back(d);
    }

    return res;
  }

  Expr *expr = nullptr;
  if (n->m_init) expr = conv(n->m_init.get(), state)[0]->as<Expr>();

  auto tmp = Global::create(n->m_name, conv(n->m_type, state)[0]->as<Type>(),
                            expr, false, false, false);

  std::string mangled = Symbol::mangle(tmp, "",
                                       state.lang == ExportLangType::None
                                           ? ExportLangType::Default
                                           : state.lang);

  auto g = Global::create(mangled, conv(n->m_type, state)[0]->as<Type>(), expr,
                          false, false, state.lang != ExportLangType::None);

  state.global_idents[n->m_name] = g->type;
  state.global_idents[mangled] = g->type;
  state.exported[n->m_name] = {state.lang, g};
  return g;
}

static QResult conv(const ConstDeclNode *n, QState &state) {
  /// TODO: cleanup

  if (state.inside_segment) {
    auto init = conv(n->m_init.get(), state)[0]->as<Expr>();

    Type *type = nullptr;
    if (n->m_type)
      type = conv(n->m_type, state)[0]->as<Type>();
    else
      type = init->infer();

    auto l = Local::create(n->m_name, type, init);
    state.local_idents.top()[n->m_name] = l->type;

    return l;
  }

  Expr *expr = nullptr;
  if (n->m_init) expr = conv(n->m_init.get(), state)[0]->as<Expr>();

  Type *type = nullptr;
  if (n->m_type)
    type = conv(n->m_type, state)[0]->as<Type>();
  else
    type = expr->infer();

  auto tmp = Global::create(n->m_name, type, expr, false, false,
                            state.lang != ExportLangType::None);
  std::string mangled = Symbol::mangle(tmp, "",
                                       state.lang == ExportLangType::None
                                           ? ExportLangType::Default
                                           : state.lang);
  auto g = Global::create(mangled, type, expr, false, false,
                          state.lang != ExportLangType::None);

  state.global_idents[n->m_name] = g->type;
  state.global_idents[mangled] = g->type;
  state.exported[n->m_name] = {state.lang, g};
  return g;
}

static QResult conv(const FunctionDeclNode *n, QState &state) {
  /// TODO: cleanup

  std::vector<std::pair<std::string, Type *>> params;
  for (auto &p : n->m_params) {
    auto res = conv(p.get(), state);
    auto t = res[0]->as<Type>();

    if (t->is<Array>())
      params.push_back({p->m_name, Ptr::create(t->as<Array>()->type)});
    else
      params.push_back({p->m_name, t});
  }

  auto seg = Segment::create(
      params, conv(n->m_type->m_return_type, state)[0]->as<Type>(), nullptr,
      n->m_type->m_variadic, n->m_type->m_pure, n->m_type->m_thread_safe,
      n->m_type->m_noexcept, n->m_type->m_return_type->is<NullTypeNode>(),
      n->m_type->m_foreign);

  Global *g = nullptr;
  std::string mangled;

  g = Global::create(n->m_name, seg->infer(), seg, false, false, true);

  if (n->m_name == "main") {
    g = Global::create(mangled = Symbol::mangle(g, "", ExportLangType::C),
                       seg->infer(), seg, false, false, true);
  } else {
    if (state.lang == ExportLangType::None) {
      g = Global::create(
          mangled = Symbol::mangle(g, "", ExportLangType::Default),
          seg->infer(), seg, false, false, false);
    } else {
      g = Global::create(mangled = Symbol::mangle(g, "", state.lang),
                         seg->infer(), seg, false, false, true);
    }
  }

  state.global_idents[n->m_name] = g->type;
  state.global_idents[mangled] = g->type;
  state.exported[n->m_name] = {state.lang, g};
  return g;
}

static QResult conv(const StructDefNode *n, QState &state) {
  /// TODO: cleanup

  std::vector<std::pair<std::string, QValue>> fields;
  std::map<std::string, Segment *> methods;

  state.typedefs[n->m_name] = n;
  for (auto &field : n->m_fields) {
    auto res = conv(field.get(), state);
    fields.push_back({field->m_name, res[0]});
  }

  for (auto &method : n->m_methods) {
    auto res = conv(method.get(), state)[0]->as<Global>();
    methods[res->name] = res->value->as<Segment>();
  }

  std::vector<QValue> result;
  result.push_back(RegionDef::create(n->m_name, fields, methods));

  for (auto static_method : n->m_static_methods) {
    std::shared_ptr<StmtNode> clone;
    if (static_method->is<FunctionDeclNode>()) {
      clone = std::make_shared<FunctionDeclNode>(
          *static_method->as<FunctionDeclNode>());
      auto x = std::static_pointer_cast<FunctionDeclNode>(clone);
      x->m_name = n->m_name + "::" + x->m_name;
    } else if (static_method->is<FunctionDefNode>()) {
      clone = std::make_shared<FunctionDefNode>(
          *static_method->as<FunctionDefNode>());
      auto x = std::static_pointer_cast<FunctionDefNode>(clone);
      x->m_decl->m_name = n->m_name + "::" + x->m_decl->m_name;
    }

    auto res = conv(clone.get(), state);
    result.insert(result.end(), res.begin(), res.end());
  }

  return result;
}

static QResult conv(const StructFieldNode *n, QState &state) {
  /// TODO: cleanup

  return conv(n->m_type, state)[0]->as<Type>();
}

static QResult conv(const RegionDefNode *n, QState &state) {
  /// TODO: cleanup

  std::vector<std::pair<std::string, QValue>> fields;
  std::map<std::string, Segment *> methods;

  state.typedefs[n->m_name] = n;
  for (auto &field : n->m_fields) {
    auto res = conv(field.get(), state);
    fields.push_back({field->m_name, res[0]});
  }

  for (auto &method : n->m_methods) {
    auto res = conv(method.get(), state)[0]->as<Global>();
    methods[res->name] = res->value->as<Segment>();
  }

  std::vector<QValue> result;
  result.push_back(RegionDef::create(n->m_name, fields, methods));

  for (auto &static_method : n->m_static_methods) {
    std::shared_ptr<StmtNode> clone;
    if (static_method->is<FunctionDeclNode>()) {
      clone = std::make_shared<FunctionDeclNode>(
          *static_method->as<FunctionDeclNode>());
      auto x = std::static_pointer_cast<FunctionDeclNode>(clone);
      x->m_name = n->m_name + "::" + x->m_name;
    } else if (static_method->is<FunctionDefNode>()) {
      clone = std::make_shared<FunctionDefNode>(
          *static_method->as<FunctionDefNode>());
      auto x = std::static_pointer_cast<FunctionDefNode>(clone);
      x->m_decl->m_name = n->m_name + "::" + x->m_decl->m_name;
    }

    auto res = conv(clone.get(), state);
    result.insert(result.end(), res.begin(), res.end());
  }

  return result;
}

static QResult conv(const RegionFieldNode *n, QState &state) {
  /// TODO: cleanup

  return conv(n->m_type, state)[0]->as<Type>();
}

static QResult conv(const GroupDefNode *n, QState &state) {
  /// TODO: cleanup

  std::map<std::string, QValue> fields;
  std::map<std::string, Segment *> methods;

  state.typedefs[n->m_name] = n;
  for (auto &field : n->m_fields) {
    auto res = conv(field.get(), state);
    fields[field->m_name] = res[0];
  }

  for (auto &method : n->m_methods) {
    auto res = conv(method.get(), state)[0]->as<Global>();
    methods[res->name] = res->value->as<Segment>();
  }

  std::vector<QValue> result;
  result.push_back(GroupDef::create(n->m_name, fields, methods));

  for (auto &static_method : n->m_static_methods) {
    std::shared_ptr<StmtNode> clone;
    if (static_method->is<FunctionDeclNode>()) {
      clone = std::make_shared<FunctionDeclNode>(
          *static_method->as<FunctionDeclNode>());
      auto x = std::static_pointer_cast<FunctionDeclNode>(clone);
      x->m_name = n->m_name + "::" + x->m_name;
    } else if (static_method->is<FunctionDefNode>()) {
      clone = std::make_shared<FunctionDefNode>(
          *static_method->as<FunctionDefNode>());
      auto x = std::static_pointer_cast<FunctionDefNode>(clone);
      x->m_decl->m_name = n->m_name + "::" + x->m_decl->m_name;
    }

    auto res = conv(clone.get(), state);
    result.insert(result.end(), res.begin(), res.end());
  }

  return result;
}

static QResult conv(const GroupFieldNode *n, QState &state) {
  /// TODO: cleanup

  return conv(n->m_type, state)[0]->as<Type>();
}

static QResult conv(const UnionDefNode *n, QState &state) {
  /// TODO: cleanup

  /// TODO: Implement UnionDefNode
  std::map<std::string, QValue> fields;
  for (auto &field : n->m_fields) {
    auto res = conv(field.get(), state);
    fields.insert({field->m_name, res[0]});
  }

  state.typedefs[n->m_name] = n;
  return UnionDef::create(n->m_name, fields, {});
}

static QResult conv(const UnionFieldNode *n, QState &state) {
  /// TODO: cleanup

  return conv(n->m_type, state)[0]->as<Type>();
}

static QResult conv(const EnumDefNode *n, QState &state) {
  /// TODO: cleanup

  if (n->m_fields.empty()) {
    return nullptr;
  }

  Expr *last = nullptr;

  for (auto &field : n->m_fields) {
    if (field->m_value) {
      last = conv(field->m_value.get(), state)[0]->as<Expr>();
    } else {
      if (!last) {
        last = Number::create("0");
      } else {
        last = Add::create(last, Number::create("1"));
      }
    }

    state.enum_values[Symbol::join(n->m_type->m_name, field->m_name)] = last;
  }

  return nullptr;
}

static QResult conv(const EnumFieldNode *n, QState &state) {
  /// TODO: cleanup
  if (!n->m_value) {
    return nullptr;
  }

  return conv(n->m_value.get(), state);
}

static QResult conv(const FunctionDefNode *n, QState &state) {
  /// TODO: cleanup

  state.local_idents.push({});
  state.function.push(n);

  bool old = state.inside_segment;
  state.inside_segment = true;

  auto glob = conv(n->m_decl.get(), state)[0]->as<Global>();
  auto dseg = glob->value->as<Segment>();

  auto body = conv(n->m_body.get(), state)[0]->as<Block>();

  if (body->stmts.empty() || !body->stmts.back()->is<Ret>()) {
    if (dseg->return_type->is<Void>()) {
      body->stmts.push_back(Ret::create(nullptr));
    } else {
      LOG(ERROR) << "QIR conv: function does not return" << std::endl;
      return nullptr;
    }
  }

  state.inside_segment = old;

  state.function.pop();
  state.local_idents.pop();

  auto f =
      Segment::create(dseg->params, dseg->return_type, body, dseg->is_variadic,
                      dseg->is_pure, dseg->is_thread_safe, dseg->is_no_throw,
                      dseg->is_no_return, dseg->is_foriegn);

  return Global::create(glob->name, glob->type, f, glob->_volatile,
                        glob->_atomic, glob->_extern);
}

static QResult conv(const FunctionParamNode *n, QState &state) {
  /// TODO: cleanup

  auto t = conv(n->m_type, state)[0]->as<Type>();
  if (state.inside_segment) state.local_idents.top()[n->m_name] = t;
  return t;
}

static QResult conv(const ExportNode *n, QState &state) {
  /// TODO: cleanup

  std::vector<QValue> sub;
  ExportLangType old = state.lang;
  state.lang = n->m_lang_type;

  for (auto &stmt : n->m_stmts) {
    auto res = conv(stmt.get(), state);
    if (!res) continue;
    sub.insert(sub.end(), res.begin(), res.end());
  }

  state.lang = old;
  return sub;
}

static QResult conv(const InlineAsmNode *n, QState &state) {
  /// TODO: cleanup

  std::vector<std::pair<std::string, Value *>> outputs;
  std::vector<std::pair<std::string, Value *>> inputs;

  for (auto &output : n->m_outputs) {
    auto res = conv(output.second.get(), state);
    outputs.push_back({output.first, res[0]});
  }

  for (auto &input : n->m_inputs) {
    auto res = conv(input.second.get(), state);
    inputs.push_back({input.first, res[0]});
  }

  return Asm::create(n->m_asm, outputs, inputs, n->m_clobbers);
}

static QResult conv(const ReturnStmtNode *n, QState &state) {
  /* Function: Convert a parse tree return statement node into a QIR return
   *           statement.
   *
   * Edge Cases:
   *  - If the expr is null, it is void.
   *
   * General Behavior:
   * - One-for-one lowering of the return statement node.
   **/

  if (!n->m_expr) { /* If the expr is null, it is void */
    return Ret::create(nullptr);
  }

  auto e = conv(n->m_expr.get(), state);
  if (!e) {
    LOG(ERROR) << "QIR conv: return statement expr == nullptr" << std::endl;
    return nullptr;
  }

  if (!state.function.empty())
  {
    auto f = state.function.top();
    auto rettype = conv(f->m_decl->m_type->m_return_type, state)[0]->as<Type>();
    return Ret::create(promote(rettype, e[0]->as<Expr>()));
  }

  return Ret::create(e[0]->as<Expr>());
}

static QResult conv(const RetifStmtNode *n, QState &state) {
  /* Function: Convert a parse tree return-if statement node into a QIR
   *           conditional return block.
   *
   * Edge Cases:
   *  - If the cond is null, abort.
   *  - If the return is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the return-if statement node.
   **/

  if (!n->m_cond) { /* If the cond is null, abort */
    LOG(ERROR) << "QIR conv: return-if statement condition == nullptr"
               << std::endl;
    return nullptr;
  }

  if (!n->m_return) { /* If the return is null, abort */
    LOG(ERROR) << "QIR conv: return-if statement return == nullptr"
               << std::endl;
    return nullptr;
  }

  auto cond = conv(n->m_cond.get(), state);
  if (!cond) { /* If the output condition is null, abort */
    LOG(ERROR) << "QIR conv: return-if statement condition == nullptr"
               << std::endl;
    return nullptr;
  }

  auto ret = conv(n->m_return.get(), state);
  if (!ret) { /* If the output return is null, abort */
    LOG(ERROR) << "QIR conv: return-if statement return == nullptr"
               << std::endl;
    return nullptr;
  }

  /* One-for-one lowering of the return-if statement node */
  return IfElse::create(cond[0]->as<Expr>(), Ret::create(ret[0]->as<Expr>()),
                        nullptr);
}

static QResult conv(const RetzStmtNode *n, QState &state) {
  /* Function: Convert a parse tree return-if-not statement node into a QIR
   *           conditional return block.
   *
   * Edge Cases:
   *  - If the cond is null, abort.
   *  - If the return is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the return-if statement node.
   **/

  if (!n->m_cond) { /* If the cond is null, abort */
    LOG(ERROR) << "QIR conv: return-if-not statement condition == nullptr"
               << std::endl;
    return nullptr;
  }

  if (!n->m_return) { /* If the return is null, abort */
    LOG(ERROR) << "QIR conv: return-if-not statement return == nullptr"
               << std::endl;
    return nullptr;
  }

  auto cond = conv(n->m_cond.get(), state);
  if (!cond) { /* If the output condition is null, abort */
    LOG(ERROR) << "QIR conv: return-if-not statement condition == nullptr"
               << std::endl;
    return nullptr;
  }

  auto inv_cond = Not::create(cond[0]->as<Expr>());

  auto ret = conv(n->m_return.get(), state);
  if (!ret) { /* If the output return is null, abort */
    LOG(ERROR) << "QIR conv: return-if-not statement return == nullptr"
               << std::endl;
    return nullptr;
  }

  /* One-for-one lowering of the return-if-not statement node */
  return IfElse::create(inv_cond, Ret::create(ret[0]->as<Expr>()), nullptr);
}

static QResult conv(const RetvStmtNode *n, QState &state) {
  /* Function: Convert a parse tree return-void statement node into a QIR return
   *           statement.
   *
   * Edge Cases:
   *  - If the cond is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the return-void statement node.
   **/

  if (!n->m_cond) { /* If the cond is null, abort */
    LOG(ERROR) << "QIR conv: return-void statement condition == nullptr"
               << std::endl;
    return nullptr;
  }

  auto cond = conv(n->m_cond.get(), state);
  if (!cond) { /* If the output condition is null, abort */
    LOG(ERROR) << "QIR conv: return-void statement condition == nullptr"
               << std::endl;
    return nullptr;
  }

  /* One-for-one lowering of the return-void statement node */
  return IfElse::create(cond[0]->as<Expr>(), Ret::create(nullptr), nullptr);
}

static QResult conv(const BreakStmtNode *n, QState &state) {
  /* Function: Convert a parse tree break statement node into a QIR break
   *           statement.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the break statement node.
   **/

  return Break::create();
}

static QResult conv(const ContinueStmtNode *n, QState &state) {
  /* Function: Convert a parse tree continue statement node into a QIR continue
   *           statement.
   *
   * Edge Cases:
   *
   * General Behavior:
   * - One-for-one lowering of the continue statement node.
   **/

  return Continue::create();
}

static QResult conv(const IfStmtNode *n, QState &state) {
  /* Function: Convert a parse tree if statement node into a QIR if statement.
   *
   * Edge Cases:
   *  - If the cond is null, use a default condition of true.
   *  - If the then block is null, use a default block of an empty block.
   *  - If the else block is null, use a default block of an empty block.
   *
   * General Behavior:
   * - One-for-one lowering of the if statement node.
   **/

  QResult cond = nullptr;
  QResult then_block = nullptr;
  QResult else_block = nullptr;

  if (n->m_cond) { /* If the cond is null, use a default condition of true */
    cond = conv(n->m_cond.get(), state);
    if (!cond) {
      LOG(ERROR) << "QIR conv: if statement condition == nullptr" << std::endl;
      return nullptr;
    }
  } else {
    cond = Number::create("1");
  }

  if (n->m_then) { /* If the then block is null, use a default block of an
                      empty block */
    then_block = conv(n->m_then.get(), state);
    if (!then_block) {
      LOG(ERROR) << "QIR conv: if statement then block == nullptr" << std::endl;
      return nullptr;
    }
  } else {
    then_block = Block::create({});
  }

  if (n->m_else) { /* If the else block is null, use a default block of an
                      empty block */
    else_block = conv(n->m_else.get(), state);
    if (!else_block) {
      LOG(ERROR) << "QIR conv: if statement else block == nullptr" << std::endl;
      return nullptr;
    }
  } else {
    else_block = Block::create({});
  }

  /* One-for-one lowering of the if statement node */
  return IfElse::create(UCast::create(I1::create(), cond[0]->as<Expr>()),
                        then_block[0]->as<Block>(), else_block[0]->as<Block>());
}

static QResult conv(const WhileStmtNode *n, QState &state) {
  /* Function: Convert a parse tree while loop node into a QIR while loop.
   *
   * Edge Cases:
   *  - If the cond is null, use a default condition of true.
   *  - If the stmt is null, use a default statement of an empty block.
   *
   * General Behavior:
   * - One-for-one lowering of the while loop node.
   **/

  QResult cond = nullptr;
  QResult stmt = nullptr;

  if (n->m_cond) { /* If the cond is null, use a default condition of true */
    cond = conv(n->m_cond.get(), state);

    if (!cond) {
      LOG(ERROR) << "QIR conv: while loop condition == nullptr" << std::endl;
      return nullptr;
    }
  } else {
    cond = Number::create("1");
  }

  if (n->m_stmt) { /* If the stmt is null, use a default statement of an empty
                      block */
    stmt = conv(n->m_stmt.get(), state);
    if (!stmt) {
      LOG(ERROR) << "QIR conv: while loop statement == nullptr" << std::endl;
      return nullptr;
    }
  } else {
    stmt = Block::create({});
  }

  /* One-for-one lowering of the while loop node */
  return While::create(UCast::create(I1::create(), cond[0]->as<Expr>()),
                       stmt[0]->as<Block>());
}

static QResult conv(const ForStmtNode *n, QState &state) {
  /* Function: Convert a parse tree for loop node into a QIR for loop.
   *
   * Edge Cases:
   *  - If the init, cond, step, or stmt fields are null, default them.
   *
   * General Behavior:
   * - One-for-one lowering of the for loop node.
   **/

  Value *init = nullptr;
  Expr *cond = nullptr;
  Expr *step = nullptr;
  Block *stmt = nullptr;

  /* If the init, cond, step, or stmt fields are null, ignore them */
  if (n->m_init)
    init = conv(n->m_init.get(), state)[0]->as<Value>();
  else
    init = Number::create("0");

  if (n->m_cond)
    cond = conv(n->m_cond.get(), state)[0]->as<Expr>();
  else
    cond = Number::create("1");

  if (n->m_step)
    step = conv(n->m_step.get(), state)[0]->as<Expr>();
  else
    step = Number::create("0");

  if (n->m_stmt)
    stmt = conv(n->m_stmt.get(), state)[0]->as<Block>();
  else
    stmt = Block::create({});

  /* One-for-one lowering of the for loop node */
  return For::create(init, UCast::create(I1::create(), cond), step, stmt);
}

static QResult conv(const FormStmtNode *n, QState &state) {
  /// TODO: cleanup

  /// TODO: Implement FormStmtNode
  throw std::runtime_error("QIR translation: FormStmtNode not implemented");
}

static QResult conv(const ForeachStmtNode *n, QState &state) {
  /// TODO: cleanup

  /// TODO: Implement ForeachStmtNode
  throw std::runtime_error("QIR translation: ForeachStmtNode not implemented");
}

static QResult conv(const CaseStmtNode *n, QState &state) {
  /* Function: Convert a parse tree case statement node into a QIR case
   *           statement.
   *
   * Edge Cases:
   *  - If the expr is null, abort.
   *  - If the stmt is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the case statement node.
   **/

  if (!n->m_expr) { /* If the expr is null, abort */
    LOG(ERROR) << "QIR conv: case statement expr == nullptr" << std::endl;
    return nullptr;
  }

  if (!n->m_block) { /* If the stmt is null, abort */
    LOG(ERROR) << "QIR conv: case statement stmt == nullptr" << std::endl;
    return nullptr;
  }

  auto expr = conv(n->m_expr.get(), state);
  if (!expr) { /* If the output expr is null, abort */
    LOG(ERROR) << "QIR conv: case statement expr == nullptr" << std::endl;
    return nullptr;
  }

  auto stmt = conv(n->m_block.get(), state);
  if (!stmt) { /* If the output stmt is null, abort */
    LOG(ERROR) << "QIR conv: case statement stmt == nullptr" << std::endl;
    return nullptr;
  }

  /* One-for-one lowering of the case statement node */
  return Case::create(expr[0]->as<Expr>(), stmt[0]->as<Block>());
}

static QResult conv(const SwitchStmtNode *n, QState &state) {
  /* Function: Convert a parse tree switch statement node into a QIR switch
   *           statement.
   *
   * Edge Cases:
   *  - If the input expr is null, abort.
   *  - If the expr is null, abort.
   *
   * General Behavior:
   * - One-for-one lowering of the switch statement node.
   **/

  if (!n->m_expr) { /* If the expr is null, abort */
    LOG(ERROR) << "QIR conv: switch statement expr == nullptr" << std::endl;
    return nullptr;
  }

  auto expr = conv(n->m_expr.get(), state);
  if (!expr) { /* If the output expr is null, abort */
    LOG(ERROR) << "QIR conv: switch statement expr == nullptr" << std::endl;
    return nullptr;
  }

  QResult def;
  if (n->m_default) {
    def = conv(n->m_default.get(), state);
    if (!def) { /* If the output default is null, ignore it */
      LOG(ERROR) << "QIR conv: switch statement default == nullptr"
                 << std::endl;
      return nullptr;
    }
  }
  Value *default_block = nullptr;
  if (def) default_block = def[0]->as<Block>();

  std::vector<Case *> cases;
  for (const auto &c : n->m_cases) {
    if (!c) { /* If the case is null, ignore it */
      LOG(ERROR) << "QIR conv: switch statement case == nullptr" << std::endl;
      continue;
    }

    auto res = conv(c.get(), state);
    if (!res) { /* If the output case is null, ignore it */
      LOG(ERROR) << "QIR conv: switch statement case == nullptr" << std::endl;
      continue;
    }

    cases.push_back(res[0]->as<Case>());
  }

  /* One-for-one lowering of the switch statement node */
  return Switch::create(expr[0]->as<Expr>(), cases, default_block);
}

static QResult conv(const ParseNode *n, QState &state) {
  QResult r;

  switch (n->ntype) {
    case NodeType::ExprStmtNode:
      r = conv(n->as<ExprStmtNode>(), state);
      break;

    case NodeType::StmtExprNode:
      r = conv(n->as<StmtExprNode>(), state);
      break;

    case NodeType::NopStmtNode:
      r = conv(n->as<NopStmtNode>(), state);
      break;

    case NodeType::BlockNode:
      r = conv(n->as<BlockNode>(), state);
      break;

    case NodeType::StmtGroupNode:
      r = conv(n->as<StmtGroupNode>(), state);
      break;

    case NodeType::ConstUnaryExprNode:
      r = conv(n->as<ConstUnaryExprNode>(), state);
      break;

    case NodeType::ConstPostUnaryExprNode:
      r = conv(n->as<ConstPostUnaryExprNode>(), state);
      break;

    case NodeType::ConstBinaryExprNode:
      r = conv(n->as<ConstBinaryExprNode>(), state);
      break;

    case NodeType::SeqExprNode:
      r = conv(n->as<SeqExprNode>(), state);
      break;

    case NodeType::StaticCastExprNode:
      r = conv(n->as<StaticCastExprNode>(), state);
      break;

    case NodeType::BitCastExprNode:
      r = conv(n->as<BitCastExprNode>(), state);
      break;

    case NodeType::SignedUpcastExprNode:
      r = conv(n->as<SignedUpcastExprNode>(), state);
      break;

    case NodeType::UnsignedUpcastExprNode:
      r = conv(n->as<UnsignedUpcastExprNode>(), state);
      break;

    case NodeType::DowncastExprNode:
      r = conv(n->as<DowncastExprNode>(), state);
      break;

    case NodeType::UnaryExprNode:
      r = conv(n->as<UnaryExprNode>(), state);
      break;

    case NodeType::PostUnaryExprNode:
      r = conv(n->as<PostUnaryExprNode>(), state);
      break;

    case NodeType::BinaryExprNode:
      r = conv(n->as<BinaryExprNode>(), state);
      break;

    case NodeType::CallExprNode:
      r = conv(n->as<CallExprNode>(), state);
      break;

    case NodeType::ListExprNode:
      r = conv(n->as<ListExprNode>(), state);
      break;

    case NodeType::MemberAccessNode:
      r = conv(n->as<MemberAccessNode>(), state);
      break;

    case NodeType::IndexNode:
      r = conv(n->as<IndexNode>(), state);
      break;

    case NodeType::SliceNode:
      r = conv(n->as<SliceNode>(), state);
      break;

    case NodeType::FStringNode:
      r = conv(n->as<FStringNode>(), state);
      break;

    case NodeType::IdentifierNode:
      r = conv(n->as<IdentifierNode>(), state);
      break;

    case NodeType::MutTypeNode:
      r = conv(n->as<MutTypeNode>(), state);
      break;

    case NodeType::U8TypeNode:
      r = conv(n->as<U8TypeNode>(), state);
      break;

    case NodeType::U16TypeNode:
      r = conv(n->as<U16TypeNode>(), state);
      break;

    case NodeType::U32TypeNode:
      r = conv(n->as<U32TypeNode>(), state);
      break;

    case NodeType::U64TypeNode:
      r = conv(n->as<U64TypeNode>(), state);
      break;

    case NodeType::U128TypeNode:
      r = conv(n->as<U128TypeNode>(), state);
      break;

    case NodeType::I8TypeNode:
      r = conv(n->as<I8TypeNode>(), state);
      break;

    case NodeType::I16TypeNode:
      r = conv(n->as<I16TypeNode>(), state);
      break;

    case NodeType::I32TypeNode:
      r = conv(n->as<I32TypeNode>(), state);
      break;

    case NodeType::I64TypeNode:
      r = conv(n->as<I64TypeNode>(), state);
      break;

    case NodeType::I128TypeNode:
      r = conv(n->as<I128TypeNode>(), state);
      break;

    case NodeType::F32TypeNode:
      r = conv(n->as<F32TypeNode>(), state);
      break;

    case NodeType::F64TypeNode:
      r = conv(n->as<F64TypeNode>(), state);
      break;

    case NodeType::BoolTypeNode:
      r = conv(n->as<BoolTypeNode>(), state);
      break;

    case NodeType::VoidTypeNode:
      r = conv(n->as<VoidTypeNode>(), state);
      break;

    case NodeType::NullTypeNode:
      r = conv(n->as<NullTypeNode>(), state);
      break;

    case NodeType::PointerTypeNode:
      r = conv(n->as<PointerTypeNode>(), state);
      break;

    case NodeType::OpaqueTypeNode:
      r = conv(n->as<OpaqueTypeNode>(), state);
      break;

    case NodeType::StringTypeNode:
      r = conv(n->as<StringTypeNode>(), state);
      break;

    case NodeType::EnumTypeNode:
      r = conv(n->as<EnumTypeNode>(), state);
      break;

    case NodeType::StructTypeNode:
      r = conv(n->as<StructTypeNode>(), state);
      break;

    case NodeType::GroupTypeNode:
      r = conv(n->as<GroupTypeNode>(), state);
      break;

    case NodeType::RegionTypeNode:
      r = conv(n->as<RegionTypeNode>(), state);
      break;

    case NodeType::UnionTypeNode:
      r = conv(n->as<UnionTypeNode>(), state);
      break;

    case NodeType::ArrayTypeNode:
      r = conv(n->as<ArrayTypeNode>(), state);
      break;

    case NodeType::VectorTypeNode:
      r = conv(n->as<VectorTypeNode>(), state);
      break;

    case NodeType::ResultTypeNode:
      r = conv(n->as<ResultTypeNode>(), state);
      break;

    case NodeType::GeneratorTypeNode:
      r = conv(n->as<GeneratorTypeNode>(), state);
      break;

    case NodeType::FunctionTypeNode:
      r = conv(n->as<FunctionTypeNode>(), state);
      break;

    case NodeType::IntegerNode:
      r = conv(n->as<IntegerNode>(), state);
      break;

    case NodeType::FloatLiteralNode:
      r = conv(n->as<FloatLiteralNode>(), state);
      break;

    case NodeType::StringNode:
      r = conv(n->as<StringNode>(), state);
      break;

    case NodeType::CharNode:
      r = conv(n->as<CharNode>(), state);
      break;

    case NodeType::BoolLiteralNode:
      r = conv(n->as<BoolLiteralNode>(), state);
      break;

    case NodeType::NullLiteralNode:
      r = conv(n->as<NullLiteralNode>(), state);
      break;

    case NodeType::UndefLiteralNode:
      r = conv(n->as<UndefLiteralNode>(), state);
      break;

    case NodeType::TypedefNode:
      r = conv(n->as<TypedefNode>(), state);
      break;

    case NodeType::VarDeclNode:
      r = conv(n->as<VarDeclNode>(), state);
      break;

    case NodeType::LetDeclNode:
      r = conv(n->as<LetDeclNode>(), state);
      break;

    case NodeType::ConstDeclNode:
      r = conv(n->as<ConstDeclNode>(), state);
      break;

    case NodeType::FunctionDeclNode:
      r = conv(n->as<FunctionDeclNode>(), state);
      break;

    case NodeType::StructDefNode:
      r = conv(n->as<StructDefNode>(), state);
      break;

    case NodeType::StructFieldNode:
      r = conv(n->as<StructFieldNode>(), state);
      break;

    case NodeType::RegionDefNode:
      r = conv(n->as<RegionDefNode>(), state);
      break;

    case NodeType::RegionFieldNode:
      r = conv(n->as<RegionFieldNode>(), state);
      break;

    case NodeType::GroupDefNode:
      r = conv(n->as<GroupDefNode>(), state);
      break;

    case NodeType::GroupFieldNode:
      r = conv(n->as<GroupFieldNode>(), state);
      break;

    case NodeType::UnionDefNode:
      r = conv(n->as<UnionDefNode>(), state);
      break;

    case NodeType::UnionFieldNode:
      r = conv(n->as<UnionFieldNode>(), state);
      break;

    case NodeType::EnumDefNode:
      r = conv(n->as<EnumDefNode>(), state);
      break;

    case NodeType::EnumFieldNode:
      r = conv(n->as<EnumFieldNode>(), state);
      break;

    case NodeType::FunctionDefNode:
      r = conv(n->as<FunctionDefNode>(), state);
      break;

    case NodeType::FunctionParamNode:
      r = conv(n->as<FunctionParamNode>(), state);
      break;

    case NodeType::ExportNode:
      r = conv(n->as<ExportNode>(), state);
      break;

    case NodeType::InlineAsmNode:
      r = conv(n->as<InlineAsmNode>(), state);
      break;

    case NodeType::ReturnStmtNode:
      r = conv(n->as<ReturnStmtNode>(), state);
      break;

    case NodeType::RetifStmtNode:
      r = conv(n->as<RetifStmtNode>(), state);
      break;

    case NodeType::RetzStmtNode:
      r = conv(n->as<RetzStmtNode>(), state);
      break;

    case NodeType::RetvStmtNode:
      r = conv(n->as<RetvStmtNode>(), state);
      break;

    case NodeType::BreakStmtNode:
      r = conv(n->as<BreakStmtNode>(), state);
      break;

    case NodeType::ContinueStmtNode:
      r = conv(n->as<ContinueStmtNode>(), state);
      break;

    case NodeType::IfStmtNode:
      r = conv(n->as<IfStmtNode>(), state);
      break;

    case NodeType::WhileStmtNode:
      r = conv(n->as<WhileStmtNode>(), state);
      break;

    case NodeType::ForStmtNode:
      r = conv(n->as<ForStmtNode>(), state);
      break;

    case NodeType::FormStmtNode:
      r = conv(n->as<FormStmtNode>(), state);
      break;

    case NodeType::ForeachStmtNode:
      r = conv(n->as<ForeachStmtNode>(), state);
      break;

    case NodeType::CaseStmtNode:
      r = conv(n->as<CaseStmtNode>(), state);
      break;

    case NodeType::SwitchStmtNode:
      r = conv(n->as<SwitchStmtNode>(), state);
      break;
    default:
      throw std::runtime_error("QIR translation: Unknown node type");
  }

  return r;
}
/// END: CONVERSION FUNCTIONS
///============================================================================

bool ir::q::QModule::from_ptree(quixcc_job_t *job,
                                std::shared_ptr<ParseNode> ast) {
  /// TODO: cleanup

  LOG(DEBUG) << "Converting Ptree to QUIX intermediate representation"
             << std::endl;

  auto block_node = std::static_pointer_cast<BlockNode>(ast);

  QState state;
  std::vector<QValue> children;
  for (auto &child : block_node->m_stmts) {
    auto res = conv(child.get(), state);
    if (!res) continue;

    for (auto &r : res) {
      children.push_back(r);
    }
  }

  m_root = RootNode::create(children);

  if (!verify()) {
    LOG(FATAL) << "Failed to qualify QUIX intermediate representation"
               << std::endl;
    return false;
  }

  LOG(DEBUG)
      << "Successfully converted Ptree to QUIX intermediate representation"
      << std::endl;

  return true;
}

void ir::q::QModule::acknowledge_pass(ir::q::QPassType pass,
                                      const std::string &name) {
  m_passes[pass].push_back(name);
}

void ir::q::QModule::unacknowledge_pass(ir::q::QPassType pass,
                                        const std::string &name) {
  m_passes[pass].erase(
      std::remove(m_passes[pass].begin(), m_passes[pass].end(), name),
      m_passes[pass].end());
}

void ir::q::QModule::add_tag(const std::string &tag) { m_tags.insert(tag); }

void ir::q::QModule::remove_tag(const std::string &tag) { m_tags.erase(tag); }