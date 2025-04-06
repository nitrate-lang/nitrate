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

#include <cstring>
#include <descent/Recurse.hh>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Algorithm.hh>
#include <nitrate-parser/Context.hh>

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

auto GeneralParser::Context::RecurseName() -> string {
  enum State {
    Start,
    RequireName,
    RequireScopeOrEnd,
    Exit,
  } state = Start;

  Token last;
  std::string name;

  while (state != Exit) {
    last = Peek();

    switch (state) {
      case Start: {
        if (last.Is<PuncScope>()) {
          name += "::";
          state = RequireName;
          Next();
        } else if (last.Is(Name)) {
          name += last.GetString();
          state = RequireScopeOrEnd;
          Next();
        } else {
          /* No identifier to parse */
          state = Exit;
        }
        break;
      }

      case RequireName: {
        if (last.Is(Name)) {
          name += last.GetString();
          state = RequireScopeOrEnd;
          Next();
        } else {
          Log << ParserSignal << last << "Expected identifier after '::'";
          name.clear();
          state = Exit;
          Next();  // Prevent infinite loops elsewhere
        }
        break;
      }

      case RequireScopeOrEnd: {
        if (last.Is<PuncScope>()) {
          name += "::";
          state = RequireName;
          Next();
        } else {
          state = Exit;
        }
        break;
      }

      case Exit: {
        break;
      }
    }
  }

  return name;
}

auto GeneralParser::Context::RecurseBlock(bool braces, bool single, BlockMode safety) -> FlowPtr<Block> {
  if (braces && !Next().Is<PuncLCur>()) {
    Log << ParserSignal << Current() << "Expected '{'";
  }

  auto block_start = Current().GetStart();
  std::vector<FlowPtr<Expr>> statements;

  while (true) {
    /* Ignore extra semicolons */
    if (NextIf<PuncSemi>()) {
      continue;
    }

    { /* Detect exit conditon */
      bool should_break = (braces && NextIf<PuncRCur>()) || (single && statements.size() == 1);

      if (!should_break && m.IsEof()) {
        if (braces) {
          Log << ParserSignal << Current() << "Expected '}'";
        }

        should_break = true;
      }

      if (should_break) {
        auto block = CreateBlock(statements, safety);
        block->SetOffset(block_start);

        return block;
      }
    }

    if (!Peek().Is(KeyW)) {
      auto expr = RecurseExpr({
          Token(Punc, PuncSemi),
      });

      if (!NextIf<PuncSemi>()) {
        Log << ParserSignal << Current() << "Expected ';' after statement expression";
      }

      statements.push_back(expr);
    } else {
      auto tok = Next();
      auto loc_start = tok.GetStart();
      NullableFlowPtr<Expr> r;

      switch (tok.GetKeyword()) {
        case Keyword::Scope: {
          r = RecurseScope();
          break;
        }

        case Pub: {  // they both declare external functions
          r = RecurseExport(Vis::Pub);
          break;
        }

        case Sec: {
          r = RecurseExport(Vis::Sec);
          break;
        }

        case Pro: {
          r = RecurseExport(Vis::Pro);
          break;
        }

        case lex::Import: {
          r = RecurseImport();
          break;
        }

        case Keyword::Type: {
          r = RecurseTypedef();
          break;
        }

        case Let: {
          for (const auto &variable : RecurseVariable(VariableType::Let)) {
            statements.push_back(variable);
          }
          break;
        }

        case Var: {
          for (const auto &variable : RecurseVariable(VariableType::Var)) {
            statements.push_back(variable);
          }
          break;
        }

        case Const: {
          for (const auto &variable : RecurseVariable(VariableType::Const)) {
            statements.push_back(variable);
          }
          break;
        }

        case Static: {
          Log << ParserSignal << Current()
              << "Static variables are not yet "
                 "supported";
          break;
        }

        case Keyword::Struct: {
          r = RecurseStruct(CompositeType::Struct);
          break;
        }

        case Region: {
          r = RecurseStruct(CompositeType::Region);
          break;
        }

        case Group: {
          r = RecurseStruct(CompositeType::Group);
          break;
        }

        case Class: {
          r = RecurseStruct(CompositeType::Class);
          break;
        }

        case Union: {
          r = RecurseStruct(CompositeType::Union);
          break;
        }

        case Opaque: {
          Log << ParserSignal << Current() << "Unexpected 'opaque' in block context";
          break;
        }

        case Keyword::Enum: {
          r = RecurseEnum();
          break;
        }

        case __FString: {
          r = RecurseFString();
          if (!NextIf<PuncSemi>()) {
            Log << ParserSignal << Current() << "Expected ';' after f-string expression";
          }
          break;
        }

        case Fn: {
          r = RecurseFunction(false);
          break;
        }

        case Safe: {
          if (Peek().Is<PuncLCur>()) {
            r = RecurseBlock(true, false, BlockMode::Safe);
          } else {
            r = RecurseBlock(false, true, BlockMode::Safe);
          }

          break;
        }

        case Unsafe: {
          if (Peek().Is<PuncLCur>()) {
            r = RecurseBlock(true, false, BlockMode::Unsafe);
          } else {
            r = RecurseBlock(false, true, BlockMode::Unsafe);
          }

          break;
        }

        case Pure: {
          Log << ParserSignal << Current() << "Unexpected 'pure' in block context";
          break;
        }

        case Impure: {
          Log << ParserSignal << Current() << "Unexpected 'impure' in block context";
          break;
        }

        case Quasi: {
          Log << ParserSignal << Current() << "Unexpected 'quasi' in block context";
          break;
        }

        case Retro: {
          Log << ParserSignal << Current() << "Unexpected 'retro' in block context";
          break;
        }

        case Inline: {
          Log << ParserSignal << Current() << "Unexpected 'inline' in block context";
          break;
        }

        case Foreign: {
          Log << ParserSignal << Current() << "Unexpected 'foreign' in block context";
          break;
        }

        case Promise: {
          Log << ParserSignal << Current() << "Unexpected 'promise' in block context";
          break;
        }

        case Keyword::If: {
          r = RecurseIf();
          break;
        }

        case Else: {
          Log << ParserSignal << Current() << "Unexpected 'else' in block context";
          break;
        }

        case Keyword::For: {
          r = RecurseFor();
          break;
        }

        case Keyword::While: {
          r = RecurseWhile();
          break;
        }

        case Do: {
          Log << ParserSignal << Current() << "Unexpected 'do' in block context";
          break;
        }

        case Keyword::Switch: {
          r = RecurseSwitch();
          break;
        }

        case Keyword::Break: {
          r = CreateBreak();
          if (!NextIf<PuncSemi>()) {
            Log << ParserSignal << Current() << "Expected ';' after 'break' statement";
          }

          break;
        }

        case Keyword::Continue: {
          r = CreateContinue();
          if (!NextIf<PuncSemi>()) {
            Log << ParserSignal << Current() << "Expected ';' after 'continue' statement";
          }
          break;
        }

        case Keyword::Return: {
          r = RecurseReturn();
          break;
        }

        case Keyword::Foreach: {
          r = RecurseForeach();
          break;
        }

        case Try: {
          r = RecurseTry();
          break;
        }

        case Catch: {
          Log << ParserSignal << Current() << "Unexpected 'catch' in block context";
          break;
        }

        case Throw: {
          r = RecurseThrow();
          break;
        }

        case Async: {
          Log << ParserSignal << Current() << "Unexpected 'async' in block context";
          break;
        }

        case Await: {
          r = RecurseAwait();
          break;
        }

        case __Asm__: {
          r = RecurseAssembly();
          break;
        }

        case Keyword::Null: {
          r = CreateNull();
          if (!NextIf<PuncSemi>()) {
            Log << ParserSignal << Current() << "Expected ';' after 'null' statement";
          }
          break;
        }

        case True: {
          r = CreateBoolean(true);
          if (!NextIf<PuncSemi>()) {
            Log << ParserSignal << Current() << "Expected ';' after 'true' statement";
          }
          break;
        }

        case False: {
          r = CreateBoolean(false);
          if (!NextIf<PuncSemi>()) {
            Log << ParserSignal << Current() << "Expected ';' after 'false' statement";
          }
          break;
        }

        case EscapeBlock: {
          RecurseEscapeBlock();  // We discard the block
          break;
        }

        case UnitAssert: {
          r = RecurseUnitAssert();
          break;
        }
      }

      if (r) {
        r.Unwrap()->SetOffset(loc_start);
        statements.push_back(r.Unwrap());
      }
    }
  }
}

GeneralParser::GeneralParser(ncc::lex::IScanner &lexer, std::shared_ptr<ncc::IEnvironment> env,
                             std::pmr::memory_resource &pool, const std::optional<ImportConfig> &import_config)
    : m_impl(std::make_unique<GeneralParser::Context>(lexer, import_config.value_or(ImportConfig::GetDefault(env)),
                                                      std::move(env), pool)) {}

GeneralParser::~GeneralParser() = default;

GeneralParser::GeneralParser(GeneralParser &&o) noexcept : m_impl(std::exchange(o.m_impl, nullptr)) {}

auto GeneralParser::operator=(GeneralParser &&o) noexcept -> GeneralParser & {
  if (this != &o) {
    m_impl = std::exchange(o.m_impl, nullptr);
  }

  return *this;
}

auto GeneralParser::GetLexer() -> lex::IScanner & {
  if (!m_impl) {
    qcore_panic("GeneralParser::GetLexer() called on moved-from object");
  }

  return m_impl->m_rd;
}

auto GeneralParser::Parse() -> ASTRoot {
  if (!m_impl) {
    qcore_panic("GeneralParser::Parse() called on moved-from object");
  }

  std::optional<ASTRoot> ast;

  { /* Assign the current context to thread-local global state */
    auto *rd_ptr = &m_impl->m_rd;
    ParserSwapScanner(rd_ptr);

    { /* Subscribe to events emitted by the parser */
      auto sub_id = Log->Subscribe([this](const LogMessage &m) {
        if (m.m_sev >= Error && m.m_by.GetKind() == ParserSignal.GetKind()) {
          m_impl->m_failed = true;
        }
      });

      { /* Configure the scanner to ignore comments */
        auto old_state = m_impl->m_rd.GetSkipCommentsState();
        m_impl->m_rd.SkipCommentsState(true);

        { /* Recursive descent parsing */
          auto node = m_impl->RecurseBlock(false, false, BlockMode::Unknown);
          if (m_impl->m_rd.HasError()) {
            Log << ParserSignal << "Some lexical errors have occurred";
          }

          ForEach<dfs_pre>(node, [&](auto c) {
            m_impl->m_failed |= !c || c->IsMock();
            return m_impl->m_failed ? IterOp::Abort : IterOp::Proceed;
          });

          ast = ASTRoot(node, !m_impl->m_failed);
        }

        m_impl->m_rd.SkipCommentsState(old_state);
      }

      Log->Unsubscribe(sub_id);
    }

    ParserSwapScanner(rd_ptr);
  }

  return ast.value();
}

auto ASTRoot::Check() const -> bool { return m_success; }
