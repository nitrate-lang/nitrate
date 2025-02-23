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

#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFactory.hh>

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
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateInteger(string x, SourceLocation origin) -> std::optional<FlowPtr<Integer>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateInteger(const boost::multiprecision::cpp_int& x,
                               SourceLocation origin) -> std::optional<FlowPtr<Integer>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFloat(double x, SourceLocation origin) -> FlowPtr<Float> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFloat(string x, SourceLocation origin) -> std::optional<FlowPtr<Float>> {
  /// TODO: Implement
  qcore_implement();
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
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateCall(std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
                            SourceLocation origin) -> FlowPtr<Call> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateList(std::span<const FlowPtr<Expr>> ele, SourceLocation origin) -> FlowPtr<List> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateList(const std::vector<FlowPtr<Expr>>& ele, SourceLocation origin) -> FlowPtr<List> {
  /// TODO: Implement
  qcore_implement();
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
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFormatString(std::span<const std::variant<FlowPtr<Expr>, string>> parts,
                                    SourceLocation origin) -> FlowPtr<FString> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFormatString(const std::vector<std::variant<FlowPtr<Expr>, string>>& parts,
                                    SourceLocation origin) -> FlowPtr<FString> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateIdentifier(string name, SourceLocation origin) -> FlowPtr<Identifier> {
  return CreateInstance<Identifier>(name)(m_pool, origin);
}

auto ASTFactory::CreateSequence(std::span<const FlowPtr<Expr>> ele, SourceLocation origin) -> FlowPtr<Sequence> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateSequence(const std::vector<FlowPtr<Expr>>& ele, SourceLocation origin) -> FlowPtr<Sequence> {
  /// TODO: Implement
  qcore_implement();
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
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateTemplateCall(std::span<const FlowPtr<Expr>> template_args,
                                    std::span<const FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
                                    SourceLocation origin) -> FlowPtr<Call> {
  /// TODO: Implement
  qcore_implement();
}
