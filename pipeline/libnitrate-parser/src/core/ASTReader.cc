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

  bool can_save_source_location =
      m_source.has_value() && R.has_value() && range.has_value();

  if (can_save_source_location) {
    auto start = m_source.value().get().InternLocation(range->start);
    auto end = m_source.value().get().InternLocation(range->end);

    R.value()->setLoc(start, end);
  }

  return R;
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Binexpr() {
  if (!next_if<std::string>("op") || !next_is<std::string>()) {
    return nullptr;
  }

  auto op = next<std::string>();

  auto op_it = lex::LexicalOperators.left.find(op);
  if (op_it == lex::LexicalOperators.left.end()) {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Unexpr() {
  if (!next_if<std::string>("op") || !next_is<std::string>()) {
    return nullptr;
  }

  auto op = next<std::string>();

  auto op_it = lex::LexicalOperators.left.find(op);
  if (op_it == lex::LexicalOperators.left.end()) {
    return nullptr;
  }

  auto rhs = deserialize_expression();
  if (!rhs.has_value()) {
    return nullptr;
  }

  return make<UnaryExpr>(op_it->second, rhs.value())();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_PostUnexpr() {
  if (!next_if<std::string>("op") || !next_is<std::string>()) {
    return nullptr;
  }

  auto op = next<std::string>();

  auto op_it = lex::LexicalOperators.left.find(op);
  if (op_it == lex::LexicalOperators.left.end()) {
    return nullptr;
  }

  auto lhs = deserialize_expression();
  if (!lhs.has_value()) {
    return nullptr;
  }

  return make<PostUnaryExpr>(lhs.value(), op_it->second)();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Terexpr() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Int() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Float() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_String() {
  if (!next_if<std::string>("value") || !next_is<std::string>()) {
    return nullptr;
  }

  auto value = next<std::string>();

  return make<ConstString>(std::move(value))();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Char() {
  if (!next_if<std::string>("value") || !next_is<std::string>()) {
    return nullptr;
  }

  auto value = next<std::string>();

  uint8_t c = 0;
  if (!strict_from_chars(value.data(), value.data() + value.size(), c)) {
    return nullptr;
  }

  if (c > 255) {
    return nullptr;
  }

  return make<ConstChar>(c)();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Bool() {
  if (!next_if<std::string>("value") || !next_is<bool>()) {
    return nullptr;
  }

  auto value = next<bool>();

  return make<ConstBool>(value)();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Null() {
  return make<ConstNull>()();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Undef() {
  return make<ConstUndef>()();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Call() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_List() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Assoc() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Index() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Slice() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_Fstring() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Ident() {
  if (!next_if<std::string>("name") || !next_is<std::string>()) {
    return nullptr;
  }

  auto name = next<std::string>();

  return make<Ident>(std::move(name))();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_SeqPoint() {
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

NullableFlowPtr<Base> AST_Reader::ReadKind_StmtExpr() {
  if (!next_if<std::string>("stmt")) {
    return nullptr;
  }

  auto stmt = deserialize_object();
  if (!stmt.has_value()) {
    return nullptr;
  }

  return make<StmtExpr>(stmt.value())();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_TypeExpr() {
  if (!next_if<std::string>("type")) {
    return nullptr;
  }

  auto type = deserialize_type();
  if (!type.has_value()) {
    return nullptr;
  }

  return make<TypeExpr>(type.value())();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_TemplCall() {
  /// TODO: Implement deserialization for kind
  qcore_implement();
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U1() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U1>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U8() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U8>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U16() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U16>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U32() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U32>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U64() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U64>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_U128() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U128>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I8() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I8>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I16() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I16>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I32() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I32>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I64() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I64>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_I128() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I128>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F16() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F16>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F32() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F32>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F64() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F64>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_F128() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F128>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Void() {
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<VoidTy>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Ref() {
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
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
}

NullableFlowPtr<Base> AST_Reader::ReadKind_Ptr() {
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
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
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
  auto info = Read_TypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<InferTy>()();
  node->set_width(info->width);
  node->set_range_begin(info->min);
  node->set_range_end(info->max);

  return node;
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
    auto stmt = deserialize_object();
    if (!stmt.has_value()) {
      return nullptr;
    }

    statements.push_back(stmt.value());
  }

  return make<Block>(std::move(statements), mode)();
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
  return make<ContinueStmt>()();
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
  if (!next_if<std::string>("expr")) {
    return nullptr;
  }

  auto expr = deserialize_expression();
  if (!expr.has_value()) {
    return nullptr;
  }

  return make<ExprStmt>(expr.value())();
}
