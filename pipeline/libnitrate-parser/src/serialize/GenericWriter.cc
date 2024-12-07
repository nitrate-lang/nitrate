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
    begin_obj();

    string("beg");
    uint64(n.get_start_pos());

    string("end");
    uint64(n.get_end_pos());

    string("src");
    string(std::get<2>(n.get_pos()));

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

  string("volatile");
  boolean(n.is_volatile());
}

void AST_Writer::visit(npar_node_t& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ExprStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("expr");
  n.get_expr()->accept(*this);

  end_obj();
}

void AST_Writer::visit(StmtExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("stmt");
  n.get_stmt()->accept(*this);

  end_obj();
}

void AST_Writer::visit(TypeExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("type");
  n.get_type()->accept(*this);

  end_obj();
}

void AST_Writer::visit(NamedTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(InferTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(TemplType& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U8& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U16& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U32& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U64& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(U128& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I8& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I16& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I32& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I64& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(I128& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F16& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F32& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F64& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(F128& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(VoidTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  end_obj();
}

void AST_Writer::visit(PtrTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  string("to");
  n.get_item()->accept(*this);

  end_obj();
}

void AST_Writer::visit(OpaqueTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(TupleTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  string("of");
  n.get_item()->accept(*this);

  string("size");
  n.get_size()->accept(*this);

  end_obj();
}

void AST_Writer::visit(RefTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  string("to");
  n.get_item()->accept(*this);

  end_obj();
}

void AST_Writer::visit(StructTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  write_type_metadata(n);

  { /* Write struct fields */
    string("fields");

    begin_obj();
    let fields = n.get_items();

    std::for_each(fields.begin(), fields.end(), [&](let field) {
      string(field.first);
      field.second->accept(*this);
    });

    end_obj();
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(FuncTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(UnaryExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("op");
  string(qlex_opstr(n.get_op()));

  string("rhs");
  n.get_rhs()->accept(*this);

  end_obj();
}

void AST_Writer::visit(BinExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("op");
  string(qlex_opstr(n.get_op()));

  string("lhs");
  n.get_lhs()->accept(*this);

  end_obj();
}

void AST_Writer::visit(TernaryExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstFloat& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstBool& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("value");
  boolean(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstString& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("value");
  string(n.get_value());

  end_obj();
}

void AST_Writer::visit(ConstChar& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("value");
  string(std::array<char, 2>{(char)n.get_value(), 0}.data());

  end_obj();
}

void AST_Writer::visit(ConstNull& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ConstUndef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(Call& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("callee");
  n.get_func()->accept(*this);

  { /* Write arguments */
    string("args");

    let args = n.get_args();
    begin_arr(args.size());
    std::for_each(args.begin(), args.end(), [&](let arg) {
      begin_obj();
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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("callee");
  n.get_func()->accept(*this);

  { /* Write template arguments */
    string("template");

    let args = n.get_template_args();
    begin_obj();
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
      begin_obj();
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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("key");
  n.get_key()->accept(*this);

  string("value");
  n.get_value()->accept(*this);

  end_obj();
}

void AST_Writer::visit(Field& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("field");
  string(n.get_field());

  string("base");
  n.get_base()->accept(*this);

  end_obj();
}

void AST_Writer::visit(Index& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("base");
  n.get_base()->accept(*this);

  string("index");
  n.get_index()->accept(*this);

  end_obj();
}

void AST_Writer::visit(Slice& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  { /* Write items */
    string("terms");

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(), [&](let item) {
      if (std::holds_alternative<String>(item)) {
        begin_obj();

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("name");
  string(n.get_name());

  end_obj();
}

void AST_Writer::visit(SeqPoint& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(InlineAsm& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(IfStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("cond");
  n.get_cond()->accept(*this);

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(ForStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("init");
  n.get_init()->accept(*this);

  string("cond");
  n.get_cond()->accept(*this);

  string("step");
  n.get_step()->accept(*this);

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(ForeachStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ContinueStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  end_obj();
}

void AST_Writer::visit(ReturnStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("cond");
  n.get_cond()->accept(*this);

  string("expr");
  n.get_value()->accept(*this);

  end_obj();
}

void AST_Writer::visit(CaseStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  string("match");
  n.get_cond() ? n.get_cond()->accept(*this) : null();

  string("body");
  n.get_body()->accept(*this);

  end_obj();
}

void AST_Writer::visit(SwitchStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

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

void AST_Writer::visit(TypedefDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(FnDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(FnDef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(StructField& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(StructDef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(EnumDef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ScopeDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ExportDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  write_source_location(n);

  /// TODO: Implement support for this node

  end_obj();
}
