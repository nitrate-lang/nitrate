#pragma once

#include <lsp/lang/FmtInterface.hh>
#include <nitrate-core/String.hh>
#include <sstream>
#include <stack>

namespace lsp::fmt {
  template <typename T>
  using RefNode = ncc::parse::RefNode<T>;

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
      LineStreamWritter& operator<<(ncc::lex::Operator op);
      LineStreamWritter& operator<<(ncc::parse::Vis op);
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

    void format_type_metadata(RefNode<const ncc::parse::Type> n);

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

    void visit(RefNode<const ncc::parse::Base> n) override;
    void visit(RefNode<const ncc::parse::ExprStmt> n) override;
    void visit(RefNode<const ncc::parse::StmtExpr> n) override;
    void visit(RefNode<const ncc::parse::TypeExpr> n) override;
    void visit(RefNode<const ncc::parse::NamedTy> n) override;
    void visit(RefNode<const ncc::parse::InferTy> n) override;
    void visit(RefNode<const ncc::parse::TemplType> n) override;
    void visit(RefNode<const ncc::parse::U1> n) override;
    void visit(RefNode<const ncc::parse::U8> n) override;
    void visit(RefNode<const ncc::parse::U16> n) override;
    void visit(RefNode<const ncc::parse::U32> n) override;
    void visit(RefNode<const ncc::parse::U64> n) override;
    void visit(RefNode<const ncc::parse::U128> n) override;
    void visit(RefNode<const ncc::parse::I8> n) override;
    void visit(RefNode<const ncc::parse::I16> n) override;
    void visit(RefNode<const ncc::parse::I32> n) override;
    void visit(RefNode<const ncc::parse::I64> n) override;
    void visit(RefNode<const ncc::parse::I128> n) override;
    void visit(RefNode<const ncc::parse::F16> n) override;
    void visit(RefNode<const ncc::parse::F32> n) override;
    void visit(RefNode<const ncc::parse::F64> n) override;
    void visit(RefNode<const ncc::parse::F128> n) override;
    void visit(RefNode<const ncc::parse::VoidTy> n) override;
    void visit(RefNode<const ncc::parse::PtrTy> n) override;
    void visit(RefNode<const ncc::parse::OpaqueTy> n) override;
    void visit(RefNode<const ncc::parse::TupleTy> n) override;
    void visit(RefNode<const ncc::parse::ArrayTy> n) override;
    void visit(RefNode<const ncc::parse::RefTy> n) override;
    void visit(RefNode<const ncc::parse::FuncTy> n) override;
    void visit(RefNode<const ncc::parse::UnaryExpr> n) override;
    void visit(RefNode<const ncc::parse::BinExpr> n) override;
    void visit(RefNode<const ncc::parse::PostUnaryExpr> n) override;
    void visit(RefNode<const ncc::parse::TernaryExpr> n) override;
    void visit(RefNode<const ncc::parse::ConstInt> n) override;
    void visit(RefNode<const ncc::parse::ConstFloat> n) override;
    void visit(RefNode<const ncc::parse::ConstBool> n) override;
    void visit(RefNode<const ncc::parse::ConstString> n) override;
    void visit(RefNode<const ncc::parse::ConstChar> n) override;
    void visit(RefNode<const ncc::parse::ConstNull> n) override;
    void visit(RefNode<const ncc::parse::ConstUndef> n) override;
    void visit(RefNode<const ncc::parse::Call> n) override;
    void visit(RefNode<const ncc::parse::TemplCall> n) override;
    void visit(RefNode<const ncc::parse::List> n) override;
    void visit(RefNode<const ncc::parse::Assoc> n) override;
    void visit(RefNode<const ncc::parse::Index> n) override;
    void visit(RefNode<const ncc::parse::Slice> n) override;
    void visit(RefNode<const ncc::parse::FString> n) override;
    void visit(RefNode<const ncc::parse::Ident> n) override;
    void visit(RefNode<const ncc::parse::SeqPoint> n) override;
    void visit(RefNode<const ncc::parse::Block> n) override;
    void visit(RefNode<const ncc::parse::VarDecl> n) override;
    void visit(RefNode<const ncc::parse::InlineAsm> n) override;
    void visit(RefNode<const ncc::parse::IfStmt> n) override;
    void visit(RefNode<const ncc::parse::WhileStmt> n) override;
    void visit(RefNode<const ncc::parse::ForStmt> n) override;
    void visit(RefNode<const ncc::parse::ForeachStmt> n) override;
    void visit(RefNode<const ncc::parse::BreakStmt> n) override;
    void visit(RefNode<const ncc::parse::ContinueStmt> n) override;
    void visit(RefNode<const ncc::parse::ReturnStmt> n) override;
    void visit(RefNode<const ncc::parse::ReturnIfStmt> n) override;
    void visit(RefNode<const ncc::parse::CaseStmt> n) override;
    void visit(RefNode<const ncc::parse::SwitchStmt> n) override;
    void visit(RefNode<const ncc::parse::TypedefStmt> n) override;
    void visit(RefNode<const ncc::parse::Function> n) override;
    void visit(RefNode<const ncc::parse::StructDef> n) override;
    void visit(RefNode<const ncc::parse::EnumDef> n) override;
    void visit(RefNode<const ncc::parse::ScopeStmt> n) override;
    void visit(RefNode<const ncc::parse::ExportStmt> n) override;

  public:
    CambrianFormatter(std::ostream& out, size_t theTabSize = 2)
        : line(out), indent(0), tabSize(theTabSize) {
      reset_state();
      (void)tabSize;
    }
    virtual ~CambrianFormatter() = default;

    bool format(RefNode<ncc::parse::Base> root) override {
      root.accept(*this);
      bool ok = !failed;
      reset_state();

      return ok;
    }
  };
}  // namespace lsp::fmt
