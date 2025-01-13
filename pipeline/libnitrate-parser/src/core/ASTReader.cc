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

std::optional<FlowPtr<Base>> AST_Reader::get() {
  if (!m_root) {
    if (auto root = deserialize_object()) {
      m_root = root.value();
    }
  }

  return m_root;
}

std::optional<AST_Reader::LocationRange> AST_Reader::Read_LocationRange() {
  if (!next_if<std::string>("loc")) {
    return std::nullopt;
  }

  LocationRange range;

  if (next_if<none>()) {
    return range;
  }

  /// TODO: Implement deserialization for LocationRange
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::deserialize_object() {
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
    return nullptr;
  }

  auto it = node_kinds_map.find(next<std::string>());
  if (it == node_kinds_map.end()) {
    return nullptr;
  }

  auto range = Read_LocationRange();

  NullableFlowPtr<Base> R;

  switch (it->second) {
    case QAST_BASE: {
      R = ReadKind_Node();
      break;
    }

    case QAST_BINEXPR: {
      R = ReadKind_Binexpr();
      break;
    }

    case QAST_UNEXPR: {
      R = ReadKind_Unexpr();
      break;
    }

    case QAST_TEREXPR: {
      R = ReadKind_Terexpr();
      break;
    }

    case QAST_INT: {
      R = ReadKind_Int();
      break;
    }

    case QAST_FLOAT: {
      R = ReadKind_Float();
      break;
    }

    case QAST_STRING: {
      R = ReadKind_String();
      break;
    }

    case QAST_CHAR: {
      R = ReadKind_Char();
      break;
    }

    case QAST_BOOL: {
      R = ReadKind_Bool();
      break;
    }

    case QAST_NULL: {
      R = ReadKind_Null();
      break;
    }

    case QAST_UNDEF: {
      R = ReadKind_Undef();
      break;
    }

    case QAST_CALL: {
      R = ReadKind_Call();
      break;
    }

    case QAST_LIST: {
      R = ReadKind_List();
      break;
    }

    case QAST_ASSOC: {
      R = ReadKind_Assoc();
      break;
    }

    case QAST_INDEX: {
      R = ReadKind_Index();
      break;
    }

    case QAST_SLICE: {
      R = ReadKind_Slice();
      break;
    }

    case QAST_FSTRING: {
      R = ReadKind_Fstring();
      break;
    }

    case QAST_IDENT: {
      R = ReadKind_Ident();
      break;
    }

    case QAST_SEQ: {
      R = ReadKind_SeqPoint();
      break;
    }

    case QAST_POST_UNEXPR: {
      R = ReadKind_PostUnexpr();
      break;
    }

    case QAST_SEXPR: {
      R = ReadKind_StmtExpr();
      break;
    }

    case QAST_TEXPR: {
      R = ReadKind_TypeExpr();
      break;
    }

    case QAST_TEMPL_CALL: {
      R = ReadKind_TemplCall();
      break;
    }

    case QAST_REF: {
      R = ReadKind_Ref();
      break;
    }

    case QAST_U1: {
      R = ReadKind_U1();
      break;
    }

    case QAST_U8: {
      R = ReadKind_U8();
      break;
    }

    case QAST_U16: {
      R = ReadKind_U16();
      break;
    }

    case QAST_U32: {
      R = ReadKind_U32();
      break;
    }

    case QAST_U64: {
      R = ReadKind_U64();
      break;
    }

    case QAST_U128: {
      R = ReadKind_U128();
      break;
    }

    case QAST_I8: {
      R = ReadKind_I8();
      break;
    }

    case QAST_I16: {
      R = ReadKind_I16();
      break;
    }

    case QAST_I32: {
      R = ReadKind_I32();
      break;
    }

    case QAST_I64: {
      R = ReadKind_I64();
      break;
    }

    case QAST_I128: {
      R = ReadKind_I128();
      break;
    }

    case QAST_F16: {
      R = ReadKind_F16();
      break;
    }

    case QAST_F32: {
      R = ReadKind_F32();
      break;
    }

    case QAST_F64: {
      R = ReadKind_F64();
      break;
    }

    case QAST_F128: {
      R = ReadKind_F128();
      break;
    }

    case QAST_VOID: {
      R = ReadKind_Void();
      break;
    }

    case QAST_PTR: {
      R = ReadKind_Ptr();
      break;
    }

    case QAST_OPAQUE: {
      R = ReadKind_Opaque();
      break;
    }

    case QAST_ARRAY: {
      R = ReadKind_Array();
      break;
    }

    case QAST_TUPLE: {
      R = ReadKind_Tuple();
      break;
    }

    case QAST_FUNCTOR: {
      R = ReadKind_FuncTy();
      break;
    }

    case QAST_NAMED: {
      R = ReadKind_Unres();
      break;
    }

    case QAST_INFER: {
      R = ReadKind_Infer();
      break;
    }

    case QAST_TEMPLATE: {
      R = ReadKind_Templ();
      break;
    }

    case QAST_TYPEDEF: {
      R = ReadKind_Typedef();
      break;
    }

    case QAST_STRUCT: {
      R = ReadKind_Struct();
      break;
    }

    case QAST_ENUM: {
      R = ReadKind_Enum();
      break;
    }

    case QAST_FUNCTION: {
      R = ReadKind_Function();
      break;
    }

    case QAST_SCOPE: {
      R = ReadKind_Scope();
      break;
    }

    case QAST_EXPORT: {
      R = ReadKind_Export();
      break;
    }

    case QAST_BLOCK: {
      R = ReadKind_Block();
      break;
    }

    case QAST_VAR: {
      R = ReadKind_Let();
      break;
    }

    case QAST_INLINE_ASM: {
      R = ReadKind_InlineAsm();
      break;
    }

    case QAST_RETURN: {
      R = ReadKind_Return();
      break;
    }

    case QAST_RETIF: {
      R = ReadKind_Retif();
      break;
    }

    case QAST_BREAK: {
      R = ReadKind_Break();
      break;
    }

    case QAST_CONTINUE: {
      R = ReadKind_Continue();
      break;
    }

    case QAST_IF: {
      R = ReadKind_If();
      break;
    }

    case QAST_WHILE: {
      R = ReadKind_While();
      break;
    }

    case QAST_FOR: {
      R = ReadKind_For();
      break;
    }

    case QAST_FOREACH: {
      R = ReadKind_Foreach();
      break;
    }

    case QAST_CASE: {
      R = ReadKind_Case();
      break;
    }

    case QAST_SWITCH: {
      R = ReadKind_Switch();
      break;
    }

    case QAST_ESTMT: {
      R = ReadKind_ExprStmt();
      break;
    }
  }

  bool can_save_source_location =
      m_source.has_value() && R.has_value() && range.has_value();

  if (can_save_source_location) {
    auto begid = m_source.value().get().InternLocation(range->start),
         endid = m_source.value().get().InternLocation(range->end);

    R.value()->setLoc(begid, endid);
  }

  return R;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Node() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Binexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Unexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Terexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Int() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Float() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_String() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Char() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Bool() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Null() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Undef() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Call() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_List() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Assoc() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Index() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Slice() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Fstring() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Ident() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_SeqPoint() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_PostUnexpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_StmtExpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_TypeExpr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_TemplCall() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Ref() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U1() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U8() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U16() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U32() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U64() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U128() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I8() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I16() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I32() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I64() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I128() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F16() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F32() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F64() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F128() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Void() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Ptr() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Opaque() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Array() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Tuple() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_FuncTy() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Unres() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Infer() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Templ() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Typedef() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Struct() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Enum() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Function() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Scope() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Export() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Block() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Let() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_InlineAsm() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Return() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Retif() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Break() {
  return make<BreakStmt>()();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Continue() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_If() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_While() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_For() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Foreach() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Case() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Switch() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_ExprStmt() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}
