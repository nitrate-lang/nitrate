#pragma once

#include <lsp/lang/FmtInterface.hh>
#include <nitrate-core/String.hh>
#include <sstream>
#include <stack>

namespace lsp::fmt {
  using namespace ncc;

  template <typename T>
  using FlowPtr = ncc::FlowPtr<T>;

  class CambrianFormatter final : public parse::ASTVisitor,
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
      LineStreamWritter& operator<<(ncc::lex::Operator op);
      LineStreamWritter& operator<<(parse::Vis op);
      LineStreamWritter& operator<<(ncc::string str) {
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

    void format_type_metadata(FlowPtr<parse::Type> n);

    void wrap_stmt_body(FlowPtr<parse::Stmt> n, size_t size_threshold,
                        bool use_arrow_if_wrapped);

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

    void print_line_comments(FlowPtr<parse::Base> n);
    void print_multiline_comments(FlowPtr<parse::Base> n);

    void visit(FlowPtr<parse::Base> n) override;
    void visit(FlowPtr<parse::ExprStmt> n) override;
    void visit(FlowPtr<parse::StmtExpr> n) override;
    void visit(FlowPtr<parse::TypeExpr> n) override;
    void visit(FlowPtr<parse::NamedTy> n) override;
    void visit(FlowPtr<parse::InferTy> n) override;
    void visit(FlowPtr<parse::TemplType> n) override;
    void visit(FlowPtr<parse::U1> n) override;
    void visit(FlowPtr<parse::U8> n) override;
    void visit(FlowPtr<parse::U16> n) override;
    void visit(FlowPtr<parse::U32> n) override;
    void visit(FlowPtr<parse::U64> n) override;
    void visit(FlowPtr<parse::U128> n) override;
    void visit(FlowPtr<parse::I8> n) override;
    void visit(FlowPtr<parse::I16> n) override;
    void visit(FlowPtr<parse::I32> n) override;
    void visit(FlowPtr<parse::I64> n) override;
    void visit(FlowPtr<parse::I128> n) override;
    void visit(FlowPtr<parse::F16> n) override;
    void visit(FlowPtr<parse::F32> n) override;
    void visit(FlowPtr<parse::F64> n) override;
    void visit(FlowPtr<parse::F128> n) override;
    void visit(FlowPtr<parse::VoidTy> n) override;
    void visit(FlowPtr<parse::PtrTy> n) override;
    void visit(FlowPtr<parse::OpaqueTy> n) override;
    void visit(FlowPtr<parse::TupleTy> n) override;
    void visit(FlowPtr<parse::ArrayTy> n) override;
    void visit(FlowPtr<parse::RefTy> n) override;
    void visit(FlowPtr<parse::FuncTy> n) override;
    void visit(FlowPtr<parse::UnaryExpr> n) override;
    void visit(FlowPtr<parse::BinExpr> n) override;
    void visit(FlowPtr<parse::PostUnaryExpr> n) override;
    void visit(FlowPtr<parse::TernaryExpr> n) override;
    void visit(FlowPtr<parse::ConstInt> n) override;
    void visit(FlowPtr<parse::ConstFloat> n) override;
    void visit(FlowPtr<parse::ConstBool> n) override;
    void visit(FlowPtr<parse::ConstString> n) override;
    void visit(FlowPtr<parse::ConstChar> n) override;
    void visit(FlowPtr<parse::ConstNull> n) override;
    void visit(FlowPtr<parse::ConstUndef> n) override;
    void visit(FlowPtr<parse::Call> n) override;
    void visit(FlowPtr<parse::TemplCall> n) override;
    void visit(FlowPtr<parse::List> n) override;
    void visit(FlowPtr<parse::Assoc> n) override;
    void visit(FlowPtr<parse::Index> n) override;
    void visit(FlowPtr<parse::Slice> n) override;
    void visit(FlowPtr<parse::FString> n) override;
    void visit(FlowPtr<parse::Ident> n) override;
    void visit(FlowPtr<parse::SeqPoint> n) override;
    void visit(FlowPtr<parse::Block> n) override;
    void visit(FlowPtr<parse::VarDecl> n) override;
    void visit(FlowPtr<parse::InlineAsm> n) override;
    void visit(FlowPtr<parse::IfStmt> n) override;
    void visit(FlowPtr<parse::WhileStmt> n) override;
    void visit(FlowPtr<parse::ForStmt> n) override;
    void visit(FlowPtr<parse::ForeachStmt> n) override;
    void visit(FlowPtr<parse::BreakStmt> n) override;
    void visit(FlowPtr<parse::ContinueStmt> n) override;
    void visit(FlowPtr<parse::ReturnStmt> n) override;
    void visit(FlowPtr<parse::ReturnIfStmt> n) override;
    void visit(FlowPtr<parse::CaseStmt> n) override;
    void visit(FlowPtr<parse::SwitchStmt> n) override;
    void visit(FlowPtr<parse::TypedefStmt> n) override;
    void visit(FlowPtr<parse::Function> n) override;
    void visit(FlowPtr<parse::StructDef> n) override;
    void visit(FlowPtr<parse::EnumDef> n) override;
    void visit(FlowPtr<parse::ScopeStmt> n) override;
    void visit(FlowPtr<parse::ExportStmt> n) override;

  public:
    CambrianFormatter(std::ostream& out, size_t theTabSize = 2)
        : line(out), indent(0), tabSize(theTabSize) {
      reset_state();
      (void)tabSize;
    }
    virtual ~CambrianFormatter() = default;

    bool format(FlowPtr<parse::Base> root) override {
      root.accept(*this);
      bool ok = !failed;
      reset_state();

      return ok;
    }
  };
}  // namespace lsp::fmt
