#pragma once

#include <lsp/lang/FmtInterface.hh>
#include <sstream>
#include <stack>

namespace lsp::fmt {
  class CambrianFormatter final : public npar::ASTVisitor,
                                  public ICodeFormatter {
    class LineStreamWritter {
      std::stringstream m_line_buffer;
      std::ostream& m_file;

    public:
      LineStreamWritter(std::ostream& out) : m_file(out) {}

      void reset() {
        m_line_buffer.str("");
        m_line_buffer.clear();
      }

      template <typename T>
      LineStreamWritter& operator<<(const T& val) {
        m_line_buffer << val;
        return *this;
      }
      LineStreamWritter& operator<<(qlex_op_t op);

      LineStreamWritter& operator<<(std::ostream& (*func)(std::ostream&));

      size_t length() { return m_line_buffer.tellp(); }
    };

    LineStreamWritter line;
    std::stack<size_t> field_indent_stack;
    size_t indent;
    const size_t tabSize;
    bool failed, did_root;

    void reset_state() {
      field_indent_stack = std::stack<size_t>();
      field_indent_stack.push(1);
      line.reset();
      indent = 0;
      failed = false;
      did_root = false;
    }

    std::string get_indent() const {
      if (indent == 0) {
        return "";
      }
      return std::string(indent, ' ');
    }
    std::string escape_char_literal(char ch) const;
    std::string escape_string_literal_chunk(std::string_view str) const;
    void escape_string_literal(std::string_view str, bool put_quotes = true);
    void write_float_literal_chunk(std::string_view float_str);
    void write_float_literal(std::string_view float_str);

    void format_type_metadata(npar::Type const& n);

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

    template <typename IterBegin, typename IterEnd>
    void iterate(IterBegin beg, IterEnd end, auto body) {
      size_t i = 0;
      for (auto it = beg; it != end; ++it, ++i) {
        body(*it, i);
      }
    }

    void visit(npar_node_t const& n) override;
    void visit(npar::ExprStmt const& n) override;
    void visit(npar::StmtExpr const& n) override;
    void visit(npar::TypeExpr const& n) override;
    void visit(npar::NamedTy const& n) override;
    void visit(npar::InferTy const& n) override;
    void visit(npar::TemplType const& n) override;
    void visit(npar::U1 const& n) override;
    void visit(npar::U8 const& n) override;
    void visit(npar::U16 const& n) override;
    void visit(npar::U32 const& n) override;
    void visit(npar::U64 const& n) override;
    void visit(npar::U128 const& n) override;
    void visit(npar::I8 const& n) override;
    void visit(npar::I16 const& n) override;
    void visit(npar::I32 const& n) override;
    void visit(npar::I64 const& n) override;
    void visit(npar::I128 const& n) override;
    void visit(npar::F16 const& n) override;
    void visit(npar::F32 const& n) override;
    void visit(npar::F64 const& n) override;
    void visit(npar::F128 const& n) override;
    void visit(npar::VoidTy const& n) override;
    void visit(npar::PtrTy const& n) override;
    void visit(npar::OpaqueTy const& n) override;
    void visit(npar::TupleTy const& n) override;
    void visit(npar::ArrayTy const& n) override;
    void visit(npar::RefTy const& n) override;
    void visit(npar::FuncTy const& n) override;
    void visit(npar::UnaryExpr const& n) override;
    void visit(npar::BinExpr const& n) override;
    void visit(npar::PostUnaryExpr const& n) override;
    void visit(npar::TernaryExpr const& n) override;
    void visit(npar::ConstInt const& n) override;
    void visit(npar::ConstFloat const& n) override;
    void visit(npar::ConstBool const& n) override;
    void visit(npar::ConstString const& n) override;
    void visit(npar::ConstChar const& n) override;
    void visit(npar::ConstNull const& n) override;
    void visit(npar::ConstUndef const& n) override;
    void visit(npar::Call const& n) override;
    void visit(npar::TemplCall const& n) override;
    void visit(npar::List const& n) override;
    void visit(npar::Assoc const& n) override;
    void visit(npar::Field const& n) override;
    void visit(npar::Index const& n) override;
    void visit(npar::Slice const& n) override;
    void visit(npar::FString const& n) override;
    void visit(npar::Ident const& n) override;
    void visit(npar::SeqPoint const& n) override;
    void visit(npar::Block const& n) override;
    void visit(npar::VarDecl const& n) override;
    void visit(npar::InlineAsm const& n) override;
    void visit(npar::IfStmt const& n) override;
    void visit(npar::WhileStmt const& n) override;
    void visit(npar::ForStmt const& n) override;
    void visit(npar::ForeachStmt const& n) override;
    void visit(npar::BreakStmt const& n) override;
    void visit(npar::ContinueStmt const& n) override;
    void visit(npar::ReturnStmt const& n) override;
    void visit(npar::ReturnIfStmt const& n) override;
    void visit(npar::CaseStmt const& n) override;
    void visit(npar::SwitchStmt const& n) override;
    void visit(npar::TypedefStmt const& n) override;
    void visit(npar::FnDecl const& n) override;
    void visit(npar::FnDef const& n) override;
    void visit(npar::StructField const& n) override;
    void visit(npar::StructDef const& n) override;
    void visit(npar::EnumDef const& n) override;
    void visit(npar::ScopeStmt const& n) override;
    void visit(npar::ExportStmt const& n) override;

  public:
    CambrianFormatter(std::ostream& out, size_t theTabSize = 2)
        : line(out), indent(0), tabSize(theTabSize) {
      reset_state();
      (void)tabSize;
    }
    virtual ~CambrianFormatter() = default;

    bool format(npar_node_t* root) override {
      root->accept(*this);
      bool ok = !failed;
      reset_state();

      return ok;
    }
  };
}  // namespace lsp::fmt
