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

#define AST_READER_IMPL

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTReader.hh>

using namespace ncc;
using namespace ncc::parse;

void AST_Reader::push_value(Value&& value) {
  bool inside_array = m_stack.top().first == Mode::InsideArray;
  if (inside_array) {
    if (!m_stack.top().second.empty()) {
      std::vector<Value>& vector =
          std::get<std::vector<Value>>(m_stack.top().second.back()());
      vector.push_back(std::move(value));
    }
  } else {
    m_stack.top().second.push(std::move(value));
  }
}

std::optional<FlowPtr<Base>> AST_Reader::get() {
  if (!m_stack.top().second.empty()) {
    const auto& back = m_stack.top().second.back()();
    if (std::holds_alternative<FlowPtr<Base>>(back)) [[likely]] {
      return std::get<FlowPtr<Base>>(back);
    }
  }

  return std::nullopt;
}

void AST_Reader::str(std::string_view val) { push_value(std::string(val)); }
void AST_Reader::uint(uint64_t val) { push_value(val); }
void AST_Reader::dbl(double val) { push_value(val); }
void AST_Reader::boolean(bool val) { push_value(val); }
void AST_Reader::null() { push_value(nullptr); }

void AST_Reader::begin_obj(size_t) { m_stack.push(NewObjectState); }

void AST_Reader::end_obj() {
  auto object = deserialize_object();

  // We ensure here that the stack is never
  // empty in order to remove checks from the other functions.
  if (m_stack.size() >= 2) [[likely]] {
    m_stack.pop();
  }

  m_stack.top().second.push(object);
}

void AST_Reader::begin_arr(size_t size) {
  std::vector<Value> array;
  array.reserve(size);

  m_stack.top().second.push(std::move(array));
  m_stack.top().first = Mode::InsideArray;
}

void AST_Reader::end_arr() { m_stack.top().first = Mode::NotInsideArray; }

FlowPtr<Base> AST_Reader::deserialize_object() {
  static Base BadNode(QAST_BASE, true);
  static FlowPtr<Base> Bad(&BadNode);

  // This code must be the reverse of the map contained in:
  // 'constexpr std::string_view Base::getKindName(npar_ty_t type)'
  static const std::unordered_map<std::string, npar_ty_t> node_kinds_map = {
      {"Node", QAST_BASE},
      {"Binexpr", QAST_BINEXPR},
      {"Unexpr", QAST_UNEXPR},
      {"Terexpr", QAST_TEREXPR},
      {"Int", QAST_INT},
      {"Float", QAST_FLOAT},
      {"String", QAST_STRING},
      {"Char", QAST_CHAR},
      {"Bool", QAST_BOOL},
      {"Null", QAST_NULL},
      {"Undef", QAST_UNDEF},
      {"Call", QAST_CALL},
      {"List", QAST_LIST},
      {"Assoc", QAST_ASSOC},
      {"Index", QAST_INDEX},
      {"Slice", QAST_SLICE},
      {"Fstring", QAST_FSTRING},
      {"Ident", QAST_IDENT},
      {"SeqPoint", QAST_SEQ},
      {"PostUnexpr", QAST_POST_UNEXPR},
      {"StmtExpr", QAST_SEXPR},
      {"TypeExpr", QAST_TEXPR},
      {"TemplCall", QAST_TEMPL_CALL},
      {"Ref", QAST_REF},
      {"U1", QAST_U1},
      {"U8", QAST_U8},
      {"U16", QAST_U16},
      {"U32", QAST_U32},
      {"U64", QAST_U64},
      {"U128", QAST_U128},
      {"I8", QAST_I8},
      {"I16", QAST_I16},
      {"I32", QAST_I32},
      {"I64", QAST_I64},
      {"I128", QAST_I128},
      {"F16", QAST_F16},
      {"F32", QAST_F32},
      {"F64", QAST_F64},
      {"F128", QAST_F128},
      {"Void", QAST_VOID},
      {"Ptr", QAST_PTR},
      {"Opaque", QAST_OPAQUE},
      {"Array", QAST_ARRAY},
      {"Tuple", QAST_TUPLE},
      {"FuncTy", QAST_FUNCTOR},
      {"Unres", QAST_NAMED},
      {"Infer", QAST_INFER},
      {"Templ", QAST_TEMPLATE},
      {"Typedef", QAST_TYPEDEF},
      {"Struct", QAST_STRUCT},
      {"Enum", QAST_ENUM},
      {"Function", QAST_FUNCTION},
      {"Scope", QAST_SCOPE},
      {"Export", QAST_EXPORT},
      {"Block", QAST_BLOCK},
      {"Let", QAST_VAR},
      {"InlineAsm", QAST_INLINE_ASM},
      {"Return", QAST_RETURN},
      {"Retif", QAST_RETIF},
      {"Break", QAST_BREAK},
      {"Continue", QAST_CONTINUE},
      {"If", QAST_IF},
      {"While", QAST_WHILE},
      {"For", QAST_FOR},
      {"Foreach", QAST_FOREACH},
      {"Case", QAST_CASE},
      {"Switch", QAST_SWITCH},
      {"ExprStmt", QAST_ESTMT},
  };

  if (!next_if<std::string>("kind") || !next_is<std::string>()) {
    return Bad;
  }

  auto it = node_kinds_map.find(next<std::string>());
  if (it == node_kinds_map.end()) {
    return Bad;
  }

  switch (it->second) {
    case QAST_BASE: {
      return ReadKind_Node();
    }

    case QAST_BINEXPR: {
      return ReadKind_Binexpr();
    }

    case QAST_UNEXPR: {
      return ReadKind_Unexpr();
    }

    case QAST_TEREXPR: {
      return ReadKind_Terexpr();
    }

    case QAST_INT: {
      return ReadKind_Int();
    }

    case QAST_FLOAT: {
      return ReadKind_Float();
    }

    case QAST_STRING: {
      return ReadKind_String();
    }

    case QAST_CHAR: {
      return ReadKind_Char();
    }

    case QAST_BOOL: {
      return ReadKind_Bool();
    }

    case QAST_NULL: {
      return ReadKind_Null();
    }

    case QAST_UNDEF: {
      return ReadKind_Undef();
    }

    case QAST_CALL: {
      return ReadKind_Call();
    }

    case QAST_LIST: {
      return ReadKind_List();
    }

    case QAST_ASSOC: {
      return ReadKind_Assoc();
    }

    case QAST_INDEX: {
      return ReadKind_Index();
    }

    case QAST_SLICE: {
      return ReadKind_Slice();
    }

    case QAST_FSTRING: {
      return ReadKind_Fstring();
    }

    case QAST_IDENT: {
      return ReadKind_Ident();
    }

    case QAST_SEQ: {
      return ReadKind_SeqPoint();
    }

    case QAST_POST_UNEXPR: {
      return ReadKind_PostUnexpr();
    }

    case QAST_SEXPR: {
      return ReadKind_StmtExpr();
    }

    case QAST_TEXPR: {
      return ReadKind_TypeExpr();
    }

    case QAST_TEMPL_CALL: {
      return ReadKind_TemplCall();
    }

    case QAST_REF: {
      return ReadKind_Ref();
    }

    case QAST_U1: {
      return ReadKind_U1();
    }

    case QAST_U8: {
      return ReadKind_U8();
    }

    case QAST_U16: {
      return ReadKind_U16();
    }

    case QAST_U32: {
      return ReadKind_U32();
    }

    case QAST_U64: {
      return ReadKind_U64();
    }

    case QAST_U128: {
      return ReadKind_U128();
    }

    case QAST_I8: {
      return ReadKind_I8();
    }

    case QAST_I16: {
      return ReadKind_I16();
    }

    case QAST_I32: {
      return ReadKind_I32();
    }

    case QAST_I64: {
      return ReadKind_I64();
    }

    case QAST_I128: {
      return ReadKind_I128();
    }

    case QAST_F16: {
      return ReadKind_F16();
    }

    case QAST_F32: {
      return ReadKind_F32();
    }

    case QAST_F64: {
      return ReadKind_F64();
    }

    case QAST_F128: {
      return ReadKind_F128();
    }

    case QAST_VOID: {
      return ReadKind_Void();
    }

    case QAST_PTR: {
      return ReadKind_Ptr();
    }

    case QAST_OPAQUE: {
      return ReadKind_Opaque();
    }

    case QAST_ARRAY: {
      return ReadKind_Array();
    }

    case QAST_TUPLE: {
      return ReadKind_Tuple();
    }

    case QAST_FUNCTOR: {
      return ReadKind_FuncTy();
    }

    case QAST_NAMED: {
      return ReadKind_Unres();
    }

    case QAST_INFER: {
      return ReadKind_Infer();
    }

    case QAST_TEMPLATE: {
      return ReadKind_Templ();
    }

    case QAST_TYPEDEF: {
      return ReadKind_Typedef();
    }

    case QAST_STRUCT: {
      return ReadKind_Struct();
    }

    case QAST_ENUM: {
      return ReadKind_Enum();
    }

    case QAST_FUNCTION: {
      return ReadKind_Function();
    }

    case QAST_SCOPE: {
      return ReadKind_Scope();
    }

    case QAST_EXPORT: {
      return ReadKind_Export();
    }

    case QAST_BLOCK: {
      return ReadKind_Block();
    }

    case QAST_VAR: {
      return ReadKind_Let();
    }

    case QAST_INLINE_ASM: {
      return ReadKind_InlineAsm();
    }

    case QAST_RETURN: {
      return ReadKind_Return();
    }

    case QAST_RETIF: {
      return ReadKind_Retif();
    }

    case QAST_BREAK: {
      return ReadKind_Break();
    }

    case QAST_CONTINUE: {
      return ReadKind_Continue();
    }

    case QAST_IF: {
      return ReadKind_If();
    }

    case QAST_WHILE: {
      return ReadKind_While();
    }

    case QAST_FOR: {
      return ReadKind_For();
    }

    case QAST_FOREACH: {
      return ReadKind_Foreach();
    }

    case QAST_CASE: {
      return ReadKind_Case();
    }

    case QAST_SWITCH: {
      return ReadKind_Switch();
    }

    case QAST_ESTMT: {
      return ReadKind_ExprStmt();
    }
  }
}

FlowPtr<Base> AST_Reader::ReadKind_Node() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Binexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Unexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Terexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Int() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Float() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_String() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Char() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Bool() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Null() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Undef() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Call() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_List() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Assoc() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Index() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Slice() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Fstring() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Ident() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_SeqPoint() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_PostUnexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_StmtExpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_TypeExpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_TemplCall() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Ref() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_U1() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_U8() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_U16() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_U32() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_U64() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_U128() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_I8() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_I16() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_I32() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_I64() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_I128() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_F16() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_F32() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_F64() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_F128() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Void() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Ptr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Opaque() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Array() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Tuple() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_FuncTy() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Unres() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Infer() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Templ() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Typedef() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Struct() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Enum() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Function() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Scope() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Export() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Block() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Let() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_InlineAsm() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Return() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Retif() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Break() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Continue() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_If() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_While() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_For() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Foreach() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Case() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_Switch() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

FlowPtr<Base> AST_Reader::ReadKind_ExprStmt() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}
