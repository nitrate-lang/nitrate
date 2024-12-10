#pragma once

#include <lsp/lang/FmtInterface.hh>
#include <stack>

namespace lsp::fmt {
  class CambrianFormatter final : public npar::ASTVisitor,
                                  public ICodeFormatter {
    std::stack<size_t> field_indent_stack;
    std::stringstream file, line;
    size_t indent;
    const size_t tabSize;
    bool failed;

    void reset_state() {
      field_indent_stack = std::stack<size_t>();
      field_indent_stack.push(1);
      file = std::stringstream();
      line = std::stringstream();
      indent = 0;
      failed = false;
    }

    void flush_line();
    std::string escape_char_literal(char ch);
    void escape_string_literal_chunk(std::string_view str);
    void escape_string_literal(std::string_view str, bool put_quotes = true);
    void write_float_literal_chunk(std::string_view float_str);
    void write_float_literal(std::string_view float_str);

    void format_type_metadata(npar::Type& n);

    template <typename IterBegin, typename IterEnd>
    void iterate_except_last(IterBegin beg, IterEnd end, auto body,
                             auto if_not_last) {
      size_t i = 0;
      for (auto it = beg; it != end; ++it, ++i) {
        body(*it, i);
        if (std::next(it) != end) {
          if_not_last(it);
        }
      }
    }

    void visit(npar_node_t& n) override;
    void visit(npar::ExprStmt& n) override;
    void visit(npar::StmtExpr& n) override;
    void visit(npar::TypeExpr& n) override;
    void visit(npar::NamedTy& n) override;
    void visit(npar::InferTy& n) override;
    void visit(npar::TemplType& n) override;
    void visit(npar::U1& n) override;
    void visit(npar::U8& n) override;
    void visit(npar::U16& n) override;
    void visit(npar::U32& n) override;
    void visit(npar::U64& n) override;
    void visit(npar::U128& n) override;
    void visit(npar::I8& n) override;
    void visit(npar::I16& n) override;
    void visit(npar::I32& n) override;
    void visit(npar::I64& n) override;
    void visit(npar::I128& n) override;
    void visit(npar::F16& n) override;
    void visit(npar::F32& n) override;
    void visit(npar::F64& n) override;
    void visit(npar::F128& n) override;
    void visit(npar::VoidTy& n) override;
    void visit(npar::PtrTy& n) override;
    void visit(npar::OpaqueTy& n) override;
    void visit(npar::TupleTy& n) override;
    void visit(npar::ArrayTy& n) override;
    void visit(npar::RefTy& n) override;
    void visit(npar::FuncTy& n) override;
    void visit(npar::UnaryExpr& n) override;
    void visit(npar::BinExpr& n) override;
    void visit(npar::PostUnaryExpr& n) override;
    void visit(npar::TernaryExpr& n) override;
    void visit(npar::ConstInt& n) override;
    void visit(npar::ConstFloat& n) override;
    void visit(npar::ConstBool& n) override;
    void visit(npar::ConstString& n) override;
    void visit(npar::ConstChar& n) override;
    void visit(npar::ConstNull& n) override;
    void visit(npar::ConstUndef& n) override;
    void visit(npar::Call& n) override;
    void visit(npar::TemplCall& n) override;
    void visit(npar::List& n) override;
    void visit(npar::Assoc& n) override;
    void visit(npar::Field& n) override;
    void visit(npar::Index& n) override;
    void visit(npar::Slice& n) override;
    void visit(npar::FString& n) override;
    void visit(npar::Ident& n) override;
    void visit(npar::SeqPoint& n) override;
    void visit(npar::Block& n) override;
    void visit(npar::VarDecl& n) override;
    void visit(npar::InlineAsm& n) override;
    void visit(npar::IfStmt& n) override;
    void visit(npar::WhileStmt& n) override;
    void visit(npar::ForStmt& n) override;
    void visit(npar::ForeachStmt& n) override;
    void visit(npar::BreakStmt& n) override;
    void visit(npar::ContinueStmt& n) override;
    void visit(npar::ReturnStmt& n) override;
    void visit(npar::ReturnIfStmt& n) override;
    void visit(npar::CaseStmt& n) override;
    void visit(npar::SwitchStmt& n) override;
    void visit(npar::TypedefStmt& n) override;
    void visit(npar::FnDecl& n) override;
    void visit(npar::FnDef& n) override;
    void visit(npar::StructField& n) override;
    void visit(npar::StructDef& n) override;
    void visit(npar::EnumDef& n) override;
    void visit(npar::ScopeStmt& n) override;
    void visit(npar::ExportStmt& n) override;

  public:
    CambrianFormatter(size_t theTabSize = 2) : indent(0), tabSize(theTabSize) {
      reset_state();
    }
    virtual ~CambrianFormatter() = default;

    bool format(npar_node_t* root, std::ostream& out) override {
      root->accept(*this);
      file << line.str();
      out << file.str();
      bool ok = !failed;
      reset_state();

      return ok;
    }
  };
}  // namespace lsp::fmt
