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

#include <algorithm>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

void AstWriter::WriteSourceLocation(const FlowPtr<Base>& n) const {
  string("loc");

  if (m_rd.has_value()) {
    IScanner& rd = m_rd->get();

    begin_obj(3);

    auto begin = n->Begin(rd);
    auto end = n->End(rd);

    {
      string("begin");
      begin_obj(4);

      string("off");
      uint64(begin.GetOffset());

      string("row");
      uint64(begin.GetRow());

      string("col");
      uint64(begin.GetCol());

      string("src");
      string(begin.GetFilename());

      end_obj();
    }

    {
      string("end");
      begin_obj(4);

      string("off");
      uint64(end.GetOffset());

      string("row");
      uint64(end.GetRow());

      string("col");
      uint64(end.GetCol());

      string("src");
      string(end.GetFilename());

      end_obj();
    }

    {
      string("trace");

#if NITRATE_FLOWPTR_TRACE
      begin_obj(4);

      let origin = n.Trace();

      string("src");
      string(origin.File());

      string("sub");
      string(origin.Function());

      string("row");
      uint64(origin.Line());

      string("col");
      uint64(origin.Column());

      end_obj();
#else
      null();
#endif
    }

    end_obj();
  } else {
    null();
  }
}

void AstWriter::WriteTypeMetadata(const FlowPtr<Type>& n) {
  string("width");
  n->GetWidth() ? n->GetWidth().value().Accept(*this) : null();

  string("min");
  auto min = n->GetRangeBegin();
  min.has_value() ? min.value().Accept(*this) : null();

  string("max");
  auto max = n->GetRangeEnd();
  max.has_value() ? max.value().Accept(*this) : null();
}

std::string_view AstWriter::VisStr(Vis vis) {
  switch (vis) {
    case Vis::Sec:
      return "sec";
    case Vis::Pro:
      return "pro";
    case Vis::Pub:
      return "pub";
  }
}

void AstWriter::Visit(FlowPtr<Base> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ExprStmt> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("expr");
  n->GetExpr().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<StmtExpr> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("stmt");
  n->GetStmt().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<TypeExpr> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("type");
  n->GetType().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<NamedTy> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  string("name");
  string(n->GetName());

  end_obj();
}

void AstWriter::Visit(FlowPtr<InferTy> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<TemplType> n) {
  begin_obj(7);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  string("template");
  n->GetTemplate().Accept(*this);

  string("arguments");
  auto args = n->GetArgs();
  begin_arr(args.size());
  std::for_each(args.begin(), args.end(), [&](auto arg) {
    begin_obj(2);

    string("name");
    string(*std::get<0>(arg));

    string("value");
    std::get<1>(arg).Accept(*this);

    end_obj();
  });
  end_arr();

  end_obj();
}

void AstWriter::Visit(FlowPtr<U1> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<U8> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<U16> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<U32> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<U64> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<U128> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<I8> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<I16> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<I32> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<I64> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<I128> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<F16> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<F32> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<F64> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<F128> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<VoidTy> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<PtrTy> n) {
  begin_obj(7);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  string("volatile");
  boolean(n->IsVolatile());

  string("to");
  n->GetItem().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<OpaqueTy> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  string("name");
  string(n->GetName());

  end_obj();
}

void AstWriter::Visit(FlowPtr<TupleTy> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  { /* Write sub fields */
    string("fields");

    auto fields = n->GetItems();
    begin_arr(fields.size());
    std::for_each(fields.begin(), fields.end(),
                  [&](auto field) { field.Accept(*this); });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<ArrayTy> n) {
  begin_obj(7);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  string("of");
  n->GetItem().Accept(*this);

  string("size");
  n->GetSize().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<RefTy> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  string("to");
  n->GetItem().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<FuncTy> n) {
  begin_obj(10);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  WriteTypeMetadata(n);

  { /* Write attributes */
    string("attributes");

    auto attrs = n->GetAttributes();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](auto attr) { attr.Accept(*this); });
    end_arr();
  }

  string("return");
  n->GetReturn().Accept(*this);

  switch (n->GetPurity()) {
    case Purity::Impure: {
      string("thread_safe");
      boolean(false);

      string("purity");
      string("impure");
      break;
    }

    case Purity::Impure_TSafe: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("impure");
      break;
    }

    case Purity::Pure: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("pure");
      break;
    }

    case Purity::Quasi: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("quasi");
      break;
    }

    case Purity::Retro: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("retro");
      break;
    }
  }

  { /* Write parameters */
    string("input");
    begin_obj(2);

    string("variadic");
    boolean(n->IsVariadic());

    auto params = n->GetParams();
    string("parameters");
    begin_arr(params.size());
    std::for_each(params.begin(), params.end(), [&](auto param) {
      begin_obj(3);
      string("name");
      string(*std::get<0>(param));

      string("type");
      std::get<1>(param).Accept(*this);

      string("default");
      std::get<2>(param) ? std::get<2>(param).value().Accept(*this) : null();

      end_obj();
    });
    end_arr();

    end_obj();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<UnaryExpr> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("op");
  string(op_repr(n->GetOp()));

  string("rhs");
  n->GetRHS().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<BinExpr> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("op");
  string(op_repr(n->GetOp()));

  string("lhs");
  n->GetLHS().Accept(*this);

  string("rhs");
  n->GetRHS().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<PostUnaryExpr> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("op");
  string(op_repr(n->GetOp()));

  string("lhs");
  n->GetLHS().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<TernaryExpr> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("lhs");
  n->GetLHS().Accept(*this);

  string("rhs");
  n->GetRHS().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ConstInt> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  string(n->GetValue());

  end_obj();
}

void AstWriter::Visit(FlowPtr<ConstFloat> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  string(n->GetValue());

  end_obj();
}

void AstWriter::Visit(FlowPtr<ConstBool> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  boolean(n->GetValue());

  end_obj();
}

void AstWriter::Visit(FlowPtr<ConstString> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  string(n->GetValue());

  end_obj();
}

void AstWriter::Visit(FlowPtr<ConstChar> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  uint64(n->GetValue());

  end_obj();
}

void AstWriter::Visit(FlowPtr<ConstNull> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ConstUndef> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<Call> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("callee");
  n->GetFunc().Accept(*this);

  { /* Write arguments */
    string("arguments");

    auto args = n->GetArgs();
    begin_arr(args.size());
    std::for_each(args.begin(), args.end(), [&](auto arg) {
      begin_obj(2);

      string("name");
      string(*arg.first);

      string("value");
      arg.second.Accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<TemplCall> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("callee");
  n->GetFunc().Accept(*this);

  { /* Write template arguments */
    string("template");

    auto args = n->GetTemplateArgs();
    begin_arr(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      begin_obj(2);

      string("name");
      string(*arg.first);

      string("value");
      arg.second.Accept(*this);

      end_obj();
    });

    end_arr();
  }

  { /* Write arguments */
    string("arguments");

    auto args = n->GetArgs();
    begin_arr(args.size());
    std::for_each(args.begin(), args.end(), [&](auto arg) {
      begin_obj(2);

      string("name");
      string(*arg.first);

      string("value");
      arg.second.Accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<List> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  { /* Write elements */
    string("elements");

    auto items = n->GetItems();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { item.Accept(*this); });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<Assoc> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("key");
  n->GetKey().Accept(*this);

  string("value");
  n->GetValue().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<Index> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("base");
  n->GetBase().Accept(*this);

  string("index");
  n->GetIndex().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<Slice> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("base");
  n->GetBase().Accept(*this);

  string("start");
  n->GetStart().Accept(*this);

  string("end");
  n->GetEnd().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<FString> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  { /* Write items */
    string("terms");

    auto items = n->GetItems();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (std::holds_alternative<ncc::string>(item)) {
        begin_obj(1);

        string("value");
        string(*std::get<ncc::string>(item));

        end_obj();
      } else {
        std::get<FlowPtr<Expr>>(item).Accept(*this);
      }
    });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<Ident> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  end_obj();
}

void AstWriter::Visit(FlowPtr<SeqPoint> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  { /* Write items */
    string("terms");

    auto items = n->GetItems();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { item.Accept(*this); });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<Block> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  { /* Write safety profile */
    string("safe");

    switch (n->GetSafety()) {
      case SafetyMode::Unknown:
        null();
        break;
      case SafetyMode::Safe:
        string("yes");
        break;
      case SafetyMode::Unsafe:
        string("no");
        break;
    }
  }

  { /* Write body */
    string("body");

    auto items = n->GetItems();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { item.Accept(*this); });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<VarDecl> n) {
  begin_obj(7);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("mode");
  switch (n->GetDeclType()) {
    case VarDeclType::Const:
      string("const");
      break;
    case VarDeclType::Var:
      string("var");
      break;
    case VarDeclType::Let:
      string("let");
      break;
  }

  string("name");
  string(n->GetName());

  string("type");
  n->GetType() ? n->GetType().value().Accept(*this) : null();

  string("value");
  n->GetValue() ? n->GetValue().value().Accept(*this) : null();

  { /* Write attributes */
    string("attributes");

    auto attrs = n->GetAttributes();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](auto attr) { attr.Accept(*this); });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<InlineAsm> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("assembly");
  string(n->GetCode());

  { /* Write arguments */
    string("parameters");

    auto args = n->GetArgs();
    begin_arr(args.size());
    std::for_each(args.begin(), args.end(),
                  [&](auto arg) { arg.Accept(*this); });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<IfStmt> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("then");
  n->GetThen().Accept(*this);

  string("else");
  if (n->GetElse()) {
    n->GetElse().value().Accept(*this);
  } else {
    null();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<WhileStmt> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ForStmt> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("init");
  if (n->GetInit()) {
    n->GetInit().value().Accept(*this);
  } else {
    null();
  }

  string("cond");
  if (n->GetCond()) {
    n->GetCond().value().Accept(*this);
  } else {
    null();
  }

  string("step");
  if (n->GetStep()) {
    n->GetStep().value().Accept(*this);
  } else {
    null();
  }

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ForeachStmt> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("idx");
  string(n->GetIdxIdent());

  string("val");
  string(n->GetValIdent());

  string("expr");
  n->GetExpr().Accept(*this);

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<BreakStmt> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ContinueStmt> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ReturnStmt> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("expr");
  if (n->GetValue()) {
    n->GetValue().value().Accept(*this);
  } else {
    null();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<ReturnIfStmt> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("expr");
  n->GetValue().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<CaseStmt> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("match");
  n->GetCond().Accept(*this);

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<SwitchStmt> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("match");
  n->GetCond().Accept(*this);

  { /* Write cases */
    string("cases");

    auto cases = n->GetCases();
    begin_arr(cases.size());
    std::for_each(cases.begin(), cases.end(),
                  [&](auto item) { item.Accept(*this); });
    end_arr();
  }

  string("default");
  n->GetDefault() ? n->GetDefault().value().Accept(*this) : null();

  end_obj();
}

void AstWriter::Visit(FlowPtr<TypedefStmt> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  string("type");
  n->GetType().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<Function> n) {
  begin_obj(13);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  { /* Write attributes */
    string("attributes");

    auto attrs = n->GetAttributes();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](auto attr) { attr.Accept(*this); });
    end_arr();
  }

  { /* Purity */
    switch (n->GetPurity()) {
      case Purity::Impure: {
        string("thread_safe");
        boolean(false);

        string("purity");
        string("impure");
        break;
      }

      case Purity::Impure_TSafe: {
        string("thread_safe");
        boolean(true);

        string("purity");
        string("impure");
        break;
      }

      case Purity::Pure: {
        string("thread_safe");
        boolean(true);

        string("purity");
        string("pure");
        break;
      }

      case Purity::Quasi: {
        string("thread_safe");
        boolean(true);

        string("purity");
        string("quasi");
        break;
      }

      case Purity::Retro: {
        string("thread_safe");
        boolean(true);

        string("purity");
        string("retro");
        break;
      }
    }
  }

  { /* Write capture list */
    string("captures");

    auto captures = n->GetCaptures();
    begin_arr(captures.size());
    std::for_each(captures.begin(), captures.end(), [&](auto cap) {
      begin_obj(2);

      string("name");
      string(*cap.first);

      string("is_ref");
      boolean(cap.second);

      end_obj();
    });
    end_arr();
  }

  string("name");
  string(n->GetName());

  { /* Write template parameters */
    string("template");

    if (auto params = n->GetTemplateParams()) {
      begin_arr(params->size());
      std::for_each(params->begin(), params->end(), [&](auto param) {
        begin_obj(3);

        string("name");
        string(*std::get<0>(param));

        string("type");
        std::get<1>(param).Accept(*this);

        string("default");
        std::get<2>(param) ? std::get<2>(param).value().Accept(*this) : null();

        end_obj();
      });
      end_arr();
    } else {
      null();
    }
  }

  { /* Write parameters */
    string("input");
    begin_obj(2);

    string("variadic");
    boolean(n->IsVariadic());

    auto params = n->GetParams();
    string("parameters");
    begin_arr(params.size());
    std::for_each(params.begin(), params.end(), [&](auto param) {
      begin_obj(3);

      string("name");
      string(*std::get<0>(param));

      string("type");
      std::get<1>(param).Accept(*this);

      string("default");
      std::get<2>(param) ? std::get<2>(param).value().Accept(*this) : null();

      end_obj();
    });
    end_arr();

    end_obj();
  }

  string("return");
  n->GetReturn().Accept(*this);

  { /* Write pre conditions */
    string("precond");
    if (n->GetPrecond().has_value()) {
      n->GetPrecond().value().Accept(*this);
    } else {
      null();
    }
  }

  { /* Write post conditions */
    string("postcond");
    if (n->GetPostcond().has_value()) {
      n->GetPostcond().value().Accept(*this);
    } else {
      null();
    }
  }

  string("body");
  if (n->GetBody().has_value()) {
    n->GetBody().value().Accept(*this);
  } else {
    null();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<StructDef> n) {
  begin_obj(10);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  { /* Write composite type */
    string("mode");
    switch (n->GetCompositeType()) {
      case CompositeType::Region: {
        string("region");
        break;
      }

      case CompositeType::Struct: {
        string("struct");
        break;
      }

      case CompositeType::Group: {
        string("group");
        break;
      }

      case CompositeType::Class: {
        string("class");
        break;
      }

      case CompositeType::Union: {
        string("union");
        break;
      }
    }
  }

  { /* Write attributes */
    string("attributes");
    auto attrs = n->GetAttributes();

    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](auto attr) { attr.Accept(*this); });
    end_arr();
  }

  string("name");
  string(n->GetName());

  { /* Write template parameters */
    string("template");

    if (auto params = n->GetTemplateParams()) {
      begin_arr(params->size());
      std::for_each(params->begin(), params->end(), [&](auto param) {
        begin_obj(3);

        string("name");
        string(*std::get<0>(param));

        string("type");
        std::get<1>(param).Accept(*this);

        string("default");
        std::get<2>(param) ? std::get<2>(param).value().Accept(*this) : null();

        end_obj();
      });
      end_arr();
    } else {
      null();
    }
  }

  { /* Write names */
    string("names");
    auto names = n->GetNames();
    begin_arr(names.size());
    std::for_each(names.begin(), names.end(),
                  [&](auto name) { string(*name); });
    end_arr();
  }

  { /* Write fields */
    string("fields");

    auto fields = n->GetFields();
    begin_arr(fields.size());
    std::for_each(fields.begin(), fields.end(), [&](auto field) {
      begin_obj(5);

      string("name");
      string(field.GetName());

      string("type");
      field.GetType().Accept(*this);

      string("default");
      field.GetValue().has_value() ? field.GetValue().value().Accept(*this)
                                   : null();

      string("visibility");
      string(VisStr(field.GetVis()));

      string("static");
      boolean(field.IsStatic());

      end_obj();
    });
    end_arr();
  }

  { /* Write methods */
    string("methods");

    auto methods = n->GetMethods();
    begin_arr(methods.size());
    std::for_each(methods.begin(), methods.end(), [&](auto method) {
      begin_obj(2);

      string("visibility");
      string(VisStr(method.m_vis));

      string("method");
      method.m_func.Accept(*this);

      end_obj();
    });
    end_arr();
  }

  { /* Write static methods */
    string("static-methods");

    auto statics = n->GetStaticMethods();
    begin_arr(statics.size());
    std::for_each(statics.begin(), statics.end(), [&](auto method) {
      begin_obj(2);

      string("visibility");
      string(VisStr(method.m_vis));

      string("method");
      method.m_func.Accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<EnumDef> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  string("type");
  n->GetType() ? n->GetType().value().Accept(*this) : null();

  { /* Write items */
    string("fields");

    auto items = n->GetItems();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      begin_obj(2);

      string("name");
      string(*item.first);

      string("value");
      item.second ? item.second.value().Accept(*this) : null();

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AstWriter::Visit(FlowPtr<ScopeStmt> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  { /* Write implicit dependencies */
    string("depends");

    auto deps = n->GetDeps();
    begin_arr(deps.size());
    std::for_each(deps.begin(), deps.end(), [&](auto dep) { string(*dep); });
    end_arr();
  }

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void AstWriter::Visit(FlowPtr<ExportStmt> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("abi");
  string(n->GetAbiName());

  string("visibility");
  string(VisStr(n->GetVis()));

  { /* Write attributes */
    string("attributes");

    auto attrs = n->GetAttrs();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](auto attr) { attr.Accept(*this); });
    end_arr();
  }

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}
