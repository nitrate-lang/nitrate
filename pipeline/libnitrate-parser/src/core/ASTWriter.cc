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

#include <algorithm>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>

using namespace npar;

void AST_Writer::write_source_location(npar_node_t const& n) const {
  if (m_include_source_location) {
    string("loc");
    begin_obj(2);

    string("beg");
    if (n.get_offset() == QLEX_EOFF) {
      null();
    } else {
      uint64(n.get_offset());
    }

    string("fileid");
    if (n.get_fileid() == QLEX_NOFILE) {
      null();
    } else {
      uint64(n.get_fileid());
    }

    end_obj();
  }
}

void AST_Writer::write_type_metadata(Type const& n) {
  string("width");
  n.get_width() ? n.get_width()->accept(*this) : null();

  string("min");
  n.get_range().first ? n.get_range().first->accept(*this) : null();

  string("max");
  n.get_range().second ? n.get_range().second->accept(*this) : null();
}

std::string_view AST_Writer::vis_str(Vis vis) const {
  switch (vis) {
    case Vis::Sec:
      return "sec";
    case Vis::Pro:
      return "pro";
    case Vis::Pub:
      return "pub";
  }
}

void AST_Writer::visit(npar_node_t const& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ExprStmt const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("expr");
  n.get_expr()->accept(*this);

  end_obj();
}

void AST_Writer::visit(StmtExpr const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("stmt");
  n.get_stmt()->accept(*this);

  end_obj();
}

void AST_Writer::visit(TypeExpr const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("type");
  n.get_type()->accept(*this);

  end_obj();
}

void AST_Writer::visit(NamedTy const& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(InferTy const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(TemplType const& n) {
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
  std::for_each(args.begin(), args.end(), [&](let arg) {
    begin_obj(2);

    string("name");
    string(*std::get<0>(arg));

    string("value");
    std::get<1>(arg)->accept(*this);

    end_obj();
  });
  end_arr();

  end_obj();
}

void AST_Writer::visit(U1 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U8 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U16 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U32 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U64 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U128 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I8 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I16 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I32 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I64 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I128 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F16 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F32 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F64 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F128 const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(VoidTy const& n) {
  begin_obj(5);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(PtrTy const& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("to");
  n.get_item()->accept(*this);

  end_obj();
}

void AST_Writer::visit(OpaqueTy const& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(TupleTy const& n) {
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

void AST_Writer::visit(ArrayTy const& n) {
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

void AST_Writer::visit(RefTy const& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  string("to");
  n.get_item()->accept(*this);

  end_obj();
}

void AST_Writer::visit(FuncTy const& n) {
  begin_obj(10);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  write_type_metadata(n);

  { /* Write attributes */
    string("attributes");

    let attrs = n.get_attributes();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](let attr) { attr->accept(*this); });
    end_arr();
  }

  string("return");
  n.get_return()->accept(*this);

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

    case FuncPurity::QUASI: {
      string("thread_safe");
      boolean(true);

      string("purity");
      string("quasi");
      break;
    }

    case FuncPurity::RETRO: {
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

    let params = n.get_params();

    string("variadic");
    boolean(params.is_variadic);

    string("params");
    begin_arr(params.params.size());
    std::for_each(params.params.begin(), params.params.end(), [&](let param) {
      begin_obj(3);
      string("name");
      string(*std::get<0>(param));

      string("type");
      std::get<1>(param)->accept(*this);

      string("default");
      std::get<2>(param) ? std::get<2>(param)->accept(*this) : null();

      end_obj();
    });
    end_arr();

    end_obj();
  }

  end_obj();
}

void AST_Writer::visit(UnaryExpr const& n) {
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

void AST_Writer::visit(BinExpr const& n) {
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

void AST_Writer::visit(PostUnaryExpr const& n) {
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

void AST_Writer::visit(TernaryExpr const& n) {
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

void AST_Writer::visit(ConstInt const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstFloat const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstBool const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  boolean(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstString const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstChar const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("value");
  string(std::array<char, 2>{(char)n.get_value(), 0}.data());

  end_obj();
}

void AST_Writer::visit(ConstNull const& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ConstUndef const& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(Call const& n) {
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
      string(*arg.first);

      string("value");
      arg.second->accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(TemplCall const& n) {
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
      string(*arg.first);
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
      string(*arg.first);

      string("value");
      arg.second->accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(List const& n) {
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

void AST_Writer::visit(Assoc const& n) {
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

void AST_Writer::visit(Field const& n) {
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

void AST_Writer::visit(Index const& n) {
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

void AST_Writer::visit(Slice const& n) {
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

void AST_Writer::visit(FString const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  { /* Write items */
    string("terms");

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(), [&](let item) {
      if (std::holds_alternative<qcore::str_alias>(item)) {
        begin_obj(2);

        string("kind");
        let kind_name =
            npar_node_t::getKindName(npar_node_t::getTypeCode<ConstString>());
        string(kind_name);

        string("value");
        string(*std::get<qcore::str_alias>(item));

        end_obj();
      } else {
        std::get<Expr*>(item)->accept(*this);
      }
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(Ident const& n) {
  begin_obj(3);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(SeqPoint const& n) {
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

void AST_Writer::visit(Block const& n) {
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

void AST_Writer::visit(VarDecl const& n) {
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

void AST_Writer::visit(InlineAsm const& n) {
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

void AST_Writer::visit(IfStmt const& n) {
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

void AST_Writer::visit(WhileStmt const& n) {
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

void AST_Writer::visit(ForStmt const& n) {
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

void AST_Writer::visit(ForeachStmt const& n) {
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

void AST_Writer::visit(BreakStmt const& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ContinueStmt const& n) {
  begin_obj(2);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ReturnStmt const& n) {
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

void AST_Writer::visit(ReturnIfStmt const& n) {
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

void AST_Writer::visit(CaseStmt const& n) {
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

void AST_Writer::visit(SwitchStmt const& n) {
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

void AST_Writer::visit(TypedefStmt const& n) {
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

void AST_Writer::visit(Function const& n) {
  begin_obj(13);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  { /* Write attributes */
    string("attrs");

    let attrs = n.get_attributes();
    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](let attr) { attr->accept(*this); });
    end_arr();
  }

  { /* Purity */
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

      case FuncPurity::QUASI: {
        string("thread_safe");
        boolean(true);

        string("purity");
        string("quasi");
        break;
      }

      case FuncPurity::RETRO: {
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

    let captures = n.get_captures();
    begin_arr(captures.size());
    std::for_each(captures.begin(), captures.end(), [&](let cap) {
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
  string(n.get_name());

  { /* Write template parameters */
    string("template");

    if (let params = n.get_template_params()) {
      begin_arr(params->size());
      std::for_each(params->begin(), params->end(), [&](let param) {
        begin_obj(3);

        string("name");
        string(*std::get<0>(param));

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

  { /* Write parameters */
    string("input");
    begin_obj(2);

    let params = n.get_params();

    string("variadic");
    boolean(params.is_variadic);

    string("params");
    begin_arr(params.params.size());
    std::for_each(params.params.begin(), params.params.end(), [&](let param) {
      begin_obj(3);

      string("name");
      string(*std::get<0>(param));

      string("type");
      std::get<1>(param)->accept(*this);

      string("default");
      std::get<2>(param) ? std::get<2>(param)->accept(*this) : null();

      end_obj();
    });
    end_arr();

    end_obj();
  }

  string("return");
  n.get_return()->accept(*this);

  { /* Write pre conditions */
    string("precond");
    if (n.get_precond().has_value()) {
      n.get_precond().value()->accept(*this);
    } else {
      null();
    }
  }

  { /* Write post conditions */
    string("postcond");
    if (n.get_postcond().has_value()) {
      n.get_postcond().value()->accept(*this);
    } else {
      null();
    }
  }

  string("body");
  if (n.get_body().has_value()) {
    n.get_body().value()->accept(*this);
  } else {
    null();
  }

  end_obj();
}

void AST_Writer::visit(StructDef const& n) {
  begin_obj(10);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  { /* Write composite type */
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
  }

  { /* Write attributes */
    string("attrs");
    let attrs = n.get_attributes();

    begin_arr(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](let attr) { attr->accept(*this); });
    end_arr();
  }

  string("name");
  string(n.get_name());

  { /* Write template parameters */
    string("template");

    if (let params = n.get_template_params()) {
      begin_arr(params->size());
      std::for_each(params->begin(), params->end(), [&](let param) {
        begin_obj(3);

        string("name");
        string(*std::get<0>(param));

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

  { /* Write names */
    string("names");
    let names = n.get_names();
    begin_arr(names.size());
    std::for_each(names.begin(), names.end(), [&](let name) { string(*name); });
    end_arr();
  }

  { /* Write fields */
    string("fields");

    let fields = n.get_fields();
    begin_arr(fields.size());
    std::for_each(fields.begin(), fields.end(), [&](let field) {
      begin_obj(4);

      string("name");
      string(*field.get_name());

      string("type");
      field.get_type()->accept(*this);

      string("default");
      field.get_value().has_value() ? field.get_value().value()->accept(*this)
                                    : null();

      string("vis");
      string(vis_str(field.get_vis()));

      end_obj();
    });
    end_arr();
  }

  { /* Write methods */
    string("methods");

    let methods = n.get_methods();
    begin_arr(methods.size());
    std::for_each(methods.begin(), methods.end(), [&](let method) {
      begin_obj(2);

      string("vis");
      string(vis_str(method.vis));

      string("method");
      method.func->accept(*this);

      end_obj();
    });
    end_arr();
  }

  { /* Write static methods */
    string("statics");

    let statics = n.get_static_methods();
    begin_arr(statics.size());
    std::for_each(statics.begin(), statics.end(), [&](let method) {
      begin_obj(2);

      string("vis");
      string(vis_str(method.vis));

      string("method");
      method.func->accept(*this);

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(EnumDef const& n) {
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
      string(*item.first);

      string("value");
      item.second ? item.second->accept(*this) : null();

      end_obj();
    });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(ScopeStmt const& n) {
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
    std::for_each(deps.begin(), deps.end(), [&](let dep) { string(*dep); });
    end_arr();
  }

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(ExportStmt const& n) {
  begin_obj(6);

  string("kind");
  string(n.getKindName());

  write_source_location(n);

  string("abi");
  string(n.get_abi_name());

  string("vis");
  string(vis_str(n.get_vis()));

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
