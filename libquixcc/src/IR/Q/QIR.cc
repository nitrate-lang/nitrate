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

#include <deque>
#include <stack>
#include <unordered_set>

using namespace libquixcc::ir::q;

std::string_view libquixcc::ir::q::Value::ntype_str() const {
  switch ((QType)ntype) {
    case QType::Root:
      return "Root";
    case QType::I1:
      return "I1";
    case QType::I8:
      return "I8";
    case QType::I16:
      return "I16";
    case QType::I32:
      return "I32";
    case QType::I64:
      return "I64";
    case QType::I128:
      return "I128";
    case QType::U8:
      return "U8";
    case QType::U16:
      return "U16";
    case QType::U32:
      return "U32";
    case QType::U64:
      return "U64";
    case QType::U128:
      return "U128";
    case QType::F32:
      return "F32";
    case QType::F64:
      return "F64";
    case QType::Void:
      return "Void";
    case QType::Ptr:
      return "Ptr";
    case QType::Array:
      return "Array";
    case QType::Vector:
      return "Vector";
    case QType::FType:
      return "FType";
    case QType::Region:
      return "Region";
    case QType::Union:
      return "Union";
    case QType::Opaque:
      return "Opaque";
    case QType::Block:
      return "Block";
    case QType::Segment:
      return "Segment";
    case QType::Asm:
      return "Asm";
    case QType::RegionDef:
      return "RegionDef";
    case QType::GroupDef:
      return "GroupDef";
    case QType::UnionDef:
      return "UnionDef";
    case QType::SCast:
      return "SCast";
    case QType::UCast:
      return "UCast";
    case QType::PtrICast:
      return "PtrICast";
    case QType::IPtrCast:
      return "IPtrCast";
    case QType::Bitcast:
      return "Bitcast";
    case QType::Call:
      return "Call";
    case QType::CallIndirect:
      return "CallIndirect";
    case QType::IfElse:
      return "IfElse";
    case QType::While:
      return "While";
    case QType::For:
      return "For";
    case QType::Loop:
      return "Loop";
    case QType::Break:
      return "Break";
    case QType::Continue:
      return "Continue";
    case QType::Ret:
      return "Ret";
    case QType::Throw:
      return "Throw";
    case QType::TryCatchFinally:
      return "TryCatchFinally";
    case QType::Case:
      return "Case";
    case QType::Switch:
      return "Switch";
    case QType::Ident:
      return "Ident";
    case QType::Add:
      return "Add";
    case QType::Sub:
      return "Sub";
    case QType::Mul:
      return "Mul";
    case QType::Div:
      return "Div";
    case QType::Mod:
      return "Mod";
    case QType::BitAnd:
      return "BitAnd";
    case QType::BitOr:
      return "BitOr";
    case QType::BitXor:
      return "BitXor";
    case QType::BitNot:
      return "BitNot";
    case QType::Shl:
      return "Shl";
    case QType::Shr:
      return "Shr";
    case QType::Rotl:
      return "Rotl";
    case QType::Rotr:
      return "Rotr";
    case QType::Eq:
      return "Eq";
    case QType::Ne:
      return "Ne";
    case QType::Lt:
      return "Lt";
    case QType::Gt:
      return "Gt";
    case QType::Le:
      return "Le";
    case QType::Ge:
      return "Ge";
    case QType::And:
      return "And";
    case QType::Or:
      return "Or";
    case QType::Not:
      return "Not";
    case QType::Xor:
      return "Xor";
    case QType::Local:
      return "Local";
    case QType::Global:
      return "Global";
    case QType::Number:
      return "Number";
    case QType::String:
      return "String";
    case QType::Char:
      return "Char";
    case QType::List:
      return "List";
    case QType::Assign:
      return "Assign";
    case QType::PostInc:
      return "PostInc";
    case QType::PostDec:
      return "PostDec";
    case QType::AddressOf:
      return "AddressOf";
    case QType::Deref:
      return "Deref";
    case QType::Member:
      return "Member";
    case QType::Index:
      return "Index";
    case QType::IntrinsicCall:
      return "IntrinsicCall";
    case QType::IntrinsicType:
      return "IntrinsicType";
    case QType::EnumMax:
      throw std::out_of_range("EnumMax is not a valid QType");
  }
}

bool QModule::print_impl(std::ostream &os, PState &state) const {
  if (state.modinfo) {
    os << "use QIR_1_0;\n";
    os << "; ModuleID = '" << m_name << "'\n";

    if (!m_root) return true;

    os << "; ModuleHash = '";
    m_root->printid(os);
    os << "'\n";

    os << "; SolverPasses = [";
    for (size_t i = 0; i < m_passes.at(QPassType::Solver).size(); i++) {
      os << "'" << m_passes.at(QPassType::Solver).at(i) << "'";
      if (i < m_passes.at(QPassType::Solver).size() - 1) os << ", ";
    }
    os << "]\n";

    os << "; OptimizerPasses = [";
    for (size_t i = 0; i < m_passes.at(QPassType::Optimizer).size(); i++) {
      os << "'" << m_passes.at(QPassType::Optimizer).at(i) << "'";
      if (i < m_passes.at(QPassType::Optimizer).size() - 1) os << ", ";
    }

    os << "]\n";

    os << "; Tags = [";
    for (auto it = m_tags.begin(); it != m_tags.end(); it++) {
      os << "'" << *it << "'";
      if (std::next(it) != m_tags.end()) os << ", ";
    }
    os << "]\n\n";
  }

  if (!m_root->print(os, state)) return false;

  if (state.modinfo) {
    os << "\n; End of module '";
    this->m_root->printid(os);

    os << "'\n";
  }

  return true;
}

bool QModule::verify_impl() const {
  if (!m_root) return false;

  return m_root->verify();
}

std::string_view QModule::ir_dialect_name_impl() const { return "QIR-Q"; }

unsigned int QModule::ir_dialect_version_impl() const { return 1; }

std::string_view QModule::ir_dialect_family_impl() const { return "QIR"; }

std::string_view QModule::ir_dialect_description_impl() const {
  return "Quix Q Intermediate Representation (QIR-Q-V1.0) is an intermediate "
         "representation for the Quix language. ... (write something useful "
         "here)";
}

void QModule::dft_iter(QModule::IterFunc func, QModule::PredFunc predicate,
                       bool post_order) {
  /// TODO: TEST THIS CRITICAL FUNCTION
  std::stack<Value **> stack;
  std::unordered_set<Value **> finished;

  stack.push(reinterpret_cast<Value **>(&m_root));

  auto push_children = [&stack](Value **val) -> bool {
    switch ((QType)(*val)->ntype) {
      case QType::Root:
        for (auto &child : static_cast<RootNode *>(*val)->children) {
          stack.push(&child);
        }
        break;
      case QType::I1:
      case QType::I8:
      case QType::I16:
      case QType::I32:
      case QType::I64:
      case QType::I128:
      case QType::U8:
      case QType::U16:
      case QType::U32:
      case QType::U64:
      case QType::U128:
      case QType::F32:
      case QType::F64:
      case QType::Void:
        break;
      case QType::Ptr:
        stack.push(reinterpret_cast<Value **>(&static_cast<Ptr *>(*val)->type));
        break;
      case QType::Array:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Array *>(*val)->type));
        break;
      case QType::Vector:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Vector *>(*val)->type));
        break;
      case QType::FType:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<FType *>(*val)->ret));
        for (auto &arg : static_cast<FType *>(*val)->params) {
          stack.push(reinterpret_cast<Value **>(&arg));
        }
        break;
      case QType::Region:
        for (auto &child : static_cast<Region *>(*val)->fields) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::Union:
        for (auto &child : static_cast<Union *>(*val)->fields) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::Opaque:
        break;
      case QType::Block:
        for (auto &child : static_cast<Block *>(*val)->stmts) {
          stack.push(&child);
        }
        break;
      case QType::Segment:
        for (auto &child : static_cast<Segment *>(*val)->params) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Segment *>(*val)->block));
        stack.push(reinterpret_cast<Value **>(
            &static_cast<Segment *>(*val)->return_type));
        break;
      case QType::Asm:
        for (auto &child : static_cast<Asm *>(*val)->inputs) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        for (auto &child : static_cast<Asm *>(*val)->outputs) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::RegionDef:
        for (auto &child : static_cast<RegionDef *>(*val)->fields) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        for (auto &child : static_cast<RegionDef *>(*val)->methods) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::GroupDef:
        for (auto &child : static_cast<GroupDef *>(*val)->fields) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        for (auto &child : static_cast<GroupDef *>(*val)->methods) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::UnionDef:
        for (auto &child : static_cast<UnionDef *>(*val)->fields) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        for (auto &child : static_cast<UnionDef *>(*val)->methods) {
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::SCast:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<SCast *>(*val)->type));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<SCast *>(*val)->value));
        break;
      case QType::UCast:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<UCast *>(*val)->type));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<UCast *>(*val)->value));
        break;
      case QType::PtrICast:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<PtrICast *>(*val)->value));
        break;
      case QType::IPtrCast:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<IPtrCast *>(*val)->type));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<IPtrCast *>(*val)->value));
        break;
      case QType::Bitcast:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Bitcast *>(*val)->type));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Bitcast *>(*val)->value));
        break;
      case QType::Call:
        for (auto &child : static_cast<Call *>(*val)->args) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Call *>(*val)->func));
        break;
      case QType::CallIndirect:
        for (auto &child : static_cast<CallIndirect *>(*val)->args) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        stack.push(reinterpret_cast<Value **>(
            &static_cast<CallIndirect *>(*val)->exprfunc));
        break;
      case QType::IfElse:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<IfElse *>(*val)->cond));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<IfElse *>(*val)->then));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<IfElse *>(*val)->els));
        break;
      case QType::While:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<While *>(*val)->cond));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<While *>(*val)->body));
        break;
      case QType::For:
        stack.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->init));
        stack.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->cond));
        stack.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->step));
        stack.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->body));
        break;
      case QType::Loop:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Loop *>(*val)->body));
        break;
      case QType::Break:
        break;
      case QType::Continue:
        break;
      case QType::Ret:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Ret *>(*val)->value));
        break;
      case QType::Throw:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Throw *>(*val)->value));
        break;
      case QType::TryCatchFinally:
        stack.push(reinterpret_cast<Value **>(
            &static_cast<TryCatchFinally *>(*val)->tryblock));
        for (auto &child : static_cast<TryCatchFinally *>(*val)->catchblocks) {
          stack.push(reinterpret_cast<Value **>(&child.first));
          stack.push(reinterpret_cast<Value **>(&child.second));
        }
        stack.push(reinterpret_cast<Value **>(
            &static_cast<TryCatchFinally *>(*val)->finallyblock));
        break;
      case QType::Case:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Case *>(*val)->value));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Case *>(*val)->body));
        break;
      case QType::Switch:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Switch *>(*val)->value));
        for (auto &child : static_cast<Switch *>(*val)->cases) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        stack.push(reinterpret_cast<Value **>(
            &static_cast<Switch *>(*val)->defaultcase));
        break;
      case QType::Ident:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Ident *>(*val)->type));
        break;
      case QType::Add:
        stack.push(reinterpret_cast<Value **>(&static_cast<Add *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Add *>(*val)->rhs));
        break;
      case QType::Sub:
        stack.push(reinterpret_cast<Value **>(&static_cast<Sub *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Sub *>(*val)->rhs));
        break;
      case QType::Mul:
        stack.push(reinterpret_cast<Value **>(&static_cast<Mul *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Mul *>(*val)->rhs));
        break;
      case QType::Div:
        stack.push(reinterpret_cast<Value **>(&static_cast<Div *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Div *>(*val)->rhs));
        break;
      case QType::Mod:
        stack.push(reinterpret_cast<Value **>(&static_cast<Mod *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Mod *>(*val)->rhs));
        break;
      case QType::BitAnd:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<BitAnd *>(*val)->lhs));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<BitAnd *>(*val)->rhs));
        break;
      case QType::BitOr:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<BitOr *>(*val)->lhs));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<BitOr *>(*val)->rhs));
        break;
      case QType::BitXor:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<BitXor *>(*val)->lhs));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<BitXor *>(*val)->rhs));
        break;
      case QType::BitNot:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<BitNot *>(*val)->operand));
        break;
      case QType::Shl:
        stack.push(reinterpret_cast<Value **>(&static_cast<Shl *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Shl *>(*val)->rhs));
        break;
      case QType::Shr:
        stack.push(reinterpret_cast<Value **>(&static_cast<Shr *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Shr *>(*val)->rhs));
        break;

      case QType::Rotl:
        stack.push(reinterpret_cast<Value **>(&static_cast<Rotl *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Rotl *>(*val)->rhs));
        break;
      case QType::Rotr:
        stack.push(reinterpret_cast<Value **>(&static_cast<Rotr *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Rotr *>(*val)->rhs));
        break;
      case QType::Eq:
        stack.push(reinterpret_cast<Value **>(&static_cast<Eq *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Eq *>(*val)->rhs));
        break;
      case QType::Ne:
        stack.push(reinterpret_cast<Value **>(&static_cast<Ne *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Ne *>(*val)->rhs));
        break;
      case QType::Lt:
        stack.push(reinterpret_cast<Value **>(&static_cast<Lt *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Lt *>(*val)->rhs));
        break;
      case QType::Gt:
        stack.push(reinterpret_cast<Value **>(&static_cast<Gt *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Gt *>(*val)->rhs));
        break;
      case QType::Le:
        stack.push(reinterpret_cast<Value **>(&static_cast<Le *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Le *>(*val)->rhs));
        break;
      case QType::Ge:
        stack.push(reinterpret_cast<Value **>(&static_cast<Ge *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Ge *>(*val)->rhs));
        break;
      case QType::And:
        stack.push(reinterpret_cast<Value **>(&static_cast<And *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<And *>(*val)->rhs));
        break;
      case QType::Or:
        stack.push(reinterpret_cast<Value **>(&static_cast<Or *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Or *>(*val)->rhs));
        break;
      case QType::Not:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Not *>(*val)->operand));
        break;
      case QType::Xor:
        stack.push(reinterpret_cast<Value **>(&static_cast<Xor *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(&static_cast<Xor *>(*val)->rhs));
        break;
      case QType::Local:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Local *>(*val)->type));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Local *>(*val)->value));
        break;
      case QType::Global:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Global *>(*val)->type));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Global *>(*val)->value));
        break;
      case QType::Number:
      case QType::String:
      case QType::Char:
        break;
      case QType::List:
        for (auto &child : static_cast<List *>(*val)->values) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::Assign:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Assign *>(*val)->lhs));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Assign *>(*val)->rhs));
        break;
      case QType::PostInc:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<PostInc *>(*val)->lhs));
        break;
      case QType::PostDec:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<PostDec *>(*val)->lhs));
        break;
      case QType::AddressOf:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<AddressOf *>(*val)->lhs));
        break;
      case QType::Deref:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Deref *>(*val)->lhs));
        break;
      case QType::Member:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Member *>(*val)->lhs));
        stack.push(reinterpret_cast<Value **>(
            &static_cast<Member *>(*val)->field_type));
        break;
      case QType::Index:
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Index *>(*val)->lhs));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Index *>(*val)->index));
        stack.push(
            reinterpret_cast<Value **>(&static_cast<Index *>(*val)->type));
        break;
      case QType::IntrinsicCall:
        for (auto &child : static_cast<IntrinsicCall *>(*val)->args) {
          stack.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::IntrinsicType:
        break;
      case QType::EnumMax:
        break;
    }
    return false;
  };

  while (!stack.empty()) {
    Value **current = stack.top();
    if (!*current) {
      stack.pop();
      continue;
    }

    if (post_order) {
      if (!finished.contains(current)) {
        if (push_children(current)) {
          return; /* Abort iteration */
        }
        finished.insert(current);
      } else {
        stack.pop();

        if (predicate) {
          switch (predicate(*current)) {
            case IterOp::Do:
              break;
            case IterOp::Skip:
              continue;
            case IterOp::Abort:
              return; /* Abort iteration */
          }
        }
        func(current);
      }
    } else {
      stack.pop();

      if (push_children(current)) {
        return; /* Abort iteration */
      }

      if (predicate) {
        switch (predicate(*current)) {
          case IterOp::Do:
            break;
          case IterOp::Skip:
            continue;
          case IterOp::Abort:
            return; /* Abort iteration */
        }
      }
      func(current);
    }
  }
}

void QModule::bft_iter(QModule::IterFunc func, QModule::PredFunc predicate,
                       bool post_order) {
  /// TODO: TEST THIS CRITICAL FUNCTION
  std::queue<Value **> queue;
  std::unordered_set<Value **> visited;

  queue.push(reinterpret_cast<Value **>(&m_root));

  auto enqueue_children = [&queue](Value **val) -> bool {
    switch ((QType)(*val)->ntype) {
      case QType::Root:
        for (auto &child : static_cast<RootNode *>(*val)->children) {
          queue.push(&child);
        }
        break;
      case QType::I1:
      case QType::I8:
      case QType::I16:
      case QType::I32:
      case QType::I64:
      case QType::I128:
      case QType::U8:
      case QType::U16:
      case QType::U32:
      case QType::U64:
      case QType::U128:
      case QType::F32:
      case QType::F64:
      case QType::Void:
        break;
      case QType::Ptr:
        queue.push(reinterpret_cast<Value **>(&static_cast<Ptr *>(*val)->type));
        break;
      case QType::Array:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Array *>(*val)->type));
        break;
      case QType::Vector:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Vector *>(*val)->type));
        break;
      case QType::FType:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<FType *>(*val)->ret));
        for (auto &arg : static_cast<FType *>(*val)->params) {
          queue.push(reinterpret_cast<Value **>(&arg));
        }
        break;
      case QType::Region:
        for (auto &child : static_cast<Region *>(*val)->fields) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::Union:
        for (auto &child : static_cast<Union *>(*val)->fields) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::Opaque:
        break;
      case QType::Block:
        for (auto &child : static_cast<Block *>(*val)->stmts) {
          queue.push(&child);
        }
        break;
      case QType::Segment:
        for (auto &child : static_cast<Segment *>(*val)->params) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Segment *>(*val)->block));
        queue.push(reinterpret_cast<Value **>(
            &static_cast<Segment *>(*val)->return_type));
        break;
      case QType::Asm:
        for (auto &child : static_cast<Asm *>(*val)->inputs) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        for (auto &child : static_cast<Asm *>(*val)->outputs) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::RegionDef:
        for (auto &child : static_cast<RegionDef *>(*val)->fields) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        for (auto &child : static_cast<RegionDef *>(*val)->methods) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::GroupDef:
        for (auto &child : static_cast<GroupDef *>(*val)->fields) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        for (auto &child : static_cast<GroupDef *>(*val)->methods) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::UnionDef:
        for (auto &child : static_cast<UnionDef *>(*val)->fields) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        for (auto &child : static_cast<UnionDef *>(*val)->methods) {
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        break;
      case QType::SCast:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<SCast *>(*val)->type));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<SCast *>(*val)->value));
        break;
      case QType::UCast:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<UCast *>(*val)->type));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<UCast *>(*val)->value));
        break;
      case QType::PtrICast:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<PtrICast *>(*val)->value));
        break;
      case QType::IPtrCast:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<IPtrCast *>(*val)->type));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<IPtrCast *>(*val)->value));
        break;
      case QType::Bitcast:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Bitcast *>(*val)->type));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Bitcast *>(*val)->value));
        break;
      case QType::Call:
        for (auto &child : static_cast<Call *>(*val)->args) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Call *>(*val)->func));
        break;
      case QType::CallIndirect:
        for (auto &child : static_cast<CallIndirect *>(*val)->args) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        queue.push(reinterpret_cast<Value **>(
            &static_cast<CallIndirect *>(*val)->exprfunc));
        break;
      case QType::IfElse:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<IfElse *>(*val)->cond));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<IfElse *>(*val)->then));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<IfElse *>(*val)->els));
        break;
      case QType::While:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<While *>(*val)->cond));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<While *>(*val)->body));
        break;
      case QType::For:
        queue.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->init));
        queue.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->cond));
        queue.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->step));
        queue.push(reinterpret_cast<Value **>(&static_cast<For *>(*val)->body));
        break;
      case QType::Loop:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Loop *>(*val)->body));
        break;
      case QType::Break:
        break;
      case QType::Continue:
        break;
      case QType::Ret:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Ret *>(*val)->value));
        break;
      case QType::Throw:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Throw *>(*val)->value));
        break;
      case QType::TryCatchFinally:
        queue.push(reinterpret_cast<Value **>(
            &static_cast<TryCatchFinally *>(*val)->tryblock));
        for (auto &child : static_cast<TryCatchFinally *>(*val)->catchblocks) {
          queue.push(reinterpret_cast<Value **>(&child.first));
          queue.push(reinterpret_cast<Value **>(&child.second));
        }
        queue.push(reinterpret_cast<Value **>(
            &static_cast<TryCatchFinally *>(*val)->finallyblock));
        break;
      case QType::Case:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Case *>(*val)->value));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Case *>(*val)->body));
        break;
      case QType::Switch:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Switch *>(*val)->value));
        for (auto &child : static_cast<Switch *>(*val)->cases) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        queue.push(reinterpret_cast<Value **>(
            &static_cast<Switch *>(*val)->defaultcase));
        break;
      case QType::Ident:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Ident *>(*val)->type));
        break;
      case QType::Add:
        queue.push(reinterpret_cast<Value **>(&static_cast<Add *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Add *>(*val)->rhs));
        break;
      case QType::Sub:
        queue.push(reinterpret_cast<Value **>(&static_cast<Sub *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Sub *>(*val)->rhs));
        break;
      case QType::Mul:
        queue.push(reinterpret_cast<Value **>(&static_cast<Mul *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Mul *>(*val)->rhs));
        break;
      case QType::Div:
        queue.push(reinterpret_cast<Value **>(&static_cast<Div *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Div *>(*val)->rhs));
        break;
      case QType::Mod:
        queue.push(reinterpret_cast<Value **>(&static_cast<Mod *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Mod *>(*val)->rhs));
        break;
      case QType::BitAnd:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<BitAnd *>(*val)->lhs));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<BitAnd *>(*val)->rhs));
        break;
      case QType::BitOr:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<BitOr *>(*val)->lhs));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<BitOr *>(*val)->rhs));
        break;
      case QType::BitXor:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<BitXor *>(*val)->lhs));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<BitXor *>(*val)->rhs));
        break;
      case QType::BitNot:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<BitNot *>(*val)->operand));
        break;
      case QType::Shl:
        queue.push(reinterpret_cast<Value **>(&static_cast<Shl *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Shl *>(*val)->rhs));
        break;
      case QType::Shr:
        queue.push(reinterpret_cast<Value **>(&static_cast<Shr *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Shr *>(*val)->rhs));
        break;

      case QType::Rotl:
        queue.push(reinterpret_cast<Value **>(&static_cast<Rotl *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Rotl *>(*val)->rhs));
        break;
      case QType::Rotr:
        queue.push(reinterpret_cast<Value **>(&static_cast<Rotr *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Rotr *>(*val)->rhs));
        break;
      case QType::Eq:
        queue.push(reinterpret_cast<Value **>(&static_cast<Eq *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Eq *>(*val)->rhs));
        break;
      case QType::Ne:
        queue.push(reinterpret_cast<Value **>(&static_cast<Ne *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Ne *>(*val)->rhs));
        break;
      case QType::Lt:
        queue.push(reinterpret_cast<Value **>(&static_cast<Lt *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Lt *>(*val)->rhs));
        break;
      case QType::Gt:
        queue.push(reinterpret_cast<Value **>(&static_cast<Gt *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Gt *>(*val)->rhs));
        break;
      case QType::Le:
        queue.push(reinterpret_cast<Value **>(&static_cast<Le *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Le *>(*val)->rhs));
        break;
      case QType::Ge:
        queue.push(reinterpret_cast<Value **>(&static_cast<Ge *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Ge *>(*val)->rhs));
        break;
      case QType::And:
        queue.push(reinterpret_cast<Value **>(&static_cast<And *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<And *>(*val)->rhs));
        break;
      case QType::Or:
        queue.push(reinterpret_cast<Value **>(&static_cast<Or *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Or *>(*val)->rhs));
        break;
      case QType::Not:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Not *>(*val)->operand));
        break;
      case QType::Xor:
        queue.push(reinterpret_cast<Value **>(&static_cast<Xor *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(&static_cast<Xor *>(*val)->rhs));
        break;
      case QType::Local:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Local *>(*val)->type));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Local *>(*val)->value));
        break;
      case QType::Global:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Global *>(*val)->type));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Global *>(*val)->value));
        break;
      case QType::Number:
      case QType::String:
      case QType::Char:
        break;
      case QType::List:
        for (auto &child : static_cast<List *>(*val)->values) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::Assign:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Assign *>(*val)->lhs));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Assign *>(*val)->rhs));
        break;
      case QType::PostInc:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<PostInc *>(*val)->lhs));
        break;
      case QType::PostDec:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<PostDec *>(*val)->lhs));
        break;
      case QType::AddressOf:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<AddressOf *>(*val)->lhs));
        break;
      case QType::Deref:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Deref *>(*val)->lhs));
        break;
      case QType::Member:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Member *>(*val)->lhs));
        queue.push(reinterpret_cast<Value **>(
            &static_cast<Member *>(*val)->field_type));
        break;
      case QType::Index:
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Index *>(*val)->lhs));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Index *>(*val)->index));
        queue.push(
            reinterpret_cast<Value **>(&static_cast<Index *>(*val)->type));
        break;
      case QType::IntrinsicCall:
        for (auto &child : static_cast<IntrinsicCall *>(*val)->args) {
          queue.push(reinterpret_cast<Value **>(&child));
        }
        break;
      case QType::IntrinsicType:
        break;
      case QType::EnumMax:
        break;
    }

    return false;
  };

  while (!queue.empty()) {
    Value **current = queue.front();
    queue.pop();

    if (!*current) {
      continue;
    }

    if (post_order) {
      if (!visited.contains(current)) {
        if (enqueue_children(current)) {
          return;  // Abort iteration
        }
        visited.insert(current);
        queue.push(current);
      } else {
        if (predicate) {
          switch (predicate(*current)) {
            case IterOp::Do:
              break;
            case IterOp::Skip:
              continue;
            case IterOp::Abort:
              return;  // Abort iteration
          }
        }
        func(current);
      }
    } else {
      if (enqueue_children(current)) {
        return;  // Abort iteration
      }
      if (predicate) {
        switch (predicate(*current)) {
          case IterOp::Do:
            break;
          case IterOp::Skip:
            continue;
          case IterOp::Abort:
            return;  // Abort iteration
        }
      }
      func(current);
    }
  }
}

Value *QModule::dfs_find_first(QModule::MatchFunc match,
                               QModule::PredFunc predicate, bool post_order) {
  /// TODO: Test this code !!!
  Value *result = nullptr;

  auto pred = [&result, predicate, match](Value *val) -> IterOp {
    if (predicate) {
      switch (predicate(val)) {
        case IterOp::Do:
          break;
        case IterOp::Skip:
          return IterOp::Skip;
        case IterOp::Abort:
          return IterOp::Abort;
      }
    }

    if (match(val)) {
      result = val;
      return IterOp::Abort;
    }

    return IterOp::Do;
  };

  dft_iter([&](Value **) {}, pred, post_order);

  return result;
}

Value *QModule::bfs_find_first(QModule::MatchFunc match,
                               QModule::PredFunc predicate, bool post_order) {
  /// TODO: Test this code !!!
  Value *result = nullptr;

  auto pred = [&result, predicate, match](Value *val) -> IterOp {
    if (predicate) {
      switch (predicate(val)) {
        case IterOp::Do:
          break;
        case IterOp::Skip:
          return IterOp::Skip;
        case IterOp::Abort:
          return IterOp::Abort;
      }
    }

    if (match(val)) {
      result = val;
      return IterOp::Abort;
    }

    return IterOp::Do;
  };

  bft_iter([&](Value **) {}, pred, post_order);

  return result;
}

void QModule::dfs_find_all(QModule::MatchFunc match,
                           std::vector<Value *> &results,
                           QModule::PredFunc predicate, bool post_order) {
  assert(match != nullptr);

  auto call = [&results, match](Value **val) {
    if (match(*val)) {
      results.push_back(*val);
    }
  };

  dft_iter(call, predicate, post_order);
}

void QModule::bfs_find_all(QModule::MatchFunc match,
                           std::vector<Value *> &results,
                           QModule::PredFunc predicate, bool post_order) {
  assert(match != nullptr);

  auto call = [&results, match](Value **val) {
    if (match(*val)) {
      results.push_back(*val);
    }
  };

  bft_iter(call, predicate, post_order);
}

void QModule::dfs_find_all(QModule::MatchFunc match,
                           std::vector<Value **> &results,
                           QModule::PredFunc predicate, bool post_order) {
  assert(match != nullptr);

  auto call = [&results, match](Value **val) {
    if (match(*val)) {
      results.push_back(val);
    }
  };

  dft_iter(call, predicate, post_order);
}

void QModule::bfs_find_all(QModule::MatchFunc match,
                           std::vector<Value **> &results,
                           QModule::PredFunc predicate, bool post_order) {
  assert(match != nullptr);

  auto call = [&results, match](Value **val) {
    if (match(*val)) {
      results.push_back(val);
    }
  };

  bft_iter(call, predicate, post_order);
}