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

#include "core/SyntaxTree.pb.h"

using namespace ncc;
using namespace ncc::parse;
using namespace boost::multiprecision;

template <typename Ty,
          typename = std::enable_if_t<std::is_floating_point_v<Ty>>>
static inline auto StrictFromChars(
    const char* first, const char* last, Ty& value,
    std::chars_format fmt = std::chars_format::general) noexcept -> bool {
  auto res = std::from_chars(first, last, value, fmt);

  return res.ec == std::errc() && res.ptr == last;
}

template <typename Ty>
static inline auto StrictFromChars(const char* first, const char* last,
                                   Ty& value, int base = 10) noexcept -> bool {
  auto res = std::from_chars(first, last, value, base);

  return res.ec == std::errc() && res.ptr == last;
}

auto AstReader::Get() -> std::optional<FlowPtr<Base>> {
  if (!m_root) {
    if (auto root = DeserializeObject()) {
      m_root = root.value();
    }
  }

  return m_root;
}

template <typename ValueType>
constexpr auto AstReader::NextIf(const ValueType& v) -> bool {
  if (auto n = PeekValue()) {
    if (std::holds_alternative<ValueType>(n->operator()()) &&
        std::get<ValueType>(n->operator()()) == v) {
      NextValue();
      return true;
    }
  }

  return false;
}

template <typename ValueType>
constexpr auto AstReader::NextIs() -> bool {
  if (auto n = PeekValue()) {
    if (std::holds_alternative<ValueType>(n->operator()())) {
      return true;
    }
  }

  return false;
}

template <typename ValueType>
constexpr auto AstReader::Next() -> ValueType {
  if (auto n = NextValue()) {
    if (std::holds_alternative<ValueType>(n->operator()())) {
      return std::get<ValueType>(n->operator()());
    }
  }

  qcore_panic("Attempted to read value of incorrect type");
}

auto AstReader::ReadLocationRange() -> std::optional<AstReader::LocationRange> {
  const auto parse_location_object = [&]() -> std::optional<lex::Location> {
    if (!NextIf<std::string>("off") || !NextIs<uint64_t>()) {
      return std::nullopt;
    }

    uint32_t off = Next<uint64_t>();

    if (!NextIf<std::string>("row") || !NextIs<uint64_t>()) {
      return std::nullopt;
    }

    uint32_t row = Next<uint64_t>();

    if (!NextIf<std::string>("col") || !NextIs<uint64_t>()) {
      return std::nullopt;
    }

    uint32_t col = Next<uint64_t>();

    if (!NextIf<std::string>("src") || !NextIs<std::string>()) {
      return std::nullopt;
    }

    auto src = Next<std::string>();

    return lex::Location(off, row, col, src);
  };

  if (!NextIf<std::string>("loc")) {
    return std::nullopt;
  }

  LocationRange range;

  if (NextIf<none>()) {
    return range;
  }

  if (!NextIf<std::string>("begin")) {
    return std::nullopt;
  }

  if (auto begin = parse_location_object()) {
    range.m_start = begin.value();
  } else {
    return std::nullopt;
  }

  if (!NextIf<std::string>("end")) {
    return std::nullopt;
  }

  if (auto end = parse_location_object()) {
    range.m_end = end.value();
  } else {
    return std::nullopt;
  }

  if (!NextIf<std::string>("trace")) {
    return std::nullopt;
  }

  if (NextIf<none>()) {
    return range;
  }

  if (!NextIf<std::string>("src") || !NextIs<std::string>()) {
    return std::nullopt;
  }

  Next<std::string>();

  if (!NextIf<std::string>("sub") || !NextIs<std::string>()) {
    return std::nullopt;
  }

  Next<std::string>();

  if (!NextIf<std::string>("row") || !NextIs<uint64_t>()) {
    return std::nullopt;
  }

  Next<uint64_t>();

  if (!NextIf<std::string>("col") || !NextIs<uint64_t>()) {
    return std::nullopt;
  }

  Next<uint64_t>();

  return range;
}

auto AstReader::ReadTypeMetadata() -> std::optional<AstReader::TypeMetadata> {
  AstReader::TypeMetadata info;

  if (!NextIf<std::string>("width")) {
    return std::nullopt;
  }

  if (NextIf<none>()) {
    info.m_width = nullptr;
  } else {
    auto width = DeserializeExpression();
    if (!width.has_value()) {
      return std::nullopt;
    }

    info.m_width = width.value();
  }

  if (!NextIf<std::string>("min")) {
    return std::nullopt;
  }

  if (NextIf<none>()) {
    info.m_min = nullptr;
  } else {
    auto min = DeserializeExpression();
    if (!min.has_value()) {
      return std::nullopt;
    }

    info.m_min = min.value();
  }

  if (!NextIf<std::string>("max")) {
    return std::nullopt;
  }

  if (NextIf<none>()) {
    info.m_max = nullptr;
  } else {
    auto max = DeserializeExpression();
    if (!max.has_value()) {
      return std::nullopt;
    }

    info.m_max = max.value();
  }

  return info;
}

auto AstReader::DeserializeObject() -> NullableFlowPtr<Base> {
  // This code must be the reverse of the map contained in:
  // 'constexpr std::string_view Base::GetKindName(npar_ty_t type)'
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
      {"Sequence", QAST_SEQ},
      {"PostUnexpr", QAST_POST_UNEXPR},
      {"StmtExpr", QAST_SEXPR},
      {"TypeExpr", QAST_TEXPR},
      {"TemplateCall", QAST_TEMPL_CALL},
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
      {"Assembly", QAST_INLINE_ASM},
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

  if (!NextIf<std::string>("kind") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto it = node_kinds_map.find(Next<std::string>());
  if (it == node_kinds_map.end()) {
    return nullptr;
  }

  auto range = ReadLocationRange();

  NullableFlowPtr<Base> r;

  switch (it->second) {
    case QAST_BASE: {
      r = ReadKindNode();
      break;
    }

    case QAST_BINEXPR: {
      r = ReadKindBinexpr();
      break;
    }

    case QAST_UNEXPR: {
      r = ReadKindUnexpr();
      break;
    }

    case QAST_TEREXPR: {
      r = ReadKindTerexpr();
      break;
    }

    case QAST_INT: {
      r = ReadKindInt();
      break;
    }

    case QAST_FLOAT: {
      r = ReadKindFloat();
      break;
    }

    case QAST_STRING: {
      r = ReadKindString();
      break;
    }

    case QAST_CHAR: {
      r = ReadKindChar();
      break;
    }

    case QAST_BOOL: {
      r = ReadKindBool();
      break;
    }

    case QAST_NULL: {
      r = ReadKindNull();
      break;
    }

    case QAST_UNDEF: {
      r = ReadKindUndef();
      break;
    }

    case QAST_CALL: {
      r = ReadKindCall();
      break;
    }

    case QAST_LIST: {
      r = ReadKindList();
      break;
    }

    case QAST_ASSOC: {
      r = ReadKindAssoc();
      break;
    }

    case QAST_INDEX: {
      r = ReadKindIndex();
      break;
    }

    case QAST_SLICE: {
      r = ReadKindSlice();
      break;
    }

    case QAST_FSTRING: {
      r = ReadKindFstring();
      break;
    }

    case QAST_IDENT: {
      r = ReadKindIdentifier();
      break;
    }

    case QAST_SEQ: {
      r = ReadKindSequence();
      break;
    }

    case QAST_POST_UNEXPR: {
      r = ReadKindPostUnexpr();
      break;
    }

    case QAST_SEXPR: {
      r = ReadKindStmtExpr();
      break;
    }

    case QAST_TEXPR: {
      r = ReadKindTypeExpr();
      break;
    }

    case QAST_TEMPL_CALL: {
      r = ReadKindTemplateCall();
      break;
    }

    case QAST_REF: {
      r = ReadKindRef();
      break;
    }

    case QAST_U1: {
      r = ReadKindU1();
      break;
    }

    case QAST_U8: {
      r = ReadKindU8();
      break;
    }

    case QAST_U16: {
      r = ReadKindU16();
      break;
    }

    case QAST_U32: {
      r = ReadKindU32();
      break;
    }

    case QAST_U64: {
      r = ReadKindU64();
      break;
    }

    case QAST_U128: {
      r = ReadKindU128();
      break;
    }

    case QAST_I8: {
      r = ReadKindI8();
      break;
    }

    case QAST_I16: {
      r = ReadKindI16();
      break;
    }

    case QAST_I32: {
      r = ReadKindI32();
      break;
    }

    case QAST_I64: {
      r = ReadKindI64();
      break;
    }

    case QAST_I128: {
      r = ReadKindI128();
      break;
    }

    case QAST_F16: {
      r = ReadKindF16();
      break;
    }

    case QAST_F32: {
      r = ReadKindF32();
      break;
    }

    case QAST_F64: {
      r = ReadKindF64();
      break;
    }

    case QAST_F128: {
      r = ReadKindF128();
      break;
    }

    case QAST_VOID: {
      r = ReadKindVoid();
      break;
    }

    case QAST_PTR: {
      r = ReadKindPtr();
      break;
    }

    case QAST_OPAQUE: {
      r = ReadKindOpaque();
      break;
    }

    case QAST_ARRAY: {
      r = ReadKindArray();
      break;
    }

    case QAST_TUPLE: {
      r = ReadKindTuple();
      break;
    }

    case QAST_FUNCTOR: {
      r = ReadKindFuncTy();
      break;
    }

    case QAST_NAMED: {
      r = ReadKindUnres();
      break;
    }

    case QAST_INFER: {
      r = ReadKindInfer();
      break;
    }

    case QAST_TEMPLATE: {
      r = ReadKindTempl();
      break;
    }

    case QAST_TYPEDEF: {
      r = ReadKindTypedef();
      break;
    }

    case QAST_STRUCT: {
      r = ReadKindStruct();
      break;
    }

    case QAST_ENUM: {
      r = ReadKindEnum();
      break;
    }

    case QAST_FUNCTION: {
      r = ReadKindFunction();
      break;
    }

    case QAST_SCOPE: {
      r = ReadKindScope();
      break;
    }

    case QAST_EXPORT: {
      r = ReadKindExport();
      break;
    }

    case QAST_BLOCK: {
      r = ReadKindBlock();
      break;
    }

    case QAST_VAR: {
      r = ReadKindLet();
      break;
    }

    case QAST_INLINE_ASM: {
      r = ReadKindAssembly();
      break;
    }

    case QAST_RETURN: {
      r = ReadKindReturn();
      break;
    }

    case QAST_RETIF: {
      r = ReadKindRetif();
      break;
    }

    case QAST_BREAK: {
      r = ReadKindBreak();
      break;
    }

    case QAST_CONTINUE: {
      r = ReadKindContinue();
      break;
    }

    case QAST_IF: {
      r = ReadKindIf();
      break;
    }

    case QAST_WHILE: {
      r = ReadKindWhile();
      break;
    }

    case QAST_FOR: {
      r = ReadKindFor();
      break;
    }

    case QAST_FOREACH: {
      r = ReadKindForeach();
      break;
    }

    case QAST_CASE: {
      r = ReadKindCase();
      break;
    }

    case QAST_SWITCH: {
      r = ReadKindSwitch();
      break;
    }

    case QAST_ESTMT: {
      r = ReadKindExprStmt();
      break;
    }
  }

  if (!r.has_value()) {
    std::cout << "Failed to deserialize object of type: " << it->first
              << std::endl;
  }

  bool can_save_source_location =
      m_source.has_value() && r.has_value() && range.has_value();

  if (can_save_source_location) {
    auto start = m_source.value().get().InternLocation(range->m_start);
    auto end = m_source.value().get().InternLocation(range->m_end);

    r.value()->SetLoc(start, end);
  }

  return r;
}

auto AstReader::DeserializeStatement() -> NullableFlowPtr<Stmt> {
  auto object = DeserializeObject();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->GetKind();
  if (kind < QAST__STMT_FIRST || kind > QAST__STMT_LAST) {
    return nullptr;
  }

  return object.value().As<Stmt>();
}

auto AstReader::DeserializeExpression() -> NullableFlowPtr<Expr> {
  auto object = DeserializeObject();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->GetKind();
  if (kind < QAST__EXPR_FIRST || kind > QAST__EXPR_LAST) {
    return nullptr;
  }

  return object.value().As<Expr>();
}

auto AstReader::DeserializeType() -> NullableFlowPtr<Type> {
  auto object = DeserializeObject();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->GetKind();
  if (kind < QAST__TYPE_FIRST || kind > QAST__TYPE_LAST) {
    return nullptr;
  }

  return object.value().As<Type>();
}

NullableFlowPtr<Base> AstReader::ReadKindNode() {  // NOLINT
  return CreateNode<Base>(QAST_BASE)();
}

auto AstReader::ReadKindBinexpr() -> NullableFlowPtr<Binary> {
  if (!NextIf<std::string>("op") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto op = Next<std::string>();

  auto op_it = lex::LEXICAL_OPERATORS.left.find(op);
  if (op_it == lex::LEXICAL_OPERATORS.left.end()) {
    return nullptr;
  }

  if (!NextIf<std::string>("lhs")) {
    return nullptr;
  }

  auto lhs = DeserializeExpression();
  if (!lhs.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("rhs")) {
    return nullptr;
  }

  auto rhs = DeserializeExpression();
  if (!rhs.has_value()) {
    return nullptr;
  }

  return CreateNode<Binary>(lhs.value(), op_it->second, rhs.value())();
}

auto AstReader::ReadKindUnexpr() -> NullableFlowPtr<Unary> {
  if (!NextIf<std::string>("op") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto op = Next<std::string>();

  auto op_it = lex::LEXICAL_OPERATORS.left.find(op);
  if (op_it == lex::LEXICAL_OPERATORS.left.end()) {
    return nullptr;
  }

  if (!NextIf<std::string>("rhs")) {
    return nullptr;
  }

  auto rhs = DeserializeExpression();
  if (!rhs.has_value()) {
    return nullptr;
  }

  return CreateNode<Unary>(op_it->second, rhs.value())();
}

auto AstReader::ReadKindPostUnexpr() -> NullableFlowPtr<PostUnary> {
  if (!NextIf<std::string>("op") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto op = Next<std::string>();

  auto op_it = lex::LEXICAL_OPERATORS.left.find(op);
  if (op_it == lex::LEXICAL_OPERATORS.left.end()) {
    return nullptr;
  }

  if (!NextIf<std::string>("lhs")) {
    return nullptr;
  }

  auto lhs = DeserializeExpression();
  if (!lhs.has_value()) {
    return nullptr;
  }

  return CreateNode<PostUnary>(lhs.value(), op_it->second)();
}

auto AstReader::ReadKindTerexpr() -> NullableFlowPtr<Ternary> {
  if (!NextIf<std::string>("cond")) {
    return nullptr;
  }

  auto cond = DeserializeExpression();
  if (!cond.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("lhs")) {
    return nullptr;
  }

  auto lhs = DeserializeExpression();
  if (!lhs.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("rhs")) {
    return nullptr;
  }

  auto rhs = DeserializeExpression();
  if (!rhs.has_value()) {
    return nullptr;
  }

  return CreateNode<Ternary>(cond.value(), lhs.value(), rhs.value())();
}

auto AstReader::ReadKindInt() -> NullableFlowPtr<Integer> {
  if (!NextIf<std::string>("value") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto value = Next<std::string>();

  /* Ensure boost won't call std::terminate */
  bool all_digits = std::all_of(value.begin(), value.end(), ::isdigit);
  if (!all_digits) {
    return nullptr;
  }

  /* Ensure the value is within the bounds of a unsigned 128-bit integer */
  if (cpp_int(value) > cpp_int("340282366920938463463374607431768211455")) {
    return nullptr;
  }

  return CreateNode<Integer>(std::move(value))();
}

auto AstReader::ReadKindFloat() -> NullableFlowPtr<Float> {
  if (!NextIf<std::string>("value") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto value = Next<std::string>();

  long double d = 0;
  if (!StrictFromChars(value.data(), value.data() + value.size(), d,
                       std::chars_format::fixed)) {
    return nullptr;
  }

  return CreateNode<Float>(std::move(value))();
}

auto AstReader::ReadKindString() -> NullableFlowPtr<String> {
  if (!NextIf<std::string>("value") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto value = Next<std::string>();

  return CreateNode<String>(std::move(value))();
}

auto AstReader::ReadKindChar() -> NullableFlowPtr<Character> {
  if (!NextIf<std::string>("value") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto value = Next<uint64_t>();

  if (value > 255) {
    return nullptr;
  }

  return CreateNode<Character>(value)();
}

auto AstReader::ReadKindBool() -> NullableFlowPtr<Boolean> {
  if (!NextIf<std::string>("value") || !NextIs<bool>()) {
    return nullptr;
  }

  auto value = Next<bool>();

  return CreateNode<Boolean>(value)();
}

NullableFlowPtr<Null> AstReader::ReadKindNull() {  // NOLINT
  return CreateNode<Null>()();
}

NullableFlowPtr<Undefined> AstReader::ReadKindUndef() {  // NOLINT
  return CreateNode<Undefined>()();
}

auto AstReader::ReadKindCall() -> NullableFlowPtr<Call> {
  if (!NextIf<std::string>("callee")) {
    return nullptr;
  }

  auto callee = DeserializeExpression();
  if (!callee.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("arguments") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto argument_count = Next<uint64_t>();

  CallArgs arguments;
  arguments.reserve(argument_count);

  while (argument_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto name = Next<std::string>();

    if (!NextIf<std::string>("value")) {
      return nullptr;
    }

    auto value = DeserializeExpression();
    if (!value.has_value()) {
      return nullptr;
    }

    arguments.emplace_back(std::move(name), value.value());
  }

  return CreateNode<Call>(callee.value(), std::move(arguments))();
}

auto AstReader::ReadKindList() -> NullableFlowPtr<List> {
  if (!NextIf<std::string>("elements") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto element_count = Next<uint64_t>();

  ExpressionList elements;
  elements.reserve(element_count);

  while (element_count-- > 0) {
    auto element = DeserializeExpression();
    if (!element.has_value()) {
      return nullptr;
    }

    elements.push_back(element.value());
  }

  return CreateNode<List>(std::move(elements))();
}

auto AstReader::ReadKindAssoc() -> NullableFlowPtr<Assoc> {
  if (!NextIf<std::string>("key")) {
    return nullptr;
  }

  auto key = DeserializeExpression();
  if (!key.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("value")) {
    return nullptr;
  }

  auto value = DeserializeExpression();
  if (!value.has_value()) {
    return nullptr;
  }

  return CreateNode<Assoc>(key.value(), value.value())();
}

auto AstReader::ReadKindIndex() -> NullableFlowPtr<Index> {
  if (!NextIf<std::string>("base")) {
    return nullptr;
  }

  auto base = DeserializeExpression();
  if (!base.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("index")) {
    return nullptr;
  }

  auto index = DeserializeExpression();

  return CreateNode<Index>(base.value(), index.value())();
}

auto AstReader::ReadKindSlice() -> NullableFlowPtr<Slice> {
  if (!NextIf<std::string>("base")) {
    return nullptr;
  }

  auto base = DeserializeExpression();
  if (!base.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("start")) {
    return nullptr;
  }

  auto start = DeserializeExpression();
  if (!start.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("end")) {
    return nullptr;
  }

  auto end = DeserializeExpression();
  if (!end.has_value()) {
    return nullptr;
  }

  return CreateNode<Slice>(base.value(), start.value(), end.value())();
}

auto AstReader::ReadKindFstring() -> NullableFlowPtr<FString> {
  if (!NextIf<std::string>("terms") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto term_count = Next<uint64_t>();
  FStringItems terms;
  terms.reserve(term_count);

  while (term_count-- > 0) {
    if (NextIf<std::string>("value") && NextIs<std::string>()) {
      auto value = Next<std::string>();
      terms.emplace_back(value);
    } else {
      auto term = DeserializeExpression();
      if (!term.has_value()) {
        return nullptr;
      }

      terms.emplace_back(term.value());
    }
  }

  return CreateNode<FString>(std::move(terms))();
}

auto AstReader::ReadKindIdentifier() -> NullableFlowPtr<Identifier> {
  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  return CreateNode<Identifier>(std::move(name))();
}

auto AstReader::ReadKindSequence() -> NullableFlowPtr<Sequence> {
  if (!NextIf<std::string>("terms") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto expression_count = Next<uint64_t>();

  ExpressionList terms;
  terms.reserve(expression_count);

  while (expression_count-- > 0) {
    auto term = DeserializeExpression();
    if (!term.has_value()) {
      return nullptr;
    }

    terms.push_back(term.value());
  }

  return CreateNode<Sequence>(std::move(terms))();
}

auto AstReader::ReadKindStmtExpr() -> NullableFlowPtr<StmtExpr> {
  if (!NextIf<std::string>("stmt")) {
    return nullptr;
  }

  auto stmt = DeserializeStatement();
  if (!stmt.has_value()) {
    return nullptr;
  }

  return CreateNode<StmtExpr>(stmt.value())();
}

auto AstReader::ReadKindTypeExpr() -> NullableFlowPtr<TypeExpr> {
  if (!NextIf<std::string>("type")) {
    return nullptr;
  }

  auto type = DeserializeType();
  if (!type.has_value()) {
    return nullptr;
  }

  return CreateNode<TypeExpr>(type.value())();
}

auto AstReader::ReadKindTemplateCall() -> NullableFlowPtr<TemplateCall> {
  if (!NextIf<std::string>("callee")) {
    return nullptr;
  }

  auto callee = DeserializeExpression();
  if (!callee.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("template") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto template_count = Next<uint64_t>();

  CallArgs template_args;
  template_args.reserve(template_count);

  while (template_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto name = Next<std::string>();

    if (!NextIf<std::string>("value")) {
      return nullptr;
    }

    auto value = DeserializeExpression();
    if (!value.has_value()) {
      return nullptr;
    }

    template_args.emplace_back(std::move(name), value.value());
  }

  if (!NextIf<std::string>("arguments") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto argument_count = Next<uint64_t>();

  CallArgs arguments;
  arguments.reserve(argument_count);

  while (argument_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto name = Next<std::string>();

    if (!NextIf<std::string>("value")) {
      return nullptr;
    }

    auto value = DeserializeExpression();
    if (!value.has_value()) {
      return nullptr;
    }

    arguments.emplace_back(std::move(name), value.value());
  }

  return CreateNode<TemplateCall>(callee.value(), std::move(arguments),
                                  std::move(template_args))();
}

auto AstReader::ReadKindU1() -> NullableFlowPtr<U1> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<U1>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindU8() -> NullableFlowPtr<U8> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<U8>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindU16() -> NullableFlowPtr<U16> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<U16>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindU32() -> NullableFlowPtr<U32> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<U32>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindU64() -> NullableFlowPtr<U64> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<U64>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindU128() -> NullableFlowPtr<U128> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<U128>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindI8() -> NullableFlowPtr<I8> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<I8>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindI16() -> NullableFlowPtr<I16> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<I16>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindI32() -> NullableFlowPtr<I32> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<I32>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindI64() -> NullableFlowPtr<I64> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<I64>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindI128() -> NullableFlowPtr<I128> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<I128>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindF16() -> NullableFlowPtr<F16> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<F16>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindF32() -> NullableFlowPtr<F32> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<F32>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindF64() -> NullableFlowPtr<F64> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<F64>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindF128() -> NullableFlowPtr<F128> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<F128>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindVoid() -> NullableFlowPtr<VoidTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<VoidTy>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindRef() -> NullableFlowPtr<RefTy> {
  auto info = ReadTypeMetadata();

  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("to")) {
    return nullptr;
  }

  auto to = DeserializeType();
  if (!to.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<RefTy>(to.value())();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindPtr() -> NullableFlowPtr<PtrTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("volatile") || !NextIs<bool>()) {
    return nullptr;
  }

  bool is_volatile = Next<bool>();

  if (!NextIf<std::string>("to")) {
    return nullptr;
  }

  auto to = DeserializeType();
  if (!to.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<PtrTy>(to.value(), is_volatile)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindOpaque() -> NullableFlowPtr<OpaqueTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  auto node = CreateNode<OpaqueTy>(name)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindArray() -> NullableFlowPtr<ArrayTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("of")) {
    return nullptr;
  }

  auto of = DeserializeType();
  if (!of.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("size")) {
    return nullptr;
  }

  auto size = DeserializeExpression();
  if (!size.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<ArrayTy>(of.value(), size.value())();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindTuple() -> NullableFlowPtr<TupleTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("fields") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto field_count = Next<uint64_t>();

  TupleTyItems fields;
  fields.reserve(field_count);

  while (field_count-- > 0) {
    auto field = DeserializeType();
    if (!field.has_value()) {
      return nullptr;
    }

    fields.push_back(field.value());
  }

  auto node = CreateNode<TupleTy>(std::move(fields))();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindFuncTy() -> NullableFlowPtr<FuncTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("attributes") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = Next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count-- > 0) {
    auto attribute = DeserializeExpression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!NextIf<std::string>("return")) {
    return nullptr;
  }

  auto return_type = DeserializeType();
  if (!return_type.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("thread_safe") || !NextIs<bool>()) {
    return nullptr;
  }

  auto thread_safe = Next<bool>();

  if (!NextIf<std::string>("purity")) {
    return nullptr;
  }

  Purity purity;
  if (NextIf<std::string>("impure")) {
    purity = thread_safe ? Purity::Impure_TSafe : Purity::Impure;
  } else if (NextIf<std::string>("pure")) {
    purity = Purity::Pure;
  } else if (NextIf<std::string>("quasi")) {
    purity = Purity::Quasi;
  } else if (NextIf<std::string>("retro")) {
    purity = Purity::Retro;
  } else {
    return nullptr;
  }

  if (!NextIf<std::string>("input")) {
    return nullptr;
  }

  if (!NextIf<std::string>("variadic") || !NextIs<bool>()) {
    return nullptr;
  }

  auto variadic = Next<bool>();

  if (!NextIf<std::string>("parameters") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto parameter_count = Next<uint64_t>();

  FuncParams parameters;
  parameters.reserve(parameter_count);

  while (parameter_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto name = Next<std::string>();

    if (!NextIf<std::string>("type")) {
      return nullptr;
    }

    auto type = DeserializeType();
    if (!type.has_value()) {
      return nullptr;
    }

    if (!NextIf<std::string>("default")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> default_value;
    if (NextIf<none>()) {
      default_value = nullptr;
    } else {
      default_value = DeserializeExpression();
      if (!default_value.has_value()) {
        return nullptr;
      }
    }

    parameters.emplace_back(std::move(name), type.value(), default_value);
  }

  auto node = CreateNode<FuncTy>(return_type.value(), parameters, variadic,
                                 purity, attributes)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindUnres() -> NullableFlowPtr<NamedTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  auto node = CreateNode<NamedTy>(name)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindInfer() -> NullableFlowPtr<InferTy> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = CreateNode<InferTy>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindTempl() -> NullableFlowPtr<TemplateType> {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("template")) {
    return nullptr;
  }

  auto templ = DeserializeType();
  if (!templ.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("arguments") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto argument_count = Next<uint64_t>();

  CallArgs arguments;
  arguments.reserve(argument_count);

  while (argument_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto name = Next<std::string>();

    if (!NextIf<std::string>("value")) {
      return nullptr;
    }

    auto value = DeserializeExpression();
    if (!value.has_value()) {
      return nullptr;
    }

    arguments.emplace_back(std::move(name), value.value());
  }

  auto node = CreateNode<TemplateType>(templ.value(), std::move(arguments))();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

auto AstReader::ReadKindTypedef() -> NullableFlowPtr<Typedef> {
  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  if (!NextIf<std::string>("type")) {
    return nullptr;
  }

  auto type = DeserializeType();
  if (!type.has_value()) {
    return nullptr;
  }

  return CreateNode<Typedef>(name, type.value())();
}

auto AstReader::ReadKindStruct() -> NullableFlowPtr<Struct> {
  if (!NextIf<std::string>("mode")) {
    return nullptr;
  }

  CompositeType mode;

  if (NextIf<std::string>("region")) {
    mode = CompositeType::Region;
  } else if (NextIf<std::string>("struct")) {
    mode = CompositeType::Struct;
  } else if (NextIf<std::string>("group")) {
    mode = CompositeType::Group;
  } else if (NextIf<std::string>("class")) {
    mode = CompositeType::Class;
  } else if (NextIf<std::string>("union")) {
    mode = CompositeType::Union;
  } else {
    return nullptr;
  }

  if (!NextIf<std::string>("attributes") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = Next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count-- > 0) {
    auto attribute = DeserializeExpression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  if (!NextIf<std::string>("template")) {
    return nullptr;
  }

  std::optional<TemplateParameters> template_args;

  if (!NextIf<none>()) {
    if (!NextIs<uint64_t>()) {
      return nullptr;
    }

    auto template_count = Next<uint64_t>();

    template_args = TemplateParameters();
    template_args->reserve(template_count);

    while (template_count-- > 0) {
      if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
        return nullptr;
      }

      auto arg_name = Next<std::string>();

      if (!NextIf<std::string>("type")) {
        return nullptr;
      }

      auto type = DeserializeType();
      if (!type.has_value()) {
        return nullptr;
      }

      if (!NextIf<std::string>("default")) {
        return nullptr;
      }

      NullableFlowPtr<Expr> default_value;
      if (NextIf<none>()) {
        default_value = nullptr;
      } else {
        default_value = DeserializeExpression();
        if (!default_value.has_value()) {
          return nullptr;
        }
      }

      template_args->emplace_back(arg_name, type.value(), default_value);
    }
  }

  if (!NextIf<std::string>("names") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto names_count = Next<uint64_t>();

  StructNames names;
  names.reserve(names_count);

  while (names_count-- > 0) {
    if (!NextIs<std::string>()) {
      return nullptr;
    }

    auto arg_name = Next<std::string>();

    names.emplace_back(arg_name);
  }

  if (!NextIf<std::string>("fields") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto field_count = Next<uint64_t>();

  StructFields fields;
  fields.reserve(field_count);

  while (field_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto field_name = Next<std::string>();

    if (!NextIf<std::string>("type")) {
      return nullptr;
    }

    auto type = DeserializeType();
    if (!type.has_value()) {
      return nullptr;
    }

    if (!NextIf<std::string>("default")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> default_value;
    if (NextIf<none>()) {
      default_value = nullptr;
    } else {
      default_value = DeserializeExpression();
      if (!default_value.has_value()) {
        return nullptr;
      }
    }

    if (!NextIf<std::string>("visibility")) {
      return nullptr;
    }

    Vis visibility;

    if (NextIf<std::string>("pub")) {
      visibility = Vis::Pub;
    } else if (NextIf<std::string>("pro")) {
      visibility = Vis::Pro;
    } else if (NextIf<std::string>("sec")) {
      visibility = Vis::Sec;
    } else {
      return nullptr;
    }

    if (!NextIf<std::string>("static") || !NextIs<bool>()) {
      return nullptr;
    }

    auto is_static = Next<bool>();

    fields.emplace_back(visibility, is_static, field_name, type.value(),
                        default_value);
  }

  if (!NextIf<std::string>("methods") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto method_count = Next<uint64_t>();

  StructMethods methods;
  methods.reserve(method_count);

  while (method_count-- > 0) {
    if (!NextIf<std::string>("visibility")) {
      return nullptr;
    }

    Vis visibility;

    if (NextIf<std::string>("pub")) {
      visibility = Vis::Pub;
    } else if (NextIf<std::string>("pro")) {
      visibility = Vis::Pro;
    } else if (NextIf<std::string>("sec")) {
      visibility = Vis::Sec;
    } else {
      return nullptr;
    }

    if (!NextIf<std::string>("method")) {
      return nullptr;
    }

    auto method = DeserializeStatement();
    if (!method.has_value()) {
      return nullptr;
    }

    methods.emplace_back(visibility, method.value());
  }

  if (!NextIf<std::string>("static-methods") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto static_method_count = Next<uint64_t>();

  StructStaticMethods static_methods;
  static_methods.reserve(static_method_count);

  while (static_method_count-- > 0) {
    if (!NextIf<std::string>("visibility")) {
      return nullptr;
    }

    Vis visibility;

    if (NextIf<std::string>("pub")) {
      visibility = Vis::Pub;
    } else if (NextIf<std::string>("pro")) {
      visibility = Vis::Pro;
    } else if (NextIf<std::string>("sec")) {
      visibility = Vis::Sec;
    } else {
      return nullptr;
    }

    if (!NextIf<std::string>("method")) {
      return nullptr;
    }

    auto method = DeserializeStatement();
    if (!method.has_value()) {
      return nullptr;
    }

    static_methods.emplace_back(visibility, method.value());
  }

  return CreateNode<Struct>(mode, attributes, name, template_args, names,
                            fields, methods, static_methods)();
}

auto AstReader::ReadKindEnum() -> NullableFlowPtr<Enum> {
  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  if (!NextIf<std::string>("type")) {
    return nullptr;
  }

  NullableFlowPtr<Type> type;
  if (NextIf<none>()) {
    type = nullptr;
  } else {
    type = DeserializeType();
    if (!type.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("fields") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto field_count = Next<uint64_t>();

  EnumItems fields;
  fields.reserve(field_count);

  while (field_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto field_name = Next<std::string>();

    if (!NextIf<std::string>("value")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> value;
    if (NextIf<none>()) {
      value = nullptr;
    } else {
      value = DeserializeExpression();
      if (!value.has_value()) {
        return nullptr;
      }
    }

    fields.emplace_back(field_name, value);
  }

  return CreateNode<Enum>(name, type, std::move(fields))();
}

auto AstReader::ReadKindFunction() -> NullableFlowPtr<Function> {
  if (!NextIf<std::string>("attributes") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = Next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count-- > 0) {
    auto attribute = DeserializeExpression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!NextIf<std::string>("thread_safe") || !NextIs<bool>()) {
    return nullptr;
  }

  auto thread_safe = Next<bool>();

  if (!NextIf<std::string>("purity")) {
    return nullptr;
  }

  Purity purity;
  if (NextIf<std::string>("impure")) {
    purity = thread_safe ? Purity::Impure_TSafe : Purity::Impure;
  } else if (NextIf<std::string>("pure")) {
    purity = Purity::Pure;
  } else if (NextIf<std::string>("quasi")) {
    purity = Purity::Quasi;
  } else if (NextIf<std::string>("retro")) {
    purity = Purity::Retro;
  } else {
    return nullptr;
  }

  if (!NextIf<std::string>("captures") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto capture_count = Next<uint64_t>();

  FnCaptures captures;
  captures.reserve(capture_count);

  while (capture_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto name = Next<std::string>();

    if (!NextIf<std::string>("is_ref") || !NextIs<bool>()) {
      return nullptr;
    }

    auto is_ref = Next<bool>();

    captures.emplace_back(name, is_ref);
  }

  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  if (!NextIf<std::string>("template")) {
    return nullptr;
  }

  std::optional<TemplateParameters> template_args;

  if (!NextIf<none>()) {
    auto template_count = Next<uint64_t>();

    template_args = TemplateParameters();
    template_args->reserve(template_count);

    while (template_count-- > 0) {
      if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
        return nullptr;
      }

      auto arg_name = Next<std::string>();

      if (!NextIf<std::string>("type")) {
        return nullptr;
      }

      auto type = DeserializeType();
      if (!type.has_value()) {
        return nullptr;
      }

      if (!NextIf<std::string>("default")) {
        return nullptr;
      }

      NullableFlowPtr<Expr> default_value;
      if (NextIf<none>()) {
        default_value = nullptr;
      } else {
        default_value = DeserializeExpression();
        if (!default_value.has_value()) {
          return nullptr;
        }
      }

      template_args->emplace_back(arg_name, type.value(), default_value);
    }
  }

  if (!NextIf<std::string>("input")) {
    return nullptr;
  }

  if (!NextIf<std::string>("variadic") || !NextIs<bool>()) {
    return nullptr;
  }

  auto variadic = Next<bool>();

  if (!NextIf<std::string>("parameters") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto parameter_count = Next<uint64_t>();

  FuncParams parameters;
  parameters.reserve(parameter_count);

  while (parameter_count-- > 0) {
    if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
      return nullptr;
    }

    auto parameter_name = Next<std::string>();

    if (!NextIf<std::string>("type")) {
      return nullptr;
    }

    auto type = DeserializeType();
    if (!type.has_value()) {
      return nullptr;
    }

    if (!NextIf<std::string>("default")) {
      return nullptr;
    }

    NullableFlowPtr<Expr> default_value;
    if (NextIf<none>()) {
      default_value = nullptr;
    } else {
      default_value = DeserializeExpression();
      if (!default_value.has_value()) {
        return nullptr;
      }
    }

    parameters.emplace_back(std::move(parameter_name), type.value(),
                            default_value);
  }

  if (!NextIf<std::string>("return")) {
    return nullptr;
  }

  auto return_type = DeserializeType();
  if (!return_type.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("precond")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> precond;
  if (NextIf<none>()) {
    precond = nullptr;
  } else {
    precond = DeserializeExpression();
    if (!precond.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("postcond")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> postcond;
  if (NextIf<none>()) {
    postcond = nullptr;
  } else {
    postcond = DeserializeExpression();
    if (!postcond.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> body;
  if (NextIf<none>()) {
    body = nullptr;
  } else {
    body = DeserializeStatement();
    if (!body.has_value()) {
      return nullptr;
    }
  }

  return CreateNode<Function>(
      std::move(attributes), purity, std::move(captures), name,
      std::move(template_args), std::move(parameters), variadic,
      return_type.value(), precond, postcond, body)();
}

auto AstReader::ReadKindScope() -> NullableFlowPtr<Scope> {
  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  if (!NextIf<std::string>("depends") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto dependency_count = Next<uint64_t>();

  ScopeDeps dependencies;
  dependencies.reserve(dependency_count);

  while (dependency_count-- > 0) {
    auto dependency = Next<std::string>();
    dependencies.emplace_back(dependency);
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  auto body = DeserializeStatement();
  if (!body.has_value()) {
    return nullptr;
  }

  return CreateNode<Scope>(name, body.value(), dependencies)();
}

auto AstReader::ReadKindExport() -> NullableFlowPtr<Export> {
  if (!NextIf<std::string>("abi") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto abi_name = Next<std::string>();

  if (!NextIf<std::string>("visibility") || !NextIs<std::string>()) {
    return nullptr;
  }

  Vis visibility;

  if (NextIf<std::string>("pub")) {
    visibility = Vis::Pub;
  } else if (NextIf<std::string>("pro")) {
    visibility = Vis::Pro;
  } else if (NextIf<std::string>("sec")) {
    visibility = Vis::Sec;
  } else {
    return nullptr;
  }

  if (!NextIf<std::string>("attributes") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = Next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count-- > 0) {
    auto attribute = DeserializeExpression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  auto body = DeserializeStatement();
  if (!body.has_value()) {
    return nullptr;
  }

  return CreateNode<Export>(body.value(), abi_name, visibility,
                            std::move(attributes))();
}

auto AstReader::ReadKindBlock() -> NullableFlowPtr<Block> {
  if (!NextIf<std::string>("safe")) {
    return nullptr;
  }

  SafetyMode mode;

  if (NextIf<none>()) {
    mode = SafetyMode::Unknown;
  } else if (NextIf<std::string>("yes")) {
    mode = SafetyMode::Safe;
  } else if (NextIf<std::string>("no")) {
    mode = SafetyMode::Unsafe;
  } else {
    return nullptr;
  }

  if (!NextIf<std::string>("body") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto statement_count = Next<uint64_t>();

  BlockItems statements;
  statements.reserve(statement_count);

  while (statement_count-- > 0) {
    auto stmt = DeserializeStatement();
    if (!stmt.has_value()) {
      return nullptr;
    }

    statements.push_back(stmt.value());
  }

  return CreateNode<Block>(std::move(statements), mode)();
}

auto AstReader::ReadKindLet() -> NullableFlowPtr<Variable> {
  if (!NextIf<std::string>("mode")) {
    return nullptr;
  }
  VariableType mode;

  if (NextIf<std::string>("let")) {
    mode = VariableType::Let;
  } else if (NextIf<std::string>("var")) {
    mode = VariableType::Var;
  } else if (NextIf<std::string>("const")) {
    mode = VariableType::Const;
  } else {
    return nullptr;
  }

  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  if (!NextIf<std::string>("type")) {
    return nullptr;
  }

  NullableFlowPtr<Type> type;
  if (NextIf<none>()) {
    type = nullptr;
  } else {
    type = DeserializeType();
    if (!type.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("value")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> value;
  if (NextIf<none>()) {
    value = nullptr;
  } else {
    value = DeserializeExpression();
    if (!value.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("attributes") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = Next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count-- > 0) {
    auto attribute = DeserializeExpression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  return CreateNode<Variable>(name, type, value, mode, std::move(attributes))();
}

auto AstReader::ReadKindAssembly() -> NullableFlowPtr<Assembly> {
  if (!NextIf<std::string>("assembly") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto assembly = Next<std::string>();

  if (!NextIf<std::string>("parameters") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto parameter_count = Next<uint64_t>();

  ExpressionList parameters;
  parameters.reserve(parameter_count);

  while (parameter_count-- > 0) {
    auto parameter = DeserializeExpression();
    if (!parameter.has_value()) {
      return nullptr;
    }

    parameters.push_back(parameter.value());
  }

  return CreateNode<Assembly>(assembly, std::move(parameters))();
}

auto AstReader::ReadKindReturn() -> NullableFlowPtr<Return> {
  if (!NextIf<std::string>("expr")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> expression;
  if (NextIf<none>()) {
    expression = nullptr;
  } else {
    expression = DeserializeExpression();
    if (!expression.has_value()) {
      return nullptr;
    }
  }

  return CreateNode<Return>(expression)();
}

auto AstReader::ReadKindRetif() -> NullableFlowPtr<ReturnIf> {
  if (!NextIf<std::string>("cond")) {
    return nullptr;
  }

  auto condition = DeserializeExpression();
  if (!condition.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("expr")) {
    return nullptr;
  }

  auto expression = DeserializeExpression();
  if (!expression.has_value()) {
    return nullptr;
  }

  return CreateNode<ReturnIf>(condition.value(), expression.value())();
}

NullableFlowPtr<Break> AstReader::ReadKindBreak() {  // NOLINT
  return CreateNode<Break>()();
}

NullableFlowPtr<Continue> AstReader::ReadKindContinue() {  // NOLINT
  return CreateNode<Continue>()();
}

auto AstReader::ReadKindIf() -> NullableFlowPtr<If> {
  if (!NextIf<std::string>("cond")) {
    return nullptr;
  }

  auto condition = DeserializeExpression();
  if (!condition.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("then")) {
    return nullptr;
  }

  auto then_block = DeserializeStatement();

  if (!NextIf<std::string>("else")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> else_block;
  if (NextIf<none>()) {
    else_block = nullptr;
  } else {
    else_block = DeserializeStatement();
    if (!else_block.has_value()) {
      return nullptr;
    }
  }

  return CreateNode<If>(condition.value(), then_block.value(), else_block)();
}

auto AstReader::ReadKindWhile() -> NullableFlowPtr<While> {
  if (!NextIf<std::string>("cond")) {
    return nullptr;
  }

  auto condition = DeserializeExpression();
  if (!condition.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  auto body = DeserializeStatement();
  if (!body.has_value()) {
    return nullptr;
  }

  return CreateNode<While>(condition.value(), body.value())();
}

auto AstReader::ReadKindFor() -> NullableFlowPtr<For> {
  if (!NextIf<std::string>("init")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> init;
  if (NextIf<none>()) {
    init = nullptr;
  } else {
    init = DeserializeStatement();
    if (!init.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("cond")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> condition;
  if (NextIf<none>()) {
    condition = nullptr;
  } else {
    condition = DeserializeExpression();
    if (!condition.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("step")) {
    return nullptr;
  }

  NullableFlowPtr<Expr> step;
  if (NextIf<none>()) {
    step = nullptr;
  } else {
    step = DeserializeExpression();
    if (!step.has_value()) {
      return nullptr;
    }
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  auto body = DeserializeStatement();
  if (!body.has_value()) {
    return nullptr;
  }

  return CreateNode<For>(init, condition, step, body.value())();
}

auto AstReader::ReadKindForeach() -> NullableFlowPtr<Foreach> {
  if (!NextIf<std::string>("idx") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto index_name = Next<std::string>();

  if (!NextIf<std::string>("val") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto value_name = Next<std::string>();

  if (!NextIf<std::string>("expr")) {
    return nullptr;
  }

  auto expression = DeserializeExpression();
  if (!expression.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  auto body = DeserializeStatement();
  if (!body.has_value()) {
    return nullptr;
  }

  return CreateNode<Foreach>(index_name, value_name, expression.value(),
                             body.value())();
}

auto AstReader::ReadKindCase() -> NullableFlowPtr<Case> {
  if (!NextIf<std::string>("match")) {
    return nullptr;
  }

  auto match = DeserializeExpression();
  if (!match.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  auto body = DeserializeStatement();
  if (!body.has_value()) {
    return nullptr;
  }

  return CreateNode<Case>(match.value(), body.value())();
}

auto AstReader::ReadKindSwitch() -> NullableFlowPtr<Switch> {
  if (!NextIf<std::string>("match")) {
    return nullptr;
  }

  auto match = DeserializeExpression();
  if (!match.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("cases") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto case_count = Next<uint64_t>();

  SwitchCases cases;
  cases.reserve(case_count);

  while (case_count-- > 0) {
    auto case_stmt = DeserializeStatement();
    if (!case_stmt.has_value() || !case_stmt.value()->Is(QAST_CASE)) {
      return nullptr;
    }

    cases.emplace_back(case_stmt.value()->As<Case>());
  }

  if (!NextIf<std::string>("default")) {
    return nullptr;
  }

  NullableFlowPtr<Stmt> default_case;
  if (NextIf<none>()) {
    default_case = nullptr;
  } else {
    default_case = DeserializeStatement();
    if (!default_case.has_value()) {
      return nullptr;
    }
  }

  return CreateNode<Switch>(match.value(), std::move(cases), default_case)();
}

auto AstReader::ReadKindExprStmt() -> NullableFlowPtr<ExprStmt> {
  if (!NextIf<std::string>("expr")) {
    return nullptr;
  }

  auto expr = DeserializeExpression();
  if (!expr.has_value()) {
    return nullptr;
  }

  return CreateNode<ExprStmt>(expr.value())();
}
