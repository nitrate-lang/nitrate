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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Node.h>

#include <algorithm>
#include <nitrate-parser/Writer.hh>

using namespace npar;

void AST_Writer::write_source_location(npar_node_t& n) const {
  if (m_include_source_location) {
    string("loc");
    begin_obj(2);

    string("beg");
    if (n.get_offset() == UINT32_MAX) {
      uint64(n.get_offset());
    } else {
      null();
    }

    string("fileid");
    if (n.get_fileid() == UINT32_MAX) {
      uint64(n.get_fileid());
    } else {
      null();
    }

    end_obj();
  }
}

void AST_Writer::write_type_metadata(Type& n) {
  string("width");
  n.get_width() ? n.get_width()->accept(*this) : null();

  string("min");
  n.get_range().first ? n.get_range().first->accept(*this) : null();

  string("max");
  n.get_range().second ? n.get_range().second->accept(*this) : null();
}

void AST_Writer::visit(npar_node_t& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ExprStmt& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("expr");
  n.get_expr()->accept(*this);

  end_obj();
}

void AST_Writer::visit(StmtExpr& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("stmt");
  n.get_stmt()->accept(*this);

  end_obj();
}

void AST_Writer::visit(TypeExpr& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("type");
  n.get_type()->accept(*this);

  end_obj();
}

void AST_Writer::visit(NamedTy& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(InferTy& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(TemplType& n) {
  begin_obj(7);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("template");
  n.get_template()->accept(*this);

  string("args");
  let args = n.get_args();
  begin_arr(args.size());
  std::for_each(args.begin(), args.end(), [&](let arg) { arg->accept(*this); });
  end_arr();

  end_obj();
}

void AST_Writer::visit(U1& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U8& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U16& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U32& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U64& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U128& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I8& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I16& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I32& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I64& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I128& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F16& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F32& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F64& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F128& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(VoidTy& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(PtrTy& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("to");
  n.get_item()->accept(*this);

  end_obj();
}

void AST_Writer::visit(OpaqueTy& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(TupleTy& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  { /* Write sub fields */
    string("fields");

    let fields = n.get_items();
    begin_arr(fields.size());
    std::for_each(fields.begin(), fields.end(),
                  [&](let field) { field->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(ArrayTy& n) {
  begin_obj(7);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("of");
  n.get_item()->accept(*this);

  string("size");
  n.get_size()->accept(*this);

  end_obj();
}

void AST_Writer::visit(RefTy& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("to");
  n.get_item()->accept(*this);

  end_obj();
}

void AST_Writer::visit(FuncTy& n) {
  begin_obj(12);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("return");
  n.get_return_ty()->accept(*this);

  string("variadic");
  boolean(n.is_variadic());

  string("foreign");
  boolean(n.is_foreign());

  string("noreturn");
  boolean(n.is_noreturn());

  switch (n.get_purity()) {
    case FuncPurity::IMPURE_THREAD_UNSAFE: {
      string("thread_safe");
      boolean(false);

      string("purity");
      string("impure");
      break;
    }

    case FuncPurity::IMPURE_THREAD_SAFE: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("impure");
      break;
    }

    case FuncPurity::PURE: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("pure");
      break;
    }

    case FuncPurity::QUASIPURE: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("quasipure");
      break;
    }

    case FuncPurity::RETROPURE: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("retropure");
      break;
    }
  }

  { /* Write parameters */
    string("params");

    let params = n.get_params();
    begin_arr(params.size());
    std::for_each(params.begin(), params.end(), [&](let param) {
      begin_obj(3);
      string("name");
      string(std::get<0>(param));

      string("type");
      std::get<1>(param)->accept(*this);

      string("default");
      std::get<2>(param) ? std::get<2>(param)->accept(*this) : null();

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(UnaryExpr& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("op");
  string(qlex_opstr(n.get_op()));

  string("rhs");
  n.get_rhs()->accept(*this);

  end_obj();
}

void AST_Writer::visit(BinExpr& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("op");
  string(qlex_opstr(n.get_op()));

  string("lhs");
  n.get_lhs()->accept(*this);

  string("rhs");
  n.get_rhs()->accept(*this);

  end_obj();
}

void AST_Writer::visit(PostUnaryExpr& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("op");
  string(qlex_opstr(n.get_op()));

  string("lhs");
  n.get_lhs()->accept(*this);

  end_obj();
}

void AST_Writer::visit(TernaryExpr& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("cond");
  n.get_cond()->accept(*this);

  string("lhs");
  n.get_lhs()->accept(*this);

  string("rhs");
  n.get_rhs()->accept(*this);

  end_obj();
}

void AST_Writer::visit(ConstInt& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstFloat& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstBool& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  boolean(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstString& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstChar& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(std::array<char, 2>{(char)n.get_value(), 0}.data());

  end_obj();
}

void AST_Writer::visit(ConstNull& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ConstUndef& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(Call& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("callee");
  n.get_func()->accept(*this);

  { /* Write arguments */
    string("args");

    let args = n.get_args();
    begin_arr(args.size());
    std::for_each(args.begin(), args.end(), [&](let arg) {
      begin_obj(2);

      string("name");
      string(arg.first);

      string("value");
      arg.second->accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(TemplCall& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("callee");
  n.get_func()->accept(*this);

  { /* Write template arguments */
    string("template");

    let args = n.get_template_args();
    begin_obj(args.size());
    std::for_each(args.begin(), args.end(), [&](let arg) {
      string(arg.first);
      arg.second->accept(*this);
    });
    end_obj();
  }

  { /* Write arguments */
    string("args");

    let args = n.get_args();
    begin_arr(args.size());
    std::for_each(args.begin(), args.end(), [&](let arg) {
      begin_obj(2);

      string("name");
      string(arg.first);

      string("value");
      arg.second->accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(List& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  { /* Write elements */
    string("elements");

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](let item) { item->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(Assoc& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("key");
  n.get_key()->accept(*this);

  string("value");
  n.get_value()->accept(*this);

  end_obj();
}

void AST_Writer::visit(Field& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("field");
  string(n.get_field());

  string("base");
  n.get_base()->accept(*this);

  end_obj();
}

void AST_Writer::visit(Index& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("base");
  n.get_base()->accept(*this);

  string("index");
  n.get_index()->accept(*this);

  end_obj();
}

void AST_Writer::visit(Slice& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("base");
  n.get_base()->accept(*this);

  string("start");
  n.get_start()->accept(*this);

  string("end");
  n.get_end()->accept(*this);

  end_obj();
}

void AST_Writer::visit(FString& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  { /* Write items */
    string("terms");

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(), [&](let item) {
      if (std::holds_alternative<String>(item)) {
        begin_obj(2);

        string("kind");
        let kind_name =
            npar_node_t::getKindName(npar_node_t::getTypeCode<ConstString>());
        string(kind_name);

        string("value");
        string(std::get<String>(item));

        end_obj();
      } else {
        std::get<Expr*>(item)->accept(*this);
      }
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(Ident& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(SeqPoint& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  { /* Write items */
    string("terms");

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](let item) { item->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(Block& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  { /* Write safety profile */
    string("safe");

    switch (n.get_safety()) {
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

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](let item) { item->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(VarDecl& n) {
  begin_obj(7);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("mode");
  switch (n.get_decl_type()) {
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
  string(n.get_name());

  string("type");
  n.get_type() ? n.get_type()->accept(*this) : null();

  string("value");
  n.get_value() ? n.get_value()->accept(*this) : null();

  { /* Write attributes */
    string("attributes");

    let attrs = n.get_attributes();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](let attr) { attr->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(InlineAsm& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("code");
  string(n.get_code());

  { /* Write arguments */
    string("params");

    let args = n.get_args();
    begin_arr(args.size());
    std::for_each(args.begin(), args.end(),
                  [&](let arg) { arg->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(IfStmt& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("cond");
  n.get_cond()->accept(*this);

  string("then");
  n.get_then()->accept(*this);

  string("else");
  if (n.get_else()) {
    n.get_else()->accept(*this);
  } else {
    null();
  }

  end_obj();
}

void AST_Writer::visit(WhileStmt& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("cond");
  n.get_cond()->accept(*this);

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(ForStmt& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("init");
  if (n.get_init()) {
    n.get_init().value()->accept(*this);
  } else {
    null();
  }

  string("cond");
  if (n.get_cond()) {
    n.get_cond().value()->accept(*this);
  } else {
    null();
  }

  string("step");
  if (n.get_step()) {
    n.get_step().value()->accept(*this);
  } else {
    null();
  }

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(ForeachStmt& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("idx");
  string(n.get_idx_ident());

  string("val");
  string(n.get_val_ident());

  string("expr");
  n.get_expr()->accept(*this);

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(BreakStmt& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ContinueStmt& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ReturnStmt& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("expr");
  if (n.get_value()) {
    n.get_value().value()->accept(*this);
  } else {
    null();
  }

  end_obj();
}

void AST_Writer::visit(ReturnIfStmt& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("cond");
  n.get_cond()->accept(*this);

  string("expr");
  n.get_value()->accept(*this);

  end_obj();
}

void AST_Writer::visit(CaseStmt& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("match");
  n.get_cond() ? n.get_cond()->accept(*this) : null();

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(SwitchStmt& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("match");
  n.get_cond()->accept(*this);

  { /* Write cases */
    string("cases");

    let cases = n.get_cases();
    begin_arr(cases.size());
    std::for_each(cases.begin(), cases.end(),
                  [&](let item) { item->accept(*this); });
    end_arr();
  }

  string("default");
  n.get_default() ? n.get_default()->accept(*this) : null();

  end_obj();
}

void AST_Writer::visit(TypedefStmt& n) {
  begin_obj(4);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  string("type");
  n.get_type()->accept(*this);

  end_obj();
}

void AST_Writer::visit(FnDecl& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  string("type");
  n.get_type()->accept(*this);

  { /* Write parameters */
    string("template");

    if (let params = n.get_template_params()) {
      begin_arr(params->size());
      std::for_each(params->begin(), params->end(), [&](let param) {
        begin_obj(3);

        string("name");
        string(std::get<0>(param));

        string("type");
        std::get<1>(param)->accept(*this);

        string("default");
        std::get<2>(param) ? std::get<2>(param)->accept(*this) : null();

        end_obj();
      });
      end_arr();
    } else {
      null();
    }
  }

  end_obj();
}

void AST_Writer::visit(FnDef& n) {
  begin_obj(9);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  string("type");
  n.get_type()->accept(*this);

  { /* Write capture group */
    string("capture");

    let captures = n.get_captures();
    begin_arr(captures.size());
    std::for_each(captures.begin(), captures.end(), [&](let cap) {
      begin_obj(2);

      string("name");
      string(cap.first);

      string("is_ref");
      boolean(cap.second);

      end_obj();
    });
    end_arr();
  }

  { /* Write template parameters */
    string("template");

    if (let params = n.get_template_params()) {
      begin_arr(params->size());
      std::for_each(params->begin(), params->end(), [&](let param) {
        begin_obj(3);

        string("name");
        string(std::get<0>(param));

        string("type");
        std::get<1>(param)->accept(*this);

        string("default");
        std::get<2>(param) ? std::get<2>(param)->accept(*this) : null();

        end_obj();
      });
      end_arr();
    } else {
      null();
    }
  }

  string("precond");
  n.get_precond() ? n.get_precond()->accept(*this) : null();

  string("postcond");
  n.get_postcond() ? n.get_postcond()->accept(*this) : null();

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(StructField& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  string("vis");
  switch (n.get_visibility()) {
    case Vis::PUBLIC: {
      string("pub");
      break;
    }

    case Vis::PRIVATE: {
      string("sec");
      break;
    }

    case Vis::PROTECTED: {
      string("pro");
      break;
    }
  }

  string("type");
  n.get_type()->accept(*this);

  string("init");
  n.get_value() ? n.get_value()->accept(*this) : null();

  end_obj();
}

void AST_Writer::visit(StructDef& n) {
  begin_obj(8);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  string("mode");
  switch (n.get_composite_type()) {
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

  { /* Write template parameters */
    string("template");

    if (let params = n.get_template_params()) {
      begin_arr(params->size());
      std::for_each(params->begin(), params->end(), [&](let param) {
        begin_obj(3);

        string("name");
        string(std::get<0>(param));

        string("type");
        std::get<1>(param)->accept(*this);

        string("default");
        std::get<2>(param) ? std::get<2>(param)->accept(*this) : null();

        end_obj();
      });
      end_arr();
    } else {
      null();
    }
  }

  { /* Write fields */
    string("fields");

    let fields = n.get_fields();
    begin_arr(fields.size());
    std::for_each(fields.begin(), fields.end(),
                  [&](let field) { field->accept(*this); });
    end_arr();
  }

  { /* Write methods */
    string("methods");

    let methods = n.get_methods();
    begin_arr(methods.size());
    std::for_each(methods.begin(), methods.end(),
                  [&](let method) { method->accept(*this); });
    end_arr();
  }

  { /* Write static methods */
    string("statics");

    let statics = n.get_static_methods();
    begin_arr(statics.size());
    std::for_each(statics.begin(), statics.end(),
                  [&](let method) { method->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(EnumDef& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  string("type");
  n.get_type() ? n.get_type()->accept(*this) : null();

  { /* Write items */
    string("fields");

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(), [&](let item) {
      begin_obj(2);

      string("name");
      string(item.first);

      string("value");
      item.second ? item.second->accept(*this) : null();

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(ScopeStmt& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  { /* Write implicit dependencies */
    string("depends");

    let deps = n.get_deps();
    begin_arr(deps.size());
    std::for_each(deps.begin(), deps.end(), [&](let dep) { string(dep); });
    end_arr();
  }

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(ExportStmt& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("abi");
  string(n.get_abi_name());

  string("vis");
  switch (n.get_vis()) {
    case Vis::PUBLIC: {
      string("pub");
      break;
    }

    case Vis::PRIVATE: {
      string("sec");
      break;
    }

    case Vis::PROTECTED: {
      string("pro");
      break;
    }
  }

  { /* Write attributes */
    string("attrs");

    let attrs = n.get_attrs();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](let attr) { attr->accept(*this); });
    end_arr();
  }

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}
