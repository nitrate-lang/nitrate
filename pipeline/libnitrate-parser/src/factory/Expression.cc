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

#include <nitrate-parser/ASTFactory.hh>

using namespace ncc::parse;

auto ASTFactory::CreateBinary(FlowPtr<Expr> lhs, lex::Operator op, FlowPtr<Expr> rhs,
                              SourceLocation dbgsrc) -> FlowPtr<Binary> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateUnary(lex::Operator op, FlowPtr<Expr> rhs, SourceLocation dbgsrc) -> FlowPtr<Unary> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreatePostUnary(FlowPtr<Expr> lhs, lex::Operator op, SourceLocation dbgsrc) -> FlowPtr<PostUnary> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateTernary(FlowPtr<Expr> condition, FlowPtr<Expr> then, FlowPtr<Expr> ele,
                               SourceLocation dbgsrc) -> FlowPtr<Ternary> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateInteger(const boost::multiprecision::uint128_type& x,
                               SourceLocation dbgsrc) -> FlowPtr<Integer> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateInteger(string x, SourceLocation dbgsrc) -> std::optional<FlowPtr<Integer>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateInteger(const boost::multiprecision::cpp_int& x,
                               SourceLocation dbgsrc) -> std::optional<FlowPtr<Integer>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFloat(double x, SourceLocation dbgsrc) -> FlowPtr<Float> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFloat(string x, SourceLocation dbgsrc) -> std::optional<FlowPtr<Float>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateString(string x, SourceLocation dbgsrc) -> FlowPtr<String> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateCharacter(char8_t x, SourceLocation dbgsrc) -> FlowPtr<Character> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateBoolean(bool x, SourceLocation dbgsrc) -> FlowPtr<Boolean> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateNull(SourceLocation dbgsrc) -> FlowPtr<Null> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateUndefined(SourceLocation dbgsrc) -> FlowPtr<Undefined> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateCall(FlowPtr<Expr> callee,
                            const std::unordered_map<std::variant<string, size_t>, FlowPtr<Expr>>& named_args,
                            SourceLocation dbgsrc) -> std::optional<FlowPtr<Call>> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateCall(const std::vector<FlowPtr<Expr>>& pos_args, FlowPtr<Expr> callee,
                            SourceLocation dbgsrc) -> FlowPtr<Call> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateCall(std::span<FlowPtr<Expr>> pos_args, FlowPtr<Expr> callee,
                            SourceLocation dbgsrc) -> FlowPtr<Call> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateList(std::span<FlowPtr<Expr>> ele, SourceLocation dbgsrc) -> FlowPtr<List> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateList(const std::vector<FlowPtr<Expr>>& ele, SourceLocation dbgsrc) -> FlowPtr<List> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateAssociation(FlowPtr<Expr> key, FlowPtr<Expr> x, SourceLocation dbgsrc) -> FlowPtr<Assoc> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateIndex(FlowPtr<Expr> base, FlowPtr<Expr> index, SourceLocation dbgsrc) -> FlowPtr<Index> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateSlice(FlowPtr<Expr> base, FlowPtr<Expr> start, FlowPtr<Expr> end,
                             SourceLocation dbgsrc) -> FlowPtr<Slice> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFormatString(string x, SourceLocation dbgsrc) -> FlowPtr<FString> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFormatString(std::span<std::variant<FlowPtr<Expr>, string>> parts,
                                    SourceLocation dbgsrc) -> FlowPtr<FString> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFormatString(const std::vector<std::variant<FlowPtr<Expr>, string>>& parts,
                                    SourceLocation dbgsrc) -> FlowPtr<FString> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateIdentifier(string name, SourceLocation dbgsrc) -> FlowPtr<Identifier> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateSequence(std::span<FlowPtr<Expr>> ele, SourceLocation dbgsrc) -> FlowPtr<Sequence> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateSequence(const std::vector<FlowPtr<Expr>>& ele, SourceLocation dbgsrc) -> FlowPtr<Sequence> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateTemplateCall(SourceLocation dbgsrc) -> FlowPtr<TemplateCall> {
  /// TODO: Implement
  qcore_implement();
}
