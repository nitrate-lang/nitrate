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

#include <descent/Recurse.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto GeneralParser::PImpl::RecurseUnitAssert() -> FlowPtr<Expr> {
  if (!NextIf<PuncLPar>()) [[unlikely]] {
    Log << SyntaxError << Current() << "Expected '(' to open the 'unit_assert' annotation";
  }

  string annotation;
  if (auto annotation_token = NextIf<Text>()) [[likely]] {
    annotation = annotation_token->GetString();
  } else {
    Log << SyntaxError << Current() << "Expected a string literal for the 'unit_assert' annotation";
  }

  if (!NextIf<PuncRPar>()) [[unlikely]] {
    Log << SyntaxError << Current() << "Expected ')' to close the 'unit_assert' annotation";
  }

  if (!NextIf<PuncColn>()) [[unlikely]] {
    Log << SyntaxError << Current() << "Expected ':' before return type in 'unit_assert' block";
  }

  if (!NextIf<Name>("bool")) [[unlikely]] {
    Log << SyntaxError << Current() << "Expected 'bool' as the return type in 'unit_assert' block";
  }

  auto test_body = RecurseBlock(true, false, BlockMode::Unknown);

  std::string unit_test_name = "__nitrate_unit_test_";
  unit_test_name += annotation;

  auto ephermal_section_attribute = m_fac
                                        .CreateCall(m_fac.CreateIdentifier("section"),
                                                    {
                                                        {0U, m_fac.CreateString(".azide.ephemeral")},
                                                    })
                                        .value();

  auto unit_test_function = m_fac
                                .CreateFunction(unit_test_name, m_fac.CreateU1(), {}, false, test_body, Purity::Impure,
                                                {ephermal_section_attribute}, nullptr, nullptr, {}, {})
                                .value();

  return unit_test_function;
}
