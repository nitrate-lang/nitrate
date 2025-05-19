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

#pragma once

#include <format/tree/Visitor.hh>
#include <nitrate-core/String.hh>
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFwd.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/ASTVisitor.hh>

namespace no3::format::details {
  using namespace ncc;
  using namespace parse;

  class CanonicalFormatter final : public ASTVisitor {
    std::ostream& m_out;
    FormatterConfig m_profile;
    bool& m_has_errors;

    void Visit(FlowPtr<NamedTy> n) override;
    void Visit(FlowPtr<InferTy> n) override;
    void Visit(FlowPtr<TemplateType> n) override;
    void Visit(FlowPtr<OpaqueTy> n) override;
    void Visit(FlowPtr<TupleTy> n) override;
    void Visit(FlowPtr<ArrayTy> n) override;
    void Visit(FlowPtr<RefTy> n) override;
    void Visit(FlowPtr<FuncTy> n) override;
    void Visit(FlowPtr<Unary> n) override;
    void Visit(FlowPtr<Binary> n) override;
    void Visit(FlowPtr<Integer> n) override;
    void Visit(FlowPtr<Float> n) override;
    void Visit(FlowPtr<Boolean> n) override;
    void Visit(FlowPtr<String> n) override;
    void Visit(FlowPtr<Character> n) override;
    void Visit(FlowPtr<Call> n) override;
    void Visit(FlowPtr<TemplateCall> n) override;
    void Visit(FlowPtr<Import> n) override;
    void Visit(FlowPtr<List> n) override;
    void Visit(FlowPtr<Assoc> n) override;
    void Visit(FlowPtr<Index> n) override;
    void Visit(FlowPtr<Slice> n) override;
    void Visit(FlowPtr<FString> n) override;
    void Visit(FlowPtr<Identifier> n) override;
    void Visit(FlowPtr<Block> n) override;
    void Visit(FlowPtr<Variable> n) override;
    void Visit(FlowPtr<Assembly> n) override;
    void Visit(FlowPtr<If> n) override;
    void Visit(FlowPtr<While> n) override;
    void Visit(FlowPtr<For> n) override;
    void Visit(FlowPtr<Foreach> n) override;
    void Visit(FlowPtr<Break> n) override;
    void Visit(FlowPtr<Continue> n) override;
    void Visit(FlowPtr<Return> n) override;
    void Visit(FlowPtr<Case> n) override;
    void Visit(FlowPtr<Switch> n) override;
    void Visit(FlowPtr<Typedef> n) override;
    void Visit(FlowPtr<Function> n) override;
    void Visit(FlowPtr<Struct> n) override;
    void Visit(FlowPtr<Enum> n) override;
    void Visit(FlowPtr<Scope> n) override;
    void Visit(FlowPtr<Export> n) override;

  public:
    CanonicalFormatter(std::ostream& out, bool& has_errors, FormatterConfig config)
        : m_out(out), m_profile(config), m_has_errors(has_errors) {}
    ~CanonicalFormatter() override = default;
  };
}  // namespace no3::format::details
