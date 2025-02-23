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

#include <boost/multiprecision/cpp_int.hpp>
#include <charconv>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <sstream>
#include <string>

using namespace ncc::parse;

auto ASTFactory::CreateBinary(FlowPtr<Expr> lhs, lex::Operator op, FlowPtr<Expr> rhs,
                              SourceLocation origin) -> FlowPtr<Binary> {
  return CreateInstance<Binary>(lhs, op, rhs)(m_pool, origin);
}

auto ASTFactory::CreateUnary(lex::Operator op, FlowPtr<Expr> rhs, SourceLocation origin) -> FlowPtr<Unary> {
  return CreateInstance<Unary>(op, rhs)(m_pool, origin);
}

auto ASTFactory::CreatePostUnary(FlowPtr<Expr> lhs, lex::Operator op, SourceLocation origin) -> FlowPtr<PostUnary> {
  return CreateInstance<PostUnary>(lhs, op)(m_pool, origin);
}

auto ASTFactory::CreateTernary(FlowPtr<Expr> condition, FlowPtr<Expr> then, FlowPtr<Expr> ele,
                               SourceLocation origin) -> FlowPtr<Ternary> {
  return CreateInstance<Ternary>(condition, then, ele)(m_pool, origin);
}

auto ASTFactory::CreateInteger(const boost::multiprecision::uint128_type& x,
                               SourceLocation origin) -> FlowPtr<Integer> {
  auto int_str = boost::multiprecision::uint128_t(x).str();
  return CreateInstance<Integer>(int_str)(m_pool, origin);
}

static const boost::multiprecision::cpp_int MAX_UINT128("340282366920938463463374607431768211455");

auto ASTFactory::CreateInteger(string x, SourceLocation origin) -> std::optional<FlowPtr<Integer>> {
  if (!std::all_of(x->begin(), x->end(), ::isdigit)) {
    return std::nullopt;
  }

  if (boost::multiprecision::cpp_int(*x) <= MAX_UINT128) {
    return CreateInstance<Integer>(x)(m_pool, origin);
  }

  return std::nullopt;
}

auto ASTFactory::CreateInteger(const boost::multiprecision::cpp_int& x,
                               SourceLocation origin) -> std::optional<FlowPtr<Integer>> {
  if (x <= MAX_UINT128) {
    return CreateInstance<Integer>(x.str())(m_pool, origin);
  }

  return std::nullopt;
}

auto ASTFactory::CreateFloat(double x, SourceLocation origin) -> FlowPtr<Float> {
  std::stringstream ss;
  ss << std::fixed << x;
  return CreateInstance<Float>(ss.str())(m_pool, origin);
}

auto ASTFactory::CreateFloat(string x, SourceLocation origin) -> std::optional<FlowPtr<Float>> {
  double f;
  if (auto res = std::from_chars(x->data(), x->data() + x->size(), f);
      res.ec == std::errc() && res.ptr == x->data() + x->size()) {
    return CreateFloat(f, origin);
  }

  return std::nullopt;
}

auto ASTFactory::CreateString(string x, SourceLocation origin) -> FlowPtr<String> {
  return CreateInstance<String>(x)(m_pool, origin);
}

auto ASTFactory::CreateCharacter(char8_t x, SourceLocation origin) -> FlowPtr<Character> {
  return CreateInstance<Character>(x)(m_pool, origin);
}

auto ASTFactory::CreateBoolean(bool x, SourceLocation origin) -> FlowPtr<Boolean> {
  return CreateInstance<Boolean>(x)(m_pool, origin);
}

auto ASTFactory::CreateNull(SourceLocation origin) -> FlowPtr<Null> { return CreateInstance<Null>()(m_pool, origin); }

auto ASTFactory::CreateUndefined(SourceLocation origin) -> FlowPtr<Undefined> {
  return CreateInstance<Undefined>()(m_pool, origin);
}

auto ASTFactory::CreateCall(FlowPtr<Expr> callee,
                            const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args,
                            SourceLocation origin) -> std::optional<FlowPtr<Call>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateCall(const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Expr> callee,
                            SourceLocation origin) -> FlowPtr<Call> {
  return CreateCall(std::span(pos_args), std::move(callee), origin);
}

auto ASTFactory::CreateCall(std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
                            SourceLocation origin) -> FlowPtr<Call> {
  auto pos_args_copy = AllocateArray<CallArg>(pos_args.size());
  for (size_t i = 0; i < pos_args.size(); ++i) {
    pos_args_copy[i].first = std::to_string(i);
    pos_args_copy[i].second = pos_args[i];
  }

  return CreateInstance<Call>(callee, pos_args_copy)(m_pool, origin);
}

auto ASTFactory::CreateList(std::span<const FlowPtr<Expr>> ele, SourceLocation origin) -> FlowPtr<List> {
  auto ele_copy = AllocateArray<FlowPtr<Expr>>(ele.size());
  std::copy(ele.begin(), ele.end(), ele_copy.begin());

  return CreateInstance<List>(ele_copy)(m_pool, origin);
}

auto ASTFactory::CreateList(const std::vector<FlowPtr<Expr>>& ele, SourceLocation origin) -> FlowPtr<List> {
  return CreateList(std::span(ele), origin);
}

auto ASTFactory::CreateAssociation(FlowPtr<Expr> key, FlowPtr<Expr> value, SourceLocation origin) -> FlowPtr<Assoc> {
  return CreateInstance<Assoc>(key, value)(m_pool, origin);
}

auto ASTFactory::CreateIndex(FlowPtr<Expr> base, FlowPtr<Expr> index, SourceLocation origin) -> FlowPtr<Index> {
  return CreateInstance<Index>(base, index)(m_pool, origin);
}

auto ASTFactory::CreateSlice(FlowPtr<Expr> base, FlowPtr<Expr> start, FlowPtr<Expr> end,
                             SourceLocation origin) -> FlowPtr<Slice> {
  return CreateInstance<Slice>(base, start, end)(m_pool, origin);
}

auto ASTFactory::CreateFormatString(string x, SourceLocation origin) -> FlowPtr<FString> {
  auto ele = AllocateArray<std::variant<string, FlowPtr<Expr>>>(1);
  ele[0] = x;
  return CreateInstance<FString>(ele)(m_pool, origin);
}

auto ASTFactory::CreateFormatString(std::span<const std::variant<string, FlowPtr<Expr>>> parts,
                                    SourceLocation origin) -> FlowPtr<FString> {
  auto parts_copy = AllocateArray<std::variant<string, FlowPtr<Expr>>>(parts.size());
  std::copy(parts.begin(), parts.end(), parts_copy.begin());

  return CreateInstance<FString>(parts_copy)(m_pool, origin);
}

auto ASTFactory::CreateFormatString(const std::vector<std::variant<string, FlowPtr<Expr>>>& parts,
                                    SourceLocation origin) -> FlowPtr<FString> {
  return CreateFormatString(std::span(parts), origin);
}

auto ASTFactory::CreateIdentifier(string name, SourceLocation origin) -> FlowPtr<Identifier> {
  return CreateInstance<Identifier>(name)(m_pool, origin);
}

auto ASTFactory::CreateSequence(std::span<const FlowPtr<Expr>> ele, SourceLocation origin) -> FlowPtr<Sequence> {
  auto ele_copy = AllocateArray<FlowPtr<Expr>>(ele.size());
  std::copy(ele.begin(), ele.end(), ele_copy.begin());

  return CreateInstance<Sequence>(ele_copy)(m_pool, origin);
}

auto ASTFactory::CreateSequence(const std::vector<FlowPtr<Expr>>& ele, SourceLocation origin) -> FlowPtr<Sequence> {
  return CreateSequence(std::span(ele), origin);
}

auto ASTFactory::CreateTemplateCall(
    FlowPtr<Expr> callee, const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& template_args,
    const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args,
    SourceLocation origin) -> std::optional<FlowPtr<Call>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateTemplateCall(const std::vector<FlowPtr<Expr>>& template_args,
                                    const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Expr> callee,
                                    SourceLocation origin) -> FlowPtr<Call> {
  return CreateTemplateCall(std::span(template_args), std::span(pos_args), std::move(callee), origin);
}

auto ASTFactory::CreateTemplateCall(std::span<const FlowPtr<Expr>> template_args,
                                    std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
                                    SourceLocation origin) -> FlowPtr<Call> {
  /// TODO: Implement
  qcore_implement();
}
