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
static inline bool StrictFromChars(
    const char* first, const char* last, Ty& value,
    std::chars_format fmt = std::chars_format::general) noexcept {
  auto res = std::from_chars(first, last, value, fmt);

  return res.ec == std::errc() && res.ptr == last;
}

template <typename Ty>
static inline bool StrictFromChars(const char* first, const char* last,
                                   Ty& value, int base = 10) noexcept {
  auto res = std::from_chars(first, last, value, base);

  return res.ec == std::errc() && res.ptr == last;
}

std::optional<FlowPtr<Base>> AstReader::Get() {
  if (!m_root) {
    if (auto root = DeserializeObject()) {
      m_root = root.value();
    }
  }

  return m_root;
}

std::optional<AstReader::LocationRange> AstReader::ReadLocationRange() {
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

    std::string src = Next<std::string>();

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

std::optional<AstReader::TypeMetadata> AstReader::ReadTypeMetadata() {
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

NullableFlowPtr<Base> AstReader::DeserializeObject() {
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
      r = ReadKindIdent();
      break;
    }

    case QAST_SEQ: {
      r = ReadKindSeqPoint();
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
      r = ReadKindTemplCall();
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
      r = ReadKindInlineAsm();
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

NullableFlowPtr<Stmt> AstReader::DeserializeStatement() {
  auto object = DeserializeObject();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->GetKind();
  if (kind < QAST__STMT_FIRST || kind > QAST__STMT_LAST) {
    return nullptr;
  }

  return object.value().as<Stmt>();
}

NullableFlowPtr<Expr> AstReader::DeserializeExpression() {
  auto object = DeserializeObject();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->GetKind();
  if (kind < QAST__EXPR_FIRST || kind > QAST__EXPR_LAST) {
    return nullptr;
  }

  return object.value().as<Expr>();
}

NullableFlowPtr<Type> AstReader::DeserializeType() {
  auto object = DeserializeObject();
  if (!object.has_value()) {
    return nullptr;
  }

  auto kind = object.value()->GetKind();
  if (kind < QAST__TYPE_FIRST || kind > QAST__TYPE_LAST) {
    return nullptr;
  }

  return object.value().as<Type>();
}

NullableFlowPtr<Base> AstReader::ReadKindNode() {
  return make<Base>(QAST_BASE)();
}

NullableFlowPtr<BinExpr> AstReader::ReadKindBinexpr() {
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

  return make<BinExpr>(lhs.value(), op_it->second, rhs.value())();
}

NullableFlowPtr<UnaryExpr> AstReader::ReadKindUnexpr() {
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

  return make<UnaryExpr>(op_it->second, rhs.value())();
}

NullableFlowPtr<PostUnaryExpr> AstReader::ReadKindPostUnexpr() {
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

  return make<PostUnaryExpr>(lhs.value(), op_it->second)();
}

NullableFlowPtr<TernaryExpr> AstReader::ReadKindTerexpr() {
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

  return make<TernaryExpr>(cond.value(), lhs.value(), rhs.value())();
}

NullableFlowPtr<ConstInt> AstReader::ReadKindInt() {
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

  return make<ConstInt>(std::move(value))();
}

NullableFlowPtr<ConstFloat> AstReader::ReadKindFloat() {
  if (!NextIf<std::string>("value") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto value = Next<std::string>();

  long double d = 0;
  if (!StrictFromChars(value.data(), value.data() + value.size(), d,
                       std::chars_format::fixed)) {
    return nullptr;
  }

  return make<ConstFloat>(std::move(value))();
}

NullableFlowPtr<ConstString> AstReader::ReadKindString() {
  if (!NextIf<std::string>("value") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto value = Next<std::string>();

  return make<ConstString>(std::move(value))();
}

NullableFlowPtr<ConstChar> AstReader::ReadKindChar() {
  if (!NextIf<std::string>("value") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto value = Next<uint64_t>();

  if (value > 255) {
    return nullptr;
  }

  return make<ConstChar>(value)();
}

NullableFlowPtr<ConstBool> AstReader::ReadKindBool() {
  if (!NextIf<std::string>("value") || !NextIs<bool>()) {
    return nullptr;
  }

  auto value = Next<bool>();

  return make<ConstBool>(value)();
}

NullableFlowPtr<ConstNull> AstReader::ReadKindNull() {
  return make<ConstNull>()();
}

NullableFlowPtr<ConstUndef> AstReader::ReadKindUndef() {
  return make<ConstUndef>()();
}

NullableFlowPtr<Call> AstReader::ReadKindCall() {
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

  while (argument_count--) {
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

  return make<Call>(callee.value(), std::move(arguments))();
}

NullableFlowPtr<List> AstReader::ReadKindList() {
  if (!NextIf<std::string>("elements") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto element_count = Next<uint64_t>();

  ExpressionList elements;
  elements.reserve(element_count);

  while (element_count--) {
    auto element = DeserializeExpression();
    if (!element.has_value()) {
      return nullptr;
    }

    elements.push_back(element.value());
  }

  return make<List>(std::move(elements))();
}

NullableFlowPtr<Assoc> AstReader::ReadKindAssoc() {
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

  return make<Assoc>(key.value(), value.value())();
}

NullableFlowPtr<Index> AstReader::ReadKindIndex() {
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

  return make<Index>(base.value(), index.value())();
}

NullableFlowPtr<Slice> AstReader::ReadKindSlice() {
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

  return make<Slice>(base.value(), start.value(), end.value())();
}

NullableFlowPtr<FString> AstReader::ReadKindFstring() {
  if (!NextIf<std::string>("terms") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto term_count = Next<uint64_t>();
  FStringItems terms;
  terms.reserve(term_count);

  while (term_count--) {
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

  return make<FString>(std::move(terms))();
}

NullableFlowPtr<Ident> AstReader::ReadKindIdent() {
  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  return make<Ident>(std::move(name))();
}

NullableFlowPtr<SeqPoint> AstReader::ReadKindSeqPoint() {
  if (!NextIf<std::string>("terms") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto expression_count = Next<uint64_t>();

  ExpressionList terms;
  terms.reserve(expression_count);

  while (expression_count--) {
    auto term = DeserializeExpression();
    if (!term.has_value()) {
      return nullptr;
    }

    terms.push_back(term.value());
  }

  return make<SeqPoint>(std::move(terms))();
}

NullableFlowPtr<StmtExpr> AstReader::ReadKindStmtExpr() {
  if (!NextIf<std::string>("stmt")) {
    return nullptr;
  }

  auto stmt = DeserializeStatement();
  if (!stmt.has_value()) {
    return nullptr;
  }

  return make<StmtExpr>(stmt.value())();
}

NullableFlowPtr<TypeExpr> AstReader::ReadKindTypeExpr() {
  if (!NextIf<std::string>("type")) {
    return nullptr;
  }

  auto type = DeserializeType();
  if (!type.has_value()) {
    return nullptr;
  }

  return make<TypeExpr>(type.value())();
}

NullableFlowPtr<TemplCall> AstReader::ReadKindTemplCall() {
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

  while (template_count--) {
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

  while (argument_count--) {
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

  return make<TemplCall>(callee.value(), std::move(arguments),
                         std::move(template_args))();
}

NullableFlowPtr<U1> AstReader::ReadKindU1() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U1>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<U8> AstReader::ReadKindU8() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U8>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<U16> AstReader::ReadKindU16() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U16>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<U32> AstReader::ReadKindU32() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U32>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<U64> AstReader::ReadKindU64() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U64>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<U128> AstReader::ReadKindU128() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<U128>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<I8> AstReader::ReadKindI8() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I8>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<I16> AstReader::ReadKindI16() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I16>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<I32> AstReader::ReadKindI32() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I32>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<I64> AstReader::ReadKindI64() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I64>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<I128> AstReader::ReadKindI128() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<I128>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<F16> AstReader::ReadKindF16() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F16>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<F32> AstReader::ReadKindF32() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F32>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<F64> AstReader::ReadKindF64() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F64>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<F128> AstReader::ReadKindF128() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<F128>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<VoidTy> AstReader::ReadKindVoid() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<VoidTy>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<RefTy> AstReader::ReadKindRef() {
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

  auto node = make<RefTy>(to.value())();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<PtrTy> AstReader::ReadKindPtr() {
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

  auto node = make<PtrTy>(to.value(), is_volatile)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<OpaqueTy> AstReader::ReadKindOpaque() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  auto node = make<OpaqueTy>(name)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<ArrayTy> AstReader::ReadKindArray() {
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

  auto node = make<ArrayTy>(of.value(), size.value())();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<TupleTy> AstReader::ReadKindTuple() {
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

  while (field_count--) {
    auto field = DeserializeType();
    if (!field.has_value()) {
      return nullptr;
    }

    fields.push_back(field.value());
  }

  auto node = make<TupleTy>(std::move(fields))();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<FuncTy> AstReader::ReadKindFuncTy() {
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

  while (attribute_count--) {
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

  while (parameter_count--) {
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

  auto node = make<FuncTy>(return_type.value(), parameters, variadic, purity,
                           attributes)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<NamedTy> AstReader::ReadKindUnres() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  if (!NextIf<std::string>("name") || !NextIs<std::string>()) {
    return nullptr;
  }

  auto name = Next<std::string>();

  auto node = make<NamedTy>(name)();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<InferTy> AstReader::ReadKindInfer() {
  auto info = ReadTypeMetadata();
  if (!info.has_value()) {
    return nullptr;
  }

  auto node = make<InferTy>()();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<TemplType> AstReader::ReadKindTempl() {
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

  while (argument_count--) {
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

  auto node = make<TemplType>(templ.value(), std::move(arguments))();
  node->SetWidth(info->m_width);
  node->SetRangeBegin(info->m_min);
  node->SetRangeEnd(info->m_max);

  return node;
}

NullableFlowPtr<TypedefStmt> AstReader::ReadKindTypedef() {
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

  return make<TypedefStmt>(name, type.value())();
}

NullableFlowPtr<StructDef> AstReader::ReadKindStruct() {
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

  while (attribute_count--) {
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

    while (template_count--) {
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

  StructDefNames names;
  names.reserve(names_count);

  while (names_count--) {
    if (!NextIs<std::string>()) {
      return nullptr;
    }

    auto arg_name = Next<std::string>();

    names.push_back(arg_name);
  }

  if (!NextIf<std::string>("fields") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto field_count = Next<uint64_t>();

  StructDefFields fields;
  fields.reserve(field_count);

  while (field_count--) {
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

    fields.emplace_back(StructField(visibility, is_static, field_name,
                                    type.value(), default_value));
  }

  if (!NextIf<std::string>("methods") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto method_count = Next<uint64_t>();

  StructDefMethods methods;
  methods.reserve(method_count);

  while (method_count--) {
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

  StructDefStaticMethods static_methods;
  static_methods.reserve(static_method_count);

  while (static_method_count--) {
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

  return make<StructDef>(mode, attributes, name, template_args, names, fields,
                         methods, static_methods)();
}

NullableFlowPtr<EnumDef> AstReader::ReadKindEnum() {
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

  EnumDefItems fields;
  fields.reserve(field_count);

  while (field_count--) {
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

  return make<EnumDef>(name, type, std::move(fields))();
}

NullableFlowPtr<Function> AstReader::ReadKindFunction() {
  if (!NextIf<std::string>("attributes") || !NextIs<uint64_t>()) {
    return nullptr;
  }

  auto attribute_count = Next<uint64_t>();

  ExpressionList attributes;
  attributes.reserve(attribute_count);

  while (attribute_count--) {
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

  while (capture_count--) {
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

    while (template_count--) {
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

  while (parameter_count--) {
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

  return make<Function>(std::move(attributes), purity, std::move(captures),
                        name, std::move(template_args), std::move(parameters),
                        variadic, return_type.value(), precond, postcond,
                        body)();
}

NullableFlowPtr<ScopeStmt> AstReader::ReadKindScope() {
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

  while (dependency_count--) {
    auto dependency = Next<std::string>();
    dependencies.push_back(dependency);
  }

  if (!NextIf<std::string>("body")) {
    return nullptr;
  }

  auto body = DeserializeStatement();
  if (!body.has_value()) {
    return nullptr;
  }

  return make<ScopeStmt>(name, body.value(), dependencies)();
}

NullableFlowPtr<ExportStmt> AstReader::ReadKindExport() {
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

  while (attribute_count--) {
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

  return make<ExportStmt>(body.value(), abi_name, visibility,
                          std::move(attributes))();
}

NullableFlowPtr<Block> AstReader::ReadKindBlock() {
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

  while (statement_count--) {
    auto stmt = DeserializeStatement();
    if (!stmt.has_value()) {
      return nullptr;
    }

    statements.push_back(stmt.value());
  }

  return make<Block>(std::move(statements), mode)();
}

NullableFlowPtr<VarDecl> AstReader::ReadKindLet() {
  if (!NextIf<std::string>("mode")) {
    return nullptr;
  }
  VarDeclType mode;

  if (NextIf<std::string>("let")) {
    mode = VarDeclType::Let;
  } else if (NextIf<std::string>("var")) {
    mode = VarDeclType::Var;
  } else if (NextIf<std::string>("const")) {
    mode = VarDeclType::Const;
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

  while (attribute_count--) {
    auto attribute = DeserializeExpression();
    if (!attribute.has_value()) {
      return nullptr;
    }

    attributes.push_back(attribute.value());
  }

  return make<VarDecl>(name, type, value, mode, std::move(attributes))();
}

NullableFlowPtr<InlineAsm> AstReader::ReadKindInlineAsm() {
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

  while (parameter_count--) {
    auto parameter = DeserializeExpression();
    if (!parameter.has_value()) {
      return nullptr;
    }

    parameters.push_back(parameter.value());
  }

  return make<InlineAsm>(assembly, std::move(parameters))();
}

NullableFlowPtr<ReturnStmt> AstReader::ReadKindReturn() {
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

  return make<ReturnStmt>(expression)();
}

NullableFlowPtr<ReturnIfStmt> AstReader::ReadKindRetif() {
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

  return make<ReturnIfStmt>(condition.value(), expression.value())();
}

NullableFlowPtr<BreakStmt> AstReader::ReadKindBreak() {
  return make<BreakStmt>()();
}

NullableFlowPtr<ContinueStmt> AstReader::ReadKindContinue() {
  return make<ContinueStmt>()();
}

NullableFlowPtr<IfStmt> AstReader::ReadKindIf() {
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

  return make<IfStmt>(condition.value(), then_block.value(), else_block)();
}

NullableFlowPtr<WhileStmt> AstReader::ReadKindWhile() {
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

  return make<WhileStmt>(condition.value(), body.value())();
}

NullableFlowPtr<ForStmt> AstReader::ReadKindFor() {
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

  return make<ForStmt>(init, condition, step, body.value())();
}

NullableFlowPtr<ForeachStmt> AstReader::ReadKindForeach() {
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

  return make<ForeachStmt>(index_name, value_name, expression.value(),
                           body.value())();
}

NullableFlowPtr<CaseStmt> AstReader::ReadKindCase() {
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

  return make<CaseStmt>(match.value(), body.value())();
}

NullableFlowPtr<SwitchStmt> AstReader::ReadKindSwitch() {
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

  while (case_count--) {
    auto case_stmt = DeserializeStatement();
    if (!case_stmt.has_value() || !case_stmt.value()->is(QAST_CASE)) {
      return nullptr;
    }

    cases.push_back(case_stmt.value()->as<CaseStmt>());
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

  return make<SwitchStmt>(match.value(), std::move(cases), default_case)();
}

NullableFlowPtr<ExprStmt> AstReader::ReadKindExprStmt() {
  if (!NextIf<std::string>("expr")) {
    return nullptr;
  }

  auto expr = DeserializeExpression();
  if (!expr.has_value()) {
    return nullptr;
  }

  return make<ExprStmt>(expr.value())();
}
