#pragma once

#include <lsp/lang/FmtInterface.hh>
#include <nitrate-core/String.hh>
#include <sstream>
#include <stack>

namespace lsp::fmt {
  class CambrianFormatter final : public ncc::parse::ASTVisitor,
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
      LineStreamWritter& operator<<(ncc::lex::qlex_op_t op);
      LineStreamWritter& operator<<(ncc::parse::Vis op);
      LineStreamWritter& operator<<(ncc::core::str_alias str) {
        m_line_buffer << str.get();
        return *this;
      }

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

    void format_type_metadata(ncc::parse::Type const& n);

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

    void visit(ncc::parse::Base const& n) override;
    void visit(ncc::parse::ExprStmt const& n) override;
    void visit(ncc::parse::StmtExpr const& n) override;
    void visit(ncc::parse::TypeExpr const& n) override;
    void visit(ncc::parse::NamedTy const& n) override;
    void visit(ncc::parse::InferTy const& n) override;
    void visit(ncc::parse::TemplType const& n) override;
    void visit(ncc::parse::U1 const& n) override;
    void visit(ncc::parse::U8 const& n) override;
    void visit(ncc::parse::U16 const& n) override;
    void visit(ncc::parse::U32 const& n) override;
    void visit(ncc::parse::U64 const& n) override;
    void visit(ncc::parse::U128 const& n) override;
    void visit(ncc::parse::I8 const& n) override;
    void visit(ncc::parse::I16 const& n) override;
    void visit(ncc::parse::I32 const& n) override;
    void visit(ncc::parse::I64 const& n) override;
    void visit(ncc::parse::I128 const& n) override;
    void visit(ncc::parse::F16 const& n) override;
    void visit(ncc::parse::F32 const& n) override;
    void visit(ncc::parse::F64 const& n) override;
    void visit(ncc::parse::F128 const& n) override;
    void visit(ncc::parse::VoidTy const& n) override;
    void visit(ncc::parse::PtrTy const& n) override;
    void visit(ncc::parse::OpaqueTy const& n) override;
    void visit(ncc::parse::TupleTy const& n) override;
    void visit(ncc::parse::ArrayTy const& n) override;
    void visit(ncc::parse::RefTy const& n) override;
    void visit(ncc::parse::FuncTy const& n) override;
    void visit(ncc::parse::UnaryExpr const& n) override;
    void visit(ncc::parse::BinExpr const& n) override;
    void visit(ncc::parse::PostUnaryExpr const& n) override;
    void visit(ncc::parse::TernaryExpr const& n) override;
    void visit(ncc::parse::ConstInt const& n) override;
    void visit(ncc::parse::ConstFloat const& n) override;
    void visit(ncc::parse::ConstBool const& n) override;
    void visit(ncc::parse::ConstString const& n) override;
    void visit(ncc::parse::ConstChar const& n) override;
    void visit(ncc::parse::ConstNull const& n) override;
    void visit(ncc::parse::ConstUndef const& n) override;
    void visit(ncc::parse::Call const& n) override;
    void visit(ncc::parse::TemplCall const& n) override;
    void visit(ncc::parse::List const& n) override;
    void visit(ncc::parse::Assoc const& n) override;
    void visit(ncc::parse::Field const& n) override;
    void visit(ncc::parse::Index const& n) override;
    void visit(ncc::parse::Slice const& n) override;
    void visit(ncc::parse::FString const& n) override;
    void visit(ncc::parse::Ident const& n) override;
    void visit(ncc::parse::SeqPoint const& n) override;
    void visit(ncc::parse::Block const& n) override;
    void visit(ncc::parse::VarDecl const& n) override;
    void visit(ncc::parse::InlineAsm const& n) override;
    void visit(ncc::parse::IfStmt const& n) override;
    void visit(ncc::parse::WhileStmt const& n) override;
    void visit(ncc::parse::ForStmt const& n) override;
    void visit(ncc::parse::ForeachStmt const& n) override;
    void visit(ncc::parse::BreakStmt const& n) override;
    void visit(ncc::parse::ContinueStmt const& n) override;
    void visit(ncc::parse::ReturnStmt const& n) override;
    void visit(ncc::parse::ReturnIfStmt const& n) override;
    void visit(ncc::parse::CaseStmt const& n) override;
    void visit(ncc::parse::SwitchStmt const& n) override;
    void visit(ncc::parse::TypedefStmt const& n) override;
    void visit(ncc::parse::Function const& n) override;
    void visit(ncc::parse::StructDef const& n) override;
    void visit(ncc::parse::EnumDef const& n) override;
    void visit(ncc::parse::ScopeStmt const& n) override;
    void visit(ncc::parse::ExportStmt const& n) override;

  public:
    CambrianFormatter(std::ostream& out, size_t theTabSize = 2)
        : line(out), indent(0), tabSize(theTabSize) {
      reset_state();
      (void)tabSize;
    }
    virtual ~CambrianFormatter() = default;

    bool format(ncc::parse::Base* root) override {
      root->accept(*this);
      bool ok = !failed;
      reset_state();

      return ok;
    }
  };
}  // namespace lsp::fmt
