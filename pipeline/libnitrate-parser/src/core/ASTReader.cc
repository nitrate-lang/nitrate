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

#include <boost/multiprecision/cpp_int.hpp>
#include <charconv>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTReader.hh>
#include <type_traits>

using namespace ncc;
using namespace ncc::parse;
using namespace boost::multiprecision;

template <typename Ty,
          typename = std::enable_if_t<std::is_floating_point_v<Ty>>>
static inline bool strict_from_chars(
    const char* first, const char* last, Ty& value,
    std::chars_format fmt = std::chars_format::general) noexcept {
  auto res = std::from_chars(first, last, value, fmt);

  return res.ec == std::errc() && res.ptr == last;
}

template <typename Ty>
static inline bool strict_from_chars(const char* first, const char* last,
                                     Ty& value, int base = 10) noexcept {
  auto res = std::from_chars(first, last, value, base);

  return res.ec == std::errc() && res.ptr == last;
}

std::optional<FlowPtr<Base>> AST_Reader::get() {
  if (!m_root) {
    if (auto root = deserialize_object()) {
      m_root = root.value();
    }
  }

  return m_root;
}

std::optional<AST_Reader::LocationRange> AST_Reader::Read_LocationRange() {
  const auto ParseLocationObject = [&]() -> std::optional<lex::Location> {
    if (!next_if<std::string>("off") || !next_is<uint64_t>()) {
      return std::nullopt;
    }

    uint32_t off = next<uint64_t>();

    if (!next_if<std::string>("row") || !next_is<uint64_t>()) {
      return std::nullopt;
    }

    uint32_t row = next<uint64_t>();

    if (!next_if<std::string>("col") || !next_is<uint64_t>()) {
      return std::nullopt;
    }

    uint32_t col = next<uint64_t>();

    if (!next_if<std::string>("src") || !next_is<std::string>()) {
      return std::nullopt;
    }

    std::string src = next<std::string>();

    return lex::Location(off, row, col, src);
  };

  if (!next_if<std::string>("loc")) {
    return std::nullopt;
  }

  LocationRange range;

  if (next_if<none>()) {
    return range;
  }

  if (!next_if<std::string>("begin")) {
    return std::nullopt;
  }

  if (auto begin = ParseLocationObject()) {
    range.start = begin.value();
  } else {
    return std::nullopt;
  }

  if (!next_if<std::string>("end")) {
    return std::nullopt;
  }

  if (auto end = ParseLocationObject()) {
    range.end = end.value();
  } else {
    return std::nullopt;
  }

  if (!next_if<std::string>("trace")) {
    return std::nullopt;
  }

  if (next_if<none>()) {
    return range;
  }

  if (!next_if<std::string>("src") || !next_is<std::string>()) {
    return std::nullopt;
  }

  next<std::string>();

  if (!next_if<std::string>("sub") || !next_is<std::string>()) {
    return std::nullopt;
  }

  next<std::string>();

  if (!next_if<std::string>("row") || !next_is<uint64_t>()) {
    return std::nullopt;
  }

  next<uint64_t>();

  if (!next_if<std::string>("col") || !next_is<uint64_t>()) {
    return std::nullopt;
  }

  next<uint64_t>();

  return range;
}

std::optional<AST_Reader::TypeMetadata> AST_Reader::Read_TypeMetadata() {
  AST_Reader::TypeMetadata info;

  if (!next_if<std::string>("width")) {
    return std::nullopt;
  }

  if (next_if<none>()) {
    info.width = nullptr;
  } else {
    auto width = deserialize_expression();
    if (!width.has_value()) {
      return std::nullopt;
    }

    info.width = width.value();
  }

  if (!next_if<std::string>("min")) {
    return std::nullopt;
  }

  if (next_if<none>()) {
    info.min = nullptr;
  } else {
    auto min = deserialize_expression();
    if (!min.has_value()) {
      return std::nullopt;
    }

    info.min = min.value();
  }

  if (!next_if<std::string>("max")) {
    return std::nullopt;
  }

  if (next_if<none>()) {
    info.max = nullptr;
  } else {
    auto max = deserialize_expression();
    if (!max.has_value()) {
      return std::nullopt;
    }

    info.max = max.value();
  }

  return info;
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

  if (!R.has_value()) {
    std::cout << "Failed to deserialize object of type: " << it->first
              << std::endl;
  }

  bool can_save_source_location =
      m_source.has_value() && R.has_value() && range.has_value();

  if (can_save_source_location) {
    auto start = m_source.value().get().InternLocation(range->start);
    auto end = m_source.value().get().InternLocation(range->end);

    R.value()->SetLoc(start, end);
  }

  return R;
}

NullableFlowPtr<Stmt> AST_Reader::deserialize_statement() {
  auto object = deserialize_object();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->getKind();
  if (kind < QAST__STMT_FIRST || kind > QAST__STMT_LAST) {
    return nullptr;
  }

  return object.value().as<Stmt>();
}

NullableFlowPtr<Expr> AST_Reader::deserialize_expression() {
  auto object = deserialize_object();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->getKind();
  if (kind < QAST__EXPR_FIRST || kind > QAST__EXPR_LAST) {
    return nullptr;
  }

  return object.value().as<Expr>();
}

NullableFlowPtr<Type> AST_Reader::deserialize_type() {
  auto object = deserialize_object();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->getKind();
  if (kind < QAST__TYPE_FIRST || kind > QAST__TYPE_LAST) {
    return nullptr;
  }

  return object.value().as<Type>();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Node() {
  return make<Base>(QAST_BASE)();
}

NullableFlowPtr<BinExpr> AST_Reader::ReadKind_Binexpr() {
  if (!next_if<std::string>("op") || !next_is<std::string>()) {
    return nullptr;
  }

  auto op = next<std::string>();

  auto op_it = lex::LEXICAL_OPERATORS.left.find(op);
  if (op_it == lex::LEXICAL_OPERATORS.left.end()) {
    return nullptr;
  }

  if (!next_if<std::string>("lhs")) {
    return nullptr;
  }

  auto lhs = deserialize_expression();
  if (!lhs.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("rhs")) {
    return nullptr;
  }

  auto rhs = deserialize_expression();
  if (!rhs.has_value()) {
    return nullptr;
  }

  return make<BinExpr>(lhs.value(), op_it->second, rhs.value())();
}

NullableFlowPtr<UnaryExpr> AST_Reader::ReadKind_Unexpr() {
  if (!next_if<std::string>("op") || !next_is<std::string>()) {
    return nullptr;
  }

  auto op = next<std::string>();

  auto op_it = lex::LEXICAL_OPERATORS.left.find(op);
  if (op_it == lex::LEXICAL_OPERATORS.left.end()) {
    return nullptr;
  }

  if (!next_if<std::string>("rhs")) {
    return nullptr;
  }

  auto rhs = deserialize_expression();
  if (!rhs.has_value()) {
    return nullptr;
  }

  return make<UnaryExpr>(op_it->second, rhs.value())();
}

NullableFlowPtr<PostUnaryExpr> AST_Reader::ReadKind_PostUnexpr() {
  if (!next_if<std::string>("op") || !next_is<std::string>()) {
    return nullptr;
  }

  auto op = next<std::string>();

  auto op_it = lex::LEXICAL_OPERATORS.left.find(op);
  if (op_it == lex::LEXICAL_OPERATORS.left.end()) {
    return nullptr;
  }

  if (!next_if<std::string>("lhs")) {
    return nullptr;
  }

  auto lhs = deserialize_expression();
  if (!lhs.has_value()) {
    return nullptr;
  }

  return make<PostUnaryExpr>(lhs.value(), op_it->second)();
}

NullableFlowPtr<TernaryExpr> AST_Reader::ReadKind_Terexpr() {
  if (!next_if<std::string>("cond")) {
    return nullptr;
  }

  auto cond = deserialize_expression();
  if (!cond.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("lhs")) {
    return nullptr;
  }

  auto lhs = deserialize_expression();
  if (!lhs.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("rhs")) {
    return nullptr;
  }

  auto rhs = deserialize_expression();
  if (!rhs.has_value()) {
    return nullptr;
  }

  return make<TernaryExpr>(cond.value(), lhs.value(), rhs.value())();
}

NullableFlowPtr<ConstInt> AST_Reader::ReadKind_Int() {
  if (!next_if<std::string>("value") || !next_is<std::string>()) {
    return nullptr;
  }

  auto value = next<std::string>();

  /* Ensure boost won't call std::terminate */
  bool all_digits = std::all_of(value.begin(), value.end(), ::isdigit);
  if (!all_digits) {
    return nullptr;
  }

  /* Ensure the value is within the bounds of a unsigned 128-bit integer */
  if (cpp_int(value) > cpp_int("340282366920938463463374607431768211455")) {
    return nullptr;
  }

  return make<ConstInt>(std::move(value))();
}

NullableFlowPtr<ConstFloat> AST_Reader::ReadKind_Float() {
  if (!next_if<std::string>("value") || !next_is<std::string>()) {
    return nullptr;
  }

  auto value = next<std::string>();

  long double d = 0;
  if (!strict_from_chars(value.data(), value.data() + value.size(), d,
                         std::chars_format::fixed)) {
    return nullptr;
  }

  return make<ConstFloat>(std::move(value))();
}

NullableFlowPtr<ConstString> AST_Reader::ReadKind_String() {
  if (!next_if<std::string>("value") || !next_is<std::string>()) {
    return nullptr;
  }

  auto value = next<std::string>();

  return make<ConstString>(std::move(value))();
}

NullableFlowPtr<ConstChar> AST_Reader::ReadKind_Char() {
  if (!next_if<std::string>("value") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto value = next<uint64_t>();

  if (value > 255) {
    return nullptr;
  }

  return make<ConstChar>(value)();
}

NullableFlowPtr<ConstBool> AST_Reader::ReadKind_Bool() {
  if (!next_if<std::string>("value") || !next_is<bool>()) {
    return nullptr;
  }

  auto value = next<bool>();

  return make<ConstBool>(value)();
}

NullableFlowPtr<ConstNull> AST_Reader::ReadKind_Null() {
  return make<ConstNull>()();
}

NullableFlowPtr<ConstUndef> AST_Reader::ReadKind_Undef() {
  return make<ConstUndef>()();
}

NullableFlowPtr<Call> AST_Reader::ReadKind_Call() {
  if (!next_if<std::string>("callee")) {
    return nullptr;
  }

  auto callee = deserialize_expression();
  if (!callee.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("arguments") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto argument_count = next<uint64_t>();

  CallArgs arguments;
  arguments.reserve(argument_count);

  while (argument_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto name = next<std::string>();

    if (!next_if<std::string>("value")) {
      return nullptr;
    }

    auto value = deserialize_expression();
    if (!value.has_value()) {
      return nullptr;
    }

    arguments.emplace_back(std::move(name), value.value());
  }

  return make<Call>(callee.value(), std::move(arguments))();
}

NullableFlowPtr<List> AST_Reader::ReadKind_List() {
  if (!next_if<std::string>("elements") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto element_count = next<uint64_t>();

  ExpressionList elements;
  elements.reserve(element_count);

  while (element_count--) {
    auto element = deserialize_expression();
    if (!element.has_value()) {
      return nullptr;
    }

    elements.push_back(element.value());
  }

  return make<List>(std::move(elements))();
}

NullableFlowPtr<Assoc> AST_Reader::ReadKind_Assoc() {
  if (!next_if<std::string>("key")) {
    return nullptr;
  }

  auto key = deserialize_expression();
  if (!key.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("value")) {
    return nullptr;
  }

  auto value = deserialize_expression();
  if (!value.has_value()) {
    return nullptr;
  }

  return make<Assoc>(key.value(), value.value())();
}

NullableFlowPtr<Index> AST_Reader::ReadKind_Index() {
  if (!next_if<std::string>("base")) {
    return nullptr;
  }

  auto base = deserialize_expression();
  if (!base.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("index")) {
    return nullptr;
  }

  auto index = deserialize_expression();

  return make<Index>(base.value(), index.value())();
}

NullableFlowPtr<Slice> AST_Reader::ReadKind_Slice() {
  if (!next_if<std::string>("base")) {
    return nullptr;
  }

  auto base = deserialize_expression();
  if (!base.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("start")) {
    return nullptr;
  }

  auto start = deserialize_expression();
  if (!start.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("end")) {
    return nullptr;
  }

  auto end = deserialize_expression();
  if (!end.has_value()) {
    return nullptr;
  }

  return make<Slice>(base.value(), start.value(), end.value())();
}

NullableFlowPtr<FString> AST_Reader::ReadKind_Fstring() {
  if (!next_if<std::string>("terms") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto term_count = next<uint64_t>();
  FStringItems terms;
  terms.reserve(term_count);

  while (term_count--) {
    if (next_if<std::string>("value") && next_is<std::string>()) {
      auto value = next<std::string>();
      terms.emplace_back(value);
    } else {
      auto term = deserialize_expression();
      if (!term.has_value()) {
        return nullptr;
      }

      terms.emplace_back(term.value());
    }
  }

  return make<FString>(std::move(terms))();
}

NullableFlowPtr<Ident> AST_Reader::ReadKind_Ident() {
  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  return make<Ident>(std::move(name))();
}

NullableFlowPtr<SeqPoint> AST_Reader::ReadKind_SeqPoint() {
  if (!next_if<std::string>("terms") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto expression_count = next<uint64_t>();

  ExpressionList terms;
  terms.reserve(expression_count);

  while (expression_count--) {
    auto term = deserialize_expression();
    if (!term.has_value()) {
      return nullptr;
    }

    terms.push_back(term.value());
  }

  return make<SeqPoint>(std::move(terms))();
}

NullableFlowPtr<StmtExpr> AST_Reader::ReadKind_StmtExpr() {
  if (!next_if<std::string>("stmt")) {
    return nullptr;
  }

  auto stmt = deserialize_statement();
  if (!stmt.has_value()) {
    return nullptr;
  }

  return make<StmtExpr>(stmt.value())();
}

NullableFlowPtr<TypeExpr> AST_Reader::ReadKind_TypeExpr() {
  if (!next_if<std::string>("type")) {
    return nullptr;
  }

  auto type = deserialize_type();
  if (!type.has_value()) {
    return nullptr;
  }

  return make<TypeExpr>(type.value())();
}

NullableFlowPtr<TemplCall> AST_Reader::ReadKind_TemplCall() {
  if (!next_if<std::string>("callee")) {
    return nullptr;
  }

  auto callee = deserialize_expression();
  if (!callee.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("template") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto template_count = next<uint64_t>();

  CallArgs template_args;
  template_args.reserve(template_count);

  while (template_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto name = next<std::string>();

    if (!next_if<std::string>("value")) {
      return nullptr;
    }

    auto value = deserialize_expression();
    if (!value.has_value()) {
      return nullptr;
    }

    template_args.emplace_back(std::move(name), value.value());
  }

  if (!next_if<std::string>("arguments") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto argument_count = next<uint64_t>();

  CallArgs arguments;
  arguments.reserve(argument_count);

  while (argument_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto name = next<std::string>();

    if (!next_if<std::string>("value")) {
      return nullptr;
    }

    auto value = deserialize_expression();
    if (!value.has_value()) {
      return nullptr;
    }

    arguments.emplace_back(std::move(name), value.value());
  }

  return make<TemplCall>(callee.value(), std::move(arguments),
                         std::move(template_args))();
}

NullableFlowPtr<U1> AST_Reader::ReadKind_U1() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U1>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<U8> AST_Reader::ReadKind_U8() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U8>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<U16> AST_Reader::ReadKind_U16() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U16>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<U32> AST_Reader::ReadKind_U32() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U32>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<U64> AST_Reader::ReadKind_U64() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U64>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<U128> AST_Reader::ReadKind_U128() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U128>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<I8> AST_Reader::ReadKind_I8() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I8>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<I16> AST_Reader::ReadKind_I16() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I16>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<I32> AST_Reader::ReadKind_I32() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I32>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<I64> AST_Reader::ReadKind_I64() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I64>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<I128> AST_Reader::ReadKind_I128() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I128>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<F16> AST_Reader::ReadKind_F16() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F16>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<F32> AST_Reader::ReadKind_F32() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F32>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<F64> AST_Reader::ReadKind_F64() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F64>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<F128> AST_Reader::ReadKind_F128() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F128>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<VoidTy> AST_Reader::ReadKind_Void() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<VoidTy>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<RefTy> AST_Reader::ReadKind_Ref() {
  auto info = Read_TypeMetadata();

  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("to")) {
    return nullptr;
  }

  auto to = deserialize_type();
  if (!to.has_value()) {
    return nullptr;
  }

  auto node = make<RefTy>(to.value())();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<PtrTy> AST_Reader::ReadKind_Ptr() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("volatile") || !next_is<bool>()) {
    return nullptr;
  }

  bool is_volatile = next<bool>();

  if (!next_if<std::string>("to")) {
    return nullptr;
  }

  auto to = deserialize_type();
  if (!to.has_value()) {
    return nullptr;
  }

  auto node = make<PtrTy>(to.value(), is_volatile)();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<OpaqueTy> AST_Reader::ReadKind_Opaque() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  auto node = make<OpaqueTy>(name)();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<ArrayTy> AST_Reader::ReadKind_Array() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("of")) {
    return nullptr;
  }

  auto of = deserialize_type();
  if (!of.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("size")) {
    return nullptr;
  }

  auto size = deserialize_expression();
  if (!size.has_value()) {
    return nullptr;
  }

  auto node = make<ArrayTy>(of.value(), size.value())();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<TupleTy> AST_Reader::ReadKind_Tuple() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("fields") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto field_count = next<uint64_t>();

  TupleTyItems fields;
  fields.reserve(field_count);

  while (field_count--) {
    auto field = deserialize_type();
    if (!field.has_value()) {
      return nullptr;
    }

    fields.push_back(field.value());
  }

  auto node = make<TupleTy>(std::move(fields))();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<FuncTy> AST_Reader::ReadKind_FuncTy() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("attributes") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count--) {
    auto attribute = deserialize_expression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!next_if<std::string>("return")) {
    return nullptr;
  }

  auto return_type = deserialize_type();
  if (!return_type.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("thread_safe") || !next_is<bool>()) {
    return nullptr;
  }

  auto thread_safe = next<bool>();

  if (!next_if<std::string>("purity")) {
    return nullptr;
  }

  Purity purity;
  if (next_if<std::string>("impure")) {
    purity = thread_safe ? Purity::Impure_TSafe : Purity::Impure;
  } else if (next_if<std::string>("pure")) {
    purity = Purity::Pure;
  } else if (next_if<std::string>("quasi")) {
    purity = Purity::Quasi;
  } else if (next_if<std::string>("retro")) {
    purity = Purity::Retro;
  } else {
    return nullptr;
  }

  if (!next_if<std::string>("input")) {
    return nullptr;
  }

  if (!next_if<std::string>("variadic") || !next_is<bool>()) {
    return nullptr;
  }

  auto variadic = next<bool>();

  if (!next_if<std::string>("parameters") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto parameter_count = next<uint64_t>();

  FuncParams parameters;
  parameters.reserve(parameter_count);

  while (parameter_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto name = next<std::string>();

    if (!next_if<std::string>("type")) {
      return nullptr;
    }

    auto type = deserialize_type();
    if (!type.has_value()) {
      return nullptr;
    }

    if (!next_if<std::string>("default")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> default_value;
    if (next_if<none>()) {
      default_value = nullptr;
    } else {
      default_value = deserialize_expression();
      if (!default_value.has_value()) {
        return nullptr;
      }
    }

    parameters.emplace_back(std::move(name), type.value(), default_value);
  }

  auto node = make<FuncTy>(return_type.value(), parameters, variadic, purity,
                           attributes)();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<NamedTy> AST_Reader::ReadKind_Unres() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  auto node = make<NamedTy>(name)();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<InferTy> AST_Reader::ReadKind_Infer() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<InferTy>()();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<TemplType> AST_Reader::ReadKind_Templ() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("template")) {
    return nullptr;
  }

  auto templ = deserialize_type();
  if (!templ.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("arguments") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto argument_count = next<uint64_t>();

  CallArgs arguments;
  arguments.reserve(argument_count);

  while (argument_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto name = next<std::string>();

    if (!next_if<std::string>("value")) {
      return nullptr;
    }

    auto value = deserialize_expression();
    if (!value.has_value()) {
      return nullptr;
    }

    arguments.emplace_back(std::move(name), value.value());
  }

  auto node = make<TemplType>(templ.value(), std::move(arguments))();
  node->SetWidth(info->width);
  node->SetRangeBegin(info->min);
  node->SetRangeEnd(info->max);

  return node;
}

NullableFlowPtr<TypedefStmt> AST_Reader::ReadKind_Typedef() {
  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  if (!next_if<std::string>("type")) {
    return nullptr;
  }

  auto type = deserialize_type();
  if (!type.has_value()) {
    return nullptr;
  }

  return make<TypedefStmt>(name, type.value())();
}

NullableFlowPtr<StructDef> AST_Reader::ReadKind_Struct() {
  if (!next_if<std::string>("mode")) {
    return nullptr;
  }

  CompositeType mode;

  if (next_if<std::string>("region")) {
    mode = CompositeType::Region;
  } else if (next_if<std::string>("struct")) {
    mode = CompositeType::Struct;
  } else if (next_if<std::string>("group")) {
    mode = CompositeType::Group;
  } else if (next_if<std::string>("class")) {
    mode = CompositeType::Class;
  } else if (next_if<std::string>("union")) {
    mode = CompositeType::Union;
  } else {
    return nullptr;
  }

  if (!next_if<std::string>("attributes") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count--) {
    auto attribute = deserialize_expression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  if (!next_if<std::string>("template")) {
    return nullptr;
  }

  std::optional<TemplateParameters> template_args;

  if (!next_if<none>()) {
    if (!next_is<uint64_t>()) {
      return nullptr;
    }

    auto template_count = next<uint64_t>();

    template_args = TemplateParameters();
    template_args->reserve(template_count);

    while (template_count--) {
      if (!next_if<std::string>("name") || !next_is<std::string>()) {
        return nullptr;
      }

      auto arg_name = next<std::string>();

      if (!next_if<std::string>("type")) {
        return nullptr;
      }

      auto type = deserialize_type();
      if (!type.has_value()) {
        return nullptr;
      }

      if (!next_if<std::string>("default")) {
        return nullptr;
      }

      NullableFlowPtr<Expr> default_value;
      if (next_if<none>()) {
        default_value = nullptr;
      } else {
        default_value = deserialize_expression();
        if (!default_value.has_value()) {
          return nullptr;
        }
      }

      template_args->emplace_back(arg_name, type.value(), default_value);
    }
  }

  if (!next_if<std::string>("names") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto names_count = next<uint64_t>();

  StructDefNames names;
  names.reserve(names_count);

  while (names_count--) {
    if (!next_is<std::string>()) {
      return nullptr;
    }

    auto arg_name = next<std::string>();

    names.push_back(arg_name);
  }

  if (!next_if<std::string>("fields") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto field_count = next<uint64_t>();

  StructDefFields fields;
  fields.reserve(field_count);

  while (field_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto field_name = next<std::string>();

    if (!next_if<std::string>("type")) {
      return nullptr;
    }

    auto type = deserialize_type();
    if (!type.has_value()) {
      return nullptr;
    }

    if (!next_if<std::string>("default")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> default_value;
    if (next_if<none>()) {
      default_value = nullptr;
    } else {
      default_value = deserialize_expression();
      if (!default_value.has_value()) {
        return nullptr;
      }
    }

    if (!next_if<std::string>("visibility")) {
      return nullptr;
    }

    Vis visibility;

    if (next_if<std::string>("pub")) {
      visibility = Vis::Pub;
    } else if (next_if<std::string>("pro")) {
      visibility = Vis::Pro;
    } else if (next_if<std::string>("sec")) {
      visibility = Vis::Sec;
    } else {
      return nullptr;
    }

    if (!next_if<std::string>("static") || !next_is<bool>()) {
      return nullptr;
    }

    auto is_static = next<bool>();

    fields.emplace_back(StructField(visibility, is_static, field_name,
                                    type.value(), default_value));
  }

  if (!next_if<std::string>("methods") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto method_count = next<uint64_t>();

  StructDefMethods methods;
  methods.reserve(method_count);

  while (method_count--) {
    if (!next_if<std::string>("visibility")) {
      return nullptr;
    }

    Vis visibility;

    if (next_if<std::string>("pub")) {
      visibility = Vis::Pub;
    } else if (next_if<std::string>("pro")) {
      visibility = Vis::Pro;
    } else if (next_if<std::string>("sec")) {
      visibility = Vis::Sec;
    } else {
      return nullptr;
    }

    if (!next_if<std::string>("method")) {
      return nullptr;
    }

    auto method = deserialize_statement();
    if (!method.has_value()) {
      return nullptr;
    }

    methods.emplace_back(visibility, method.value());
  }

  if (!next_if<std::string>("static-methods") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto static_method_count = next<uint64_t>();

  StructDefStaticMethods static_methods;
  static_methods.reserve(static_method_count);

  while (static_method_count--) {
    if (!next_if<std::string>("visibility")) {
      return nullptr;
    }

    Vis visibility;

    if (next_if<std::string>("pub")) {
      visibility = Vis::Pub;
    } else if (next_if<std::string>("pro")) {
      visibility = Vis::Pro;
    } else if (next_if<std::string>("sec")) {
      visibility = Vis::Sec;
    } else {
      return nullptr;
    }

    if (!next_if<std::string>("method")) {
      return nullptr;
    }

    auto method = deserialize_statement();
    if (!method.has_value()) {
      return nullptr;
    }

    static_methods.emplace_back(visibility, method.value());
  }

  return make<StructDef>(mode, attributes, name, template_args, names, fields,
                         methods, static_methods)();
}

NullableFlowPtr<EnumDef> AST_Reader::ReadKind_Enum() {
  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  if (!next_if<std::string>("type")) {
    return nullptr;
  }

  NullableFlowPtr<Type> type;
  if (next_if<none>()) {
    type = nullptr;
  } else {
    type = deserialize_type();
    if (!type.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("fields") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto field_count = next<uint64_t>();

  EnumDefItems fields;
  fields.reserve(field_count);

  while (field_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto field_name = next<std::string>();

    if (!next_if<std::string>("value")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> value;
    if (next_if<none>()) {
      value = nullptr;
    } else {
      value = deserialize_expression();
      if (!value.has_value()) {
        return nullptr;
      }
    }

    fields.emplace_back(field_name, value);
  }

  return make<EnumDef>(name, type, std::move(fields))();
}

NullableFlowPtr<Function> AST_Reader::ReadKind_Function() {
  if (!next_if<std::string>("attributes") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count--) {
    auto attribute = deserialize_expression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!next_if<std::string>("thread_safe") || !next_is<bool>()) {
    return nullptr;
  }

  auto thread_safe = next<bool>();

  if (!next_if<std::string>("purity")) {
    return nullptr;
  }

  Purity purity;
  if (next_if<std::string>("impure")) {
    purity = thread_safe ? Purity::Impure_TSafe : Purity::Impure;
  } else if (next_if<std::string>("pure")) {
    purity = Purity::Pure;
  } else if (next_if<std::string>("quasi")) {
    purity = Purity::Quasi;
  } else if (next_if<std::string>("retro")) {
    purity = Purity::Retro;
  } else {
    return nullptr;
  }

  if (!next_if<std::string>("captures") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto capture_count = next<uint64_t>();

  FnCaptures captures;
  captures.reserve(capture_count);

  while (capture_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto name = next<std::string>();

    if (!next_if<std::string>("is_ref") || !next_is<bool>()) {
      return nullptr;
    }

    auto is_ref = next<bool>();

    captures.emplace_back(name, is_ref);
  }

  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  if (!next_if<std::string>("template")) {
    return nullptr;
  }

  std::optional<TemplateParameters> template_args;

  if (!next_if<none>()) {
    auto template_count = next<uint64_t>();

    template_args = TemplateParameters();
    template_args->reserve(template_count);

    while (template_count--) {
      if (!next_if<std::string>("name") || !next_is<std::string>()) {
        return nullptr;
      }

      auto arg_name = next<std::string>();

      if (!next_if<std::string>("type")) {
        return nullptr;
      }

      auto type = deserialize_type();
      if (!type.has_value()) {
        return nullptr;
      }

      if (!next_if<std::string>("default")) {
        return nullptr;
      }

      NullableFlowPtr<Expr> default_value;
      if (next_if<none>()) {
        default_value = nullptr;
      } else {
        default_value = deserialize_expression();
        if (!default_value.has_value()) {
          return nullptr;
        }
      }

      template_args->emplace_back(arg_name, type.value(), default_value);
    }
  }

  if (!next_if<std::string>("input")) {
    return nullptr;
  }

  if (!next_if<std::string>("variadic") || !next_is<bool>()) {
    return nullptr;
  }

  auto variadic = next<bool>();

  if (!next_if<std::string>("parameters") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto parameter_count = next<uint64_t>();

  FuncParams parameters;
  parameters.reserve(parameter_count);

  while (parameter_count--) {
    if (!next_if<std::string>("name") || !next_is<std::string>()) {
      return nullptr;
    }

    auto parameter_name = next<std::string>();

    if (!next_if<std::string>("type")) {
      return nullptr;
    }

    auto type = deserialize_type();
    if (!type.has_value()) {
      return nullptr;
    }

    if (!next_if<std::string>("default")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> default_value;
    if (next_if<none>()) {
      default_value = nullptr;
    } else {
      default_value = deserialize_expression();
      if (!default_value.has_value()) {
        return nullptr;
      }
    }

    parameters.emplace_back(std::move(parameter_name), type.value(),
                            default_value);
  }

  if (!next_if<std::string>("return")) {
    return nullptr;
  }

  auto return_type = deserialize_type();
  if (!return_type.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("precond")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> precond;
  if (next_if<none>()) {
    precond = nullptr;
  } else {
    precond = deserialize_expression();
    if (!precond.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("postcond")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> postcond;
  if (next_if<none>()) {
    postcond = nullptr;
  } else {
    postcond = deserialize_expression();
    if (!postcond.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("body")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> body;
  if (next_if<none>()) {
    body = nullptr;
  } else {
    body = deserialize_statement();
    if (!body.has_value()) {
      return nullptr;
    }
  }

  return make<Function>(std::move(attributes), purity, std::move(captures),
                        name, std::move(template_args), std::move(parameters),
                        variadic, return_type.value(), precond, postcond,
                        body)();
}

NullableFlowPtr<ScopeStmt> AST_Reader::ReadKind_Scope() {
  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  if (!next_if<std::string>("depends") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto dependency_count = next<uint64_t>();

  ScopeDeps dependencies;
  dependencies.reserve(dependency_count);

  while (dependency_count--) {
    auto dependency = next<std::string>();
    dependencies.push_back(dependency);
  }

  if (!next_if<std::string>("body")) {
    return nullptr;
  }

  auto body = deserialize_statement();
  if (!body.has_value()) {
    return nullptr;
  }

  return make<ScopeStmt>(name, body.value(), dependencies)();
}

NullableFlowPtr<ExportStmt> AST_Reader::ReadKind_Export() {
  if (!next_if<std::string>("abi") || !next_is<std::string>()) {
    return nullptr;
  }

  auto abi_name = next<std::string>();

  if (!next_if<std::string>("visibility") || !next_is<std::string>()) {
    return nullptr;
  }

  Vis visibility;

  if (next_if<std::string>("pub")) {
    visibility = Vis::Pub;
  } else if (next_if<std::string>("pro")) {
    visibility = Vis::Pro;
  } else if (next_if<std::string>("sec")) {
    visibility = Vis::Sec;
  } else {
    return nullptr;
  }

  if (!next_if<std::string>("attributes") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count--) {
    auto attribute = deserialize_expression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!next_if<std::string>("body")) {
    return nullptr;
  }

  auto body = deserialize_statement();
  if (!body.has_value()) {
    return nullptr;
  }

  return make<ExportStmt>(body.value(), abi_name, visibility,
                          std::move(attributes))();
}

NullableFlowPtr<Block> AST_Reader::ReadKind_Block() {
  if (!next_if<std::string>("safe")) {
    return nullptr;
  }

  SafetyMode mode;

  if (next_if<none>()) {
    mode = SafetyMode::Unknown;
  } else if (next_if<std::string>("yes")) {
    mode = SafetyMode::Safe;
  } else if (next_if<std::string>("no")) {
    mode = SafetyMode::Unsafe;
  } else {
    return nullptr;
  }

  if (!next_if<std::string>("body") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto statement_count = next<uint64_t>();

  BlockItems statements;
  statements.reserve(statement_count);

  while (statement_count--) {
    auto stmt = deserialize_statement();
    if (!stmt.has_value()) {
      return nullptr;
    }

    statements.push_back(stmt.value());
  }

  return make<Block>(std::move(statements), mode)();
}

NullableFlowPtr<VarDecl> AST_Reader::ReadKind_Let() {
  if (!next_if<std::string>("mode")) {
    return nullptr;
  }
  VarDeclType mode;

  if (next_if<std::string>("let")) {
    mode = VarDeclType::Let;
  } else if (next_if<std::string>("var")) {
    mode = VarDeclType::Var;
  } else if (next_if<std::string>("const")) {
    mode = VarDeclType::Const;
  } else {
    return nullptr;
  }

  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  if (!next_if<std::string>("type")) {
    return nullptr;
  }

  NullableFlowPtr<Type> type;
  if (next_if<none>()) {
    type = nullptr;
  } else {
    type = deserialize_type();
    if (!type.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("value")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> value;
  if (next_if<none>()) {
    value = nullptr;
  } else {
    value = deserialize_expression();
    if (!value.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("attributes") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count--) {
    auto attribute = deserialize_expression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  return make<VarDecl>(name, type, value, mode, std::move(attributes))();
}

NullableFlowPtr<InlineAsm> AST_Reader::ReadKind_InlineAsm() {
  if (!next_if<std::string>("assembly") || !next_is<std::string>()) {
    return nullptr;
  }

  auto assembly = next<std::string>();

  if (!next_if<std::string>("parameters") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto parameter_count = next<uint64_t>();

  ExpressionList parameters;
  parameters.reserve(parameter_count);

  while (parameter_count--) {
    auto parameter = deserialize_expression();
    if (!parameter.has_value()) {
      return nullptr;
    }

    parameters.push_back(parameter.value());
  }

  return make<InlineAsm>(assembly, std::move(parameters))();
}

NullableFlowPtr<ReturnStmt> AST_Reader::ReadKind_Return() {
  if (!next_if<std::string>("expr")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> expression;
  if (next_if<none>()) {
    expression = nullptr;
  } else {
    expression = deserialize_expression();
    if (!expression.has_value()) {
      return nullptr;
    }
  }

  return make<ReturnStmt>(expression)();
}

NullableFlowPtr<ReturnIfStmt> AST_Reader::ReadKind_Retif() {
  if (!next_if<std::string>("cond")) {
    return nullptr;
  }

  auto condition = deserialize_expression();
  if (!condition.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("expr")) {
    return nullptr;
  }

  auto expression = deserialize_expression();
  if (!expression.has_value()) {
    return nullptr;
  }

  return make<ReturnIfStmt>(condition.value(), expression.value())();
}

NullableFlowPtr<BreakStmt> AST_Reader::ReadKind_Break() {
  return make<BreakStmt>()();
}

NullableFlowPtr<ContinueStmt> AST_Reader::ReadKind_Continue() {
  return make<ContinueStmt>()();
}

NullableFlowPtr<IfStmt> AST_Reader::ReadKind_If() {
  if (!next_if<std::string>("cond")) {
    return nullptr;
  }

  auto condition = deserialize_expression();
  if (!condition.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("then")) {
    return nullptr;
  }

  auto then_block = deserialize_statement();

  if (!next_if<std::string>("else")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> else_block;
  if (next_if<none>()) {
    else_block = nullptr;
  } else {
    else_block = deserialize_statement();
    if (!else_block.has_value()) {
      return nullptr;
    }
  }

  return make<IfStmt>(condition.value(), then_block.value(), else_block)();
}

NullableFlowPtr<WhileStmt> AST_Reader::ReadKind_While() {
  if (!next_if<std::string>("cond")) {
    return nullptr;
  }

  auto condition = deserialize_expression();
  if (!condition.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("body")) {
    return nullptr;
  }

  auto body = deserialize_statement();
  if (!body.has_value()) {
    return nullptr;
  }

  return make<WhileStmt>(condition.value(), body.value())();
}

NullableFlowPtr<ForStmt> AST_Reader::ReadKind_For() {
  if (!next_if<std::string>("init")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> init;
  if (next_if<none>()) {
    init = nullptr;
  } else {
    init = deserialize_statement();
    if (!init.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("cond")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> condition;
  if (next_if<none>()) {
    condition = nullptr;
  } else {
    condition = deserialize_expression();
    if (!condition.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("step")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> step;
  if (next_if<none>()) {
    step = nullptr;
  } else {
    step = deserialize_expression();
    if (!step.has_value()) {
      return nullptr;
    }
  }

  if (!next_if<std::string>("body")) {
    return nullptr;
  }

  auto body = deserialize_statement();
  if (!body.has_value()) {
    return nullptr;
  }

  return make<ForStmt>(init, condition, step, body.value())();
}

NullableFlowPtr<ForeachStmt> AST_Reader::ReadKind_Foreach() {
  if (!next_if<std::string>("idx") || !next_is<std::string>()) {
    return nullptr;
  }

  auto index_name = next<std::string>();

  if (!next_if<std::string>("val") || !next_is<std::string>()) {
    return nullptr;
  }

  auto value_name = next<std::string>();

  if (!next_if<std::string>("expr")) {
    return nullptr;
  }

  auto expression = deserialize_expression();
  if (!expression.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("body")) {
    return nullptr;
  }

  auto body = deserialize_statement();
  if (!body.has_value()) {
    return nullptr;
  }

  return make<ForeachStmt>(index_name, value_name, expression.value(),
                           body.value())();
}

NullableFlowPtr<CaseStmt> AST_Reader::ReadKind_Case() {
  if (!next_if<std::string>("match")) {
    return nullptr;
  }

  auto match = deserialize_expression();
  if (!match.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("body")) {
    return nullptr;
  }

  auto body = deserialize_statement();
  if (!body.has_value()) {
    return nullptr;
  }

  return make<CaseStmt>(match.value(), body.value())();
}

NullableFlowPtr<SwitchStmt> AST_Reader::ReadKind_Switch() {
  if (!next_if<std::string>("match")) {
    return nullptr;
  }

  auto match = deserialize_expression();
  if (!match.has_value()) {
    return nullptr;
  }

  if (!next_if<std::string>("cases") || !next_is<uint64_t>()) {
    return nullptr;
  }

  auto case_count = next<uint64_t>();

  SwitchCases cases;
  cases.reserve(case_count);

  while (case_count--) {
    auto case_stmt = deserialize_statement();
    if (!case_stmt.has_value() || !case_stmt.value()->is(QAST_CASE)) {
      return nullptr;
    }

    cases.push_back(case_stmt.value()->as<CaseStmt>());
  }

  if (!next_if<std::string>("default")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> default_case;
  if (next_if<none>()) {
    default_case = nullptr;
  } else {
    default_case = deserialize_statement();
    if (!default_case.has_value()) {
      return nullptr;
    }
  }

  return make<SwitchStmt>(match.value(), std::move(cases), default_case)();
}

NullableFlowPtr<ExprStmt> AST_Reader::ReadKind_ExprStmt() {
  if (!next_if<std::string>("expr")) {
    return nullptr;
  }

  auto expr = deserialize_expression();
  if (!expr.has_value()) {
    return nullptr;
  }

  return make<ExprStmt>(expr.value())();
}
