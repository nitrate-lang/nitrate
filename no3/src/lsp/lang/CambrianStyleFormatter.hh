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

      void Reset() {
        m_line_buffer.str("");
        m_line_buffer.clear();
      }

      template <typename T>
      auto operator<<(const T& val) -> LineStreamWritter& {
        m_line_buffer << val;
        return *this;
      }
      auto operator<<(ncc::lex::Operator op) -> LineStreamWritter&;
      auto operator<<(parse::Vis op) -> LineStreamWritter&;
      auto operator<<(ncc::string str) -> LineStreamWritter& {
        m_line_buffer << str.Get();
        return *this;
      }

      auto operator<<(std::ostream& (*func)(std::ostream&)) -> LineStreamWritter&;

      auto Length() -> size_t { return m_line_buffer.tellp(); }
    };

    LineStreamWritter m_line;
    std::stack<size_t> m_field_indent_stack;
    size_t m_indent;
    const size_t m_tabSize;
    bool m_failed, m_did_root;

    void ResetAutomaton() {
      m_field_indent_stack = std::stack<size_t>();
      m_field_indent_stack.push(1);
      m_line.Reset();
      m_indent = 0;
      m_failed = false;
      m_did_root = false;
    }

    auto GetIndent() const -> std::string {
      if (m_indent == 0) {
        return "";
      }
      return std::string(m_indent, ' ');
    }
    auto EscapeCharLiteral(char ch) const -> std::string;
    auto EscapeStringLiteralChunk(std::string_view str) const -> std::string;
    void EscapeStringLiteral(std::string_view str, bool put_quotes = true);
    void WriteFloatLiteralChunk(std::string_view float_str);
    void WriteFloatLiteral(std::string_view float_str);

    void FormatTypeMetadata(FlowPtr<parse::Type> n);

    void WrapStmtBody(FlowPtr<parse::Stmt> n, size_t size_threshold,
                        bool use_arrow_if_wrapped);

    template <typename IterBegin, typename IterEnd>
    void IterateExceptLast(IterBegin beg, IterEnd end, auto body,
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
    void Iterate(IterBegin beg, IterEnd end, auto body) {
      size_t i = 0;
      for (auto it = beg; it != end; ++it, ++i) {
        body(*it, i);
      }
    }

    void PrintLineComments(FlowPtr<parse::Base> n);
    void PrintMultilineComments(FlowPtr<parse::Base> n);

    void Visit(FlowPtr<parse::Base> n) override;
    void Visit(FlowPtr<parse::ExprStmt> n) override;
    void Visit(FlowPtr<parse::StmtExpr> n) override;
    void Visit(FlowPtr<parse::TypeExpr> n) override;
    void Visit(FlowPtr<parse::NamedTy> n) override;
    void Visit(FlowPtr<parse::InferTy> n) override;
    void Visit(FlowPtr<parse::TemplType> n) override;
    void Visit(FlowPtr<parse::U1> n) override;
    void Visit(FlowPtr<parse::U8> n) override;
    void Visit(FlowPtr<parse::U16> n) override;
    void Visit(FlowPtr<parse::U32> n) override;
    void Visit(FlowPtr<parse::U64> n) override;
    void Visit(FlowPtr<parse::U128> n) override;
    void Visit(FlowPtr<parse::I8> n) override;
    void Visit(FlowPtr<parse::I16> n) override;
    void Visit(FlowPtr<parse::I32> n) override;
    void Visit(FlowPtr<parse::I64> n) override;
    void Visit(FlowPtr<parse::I128> n) override;
    void Visit(FlowPtr<parse::F16> n) override;
    void Visit(FlowPtr<parse::F32> n) override;
    void Visit(FlowPtr<parse::F64> n) override;
    void Visit(FlowPtr<parse::F128> n) override;
    void Visit(FlowPtr<parse::VoidTy> n) override;
    void Visit(FlowPtr<parse::PtrTy> n) override;
    void Visit(FlowPtr<parse::OpaqueTy> n) override;
    void Visit(FlowPtr<parse::TupleTy> n) override;
    void Visit(FlowPtr<parse::ArrayTy> n) override;
    void Visit(FlowPtr<parse::RefTy> n) override;
    void Visit(FlowPtr<parse::FuncTy> n) override;
    void Visit(FlowPtr<parse::UnaryExpr> n) override;
    void Visit(FlowPtr<parse::BinExpr> n) override;
    void Visit(FlowPtr<parse::PostUnaryExpr> n) override;
    void Visit(FlowPtr<parse::TernaryExpr> n) override;
    void Visit(FlowPtr<parse::ConstInt> n) override;
    void Visit(FlowPtr<parse::ConstFloat> n) override;
    void Visit(FlowPtr<parse::ConstBool> n) override;
    void Visit(FlowPtr<parse::ConstString> n) override;
    void Visit(FlowPtr<parse::ConstChar> n) override;
    void Visit(FlowPtr<parse::ConstNull> n) override;
    void Visit(FlowPtr<parse::ConstUndef> n) override;
    void Visit(FlowPtr<parse::Call> n) override;
    void Visit(FlowPtr<parse::TemplCall> n) override;
    void Visit(FlowPtr<parse::List> n) override;
    void Visit(FlowPtr<parse::Assoc> n) override;
    void Visit(FlowPtr<parse::Index> n) override;
    void Visit(FlowPtr<parse::Slice> n) override;
    void Visit(FlowPtr<parse::FString> n) override;
    void Visit(FlowPtr<parse::Ident> n) override;
    void Visit(FlowPtr<parse::SeqPoint> n) override;
    void Visit(FlowPtr<parse::Block> n) override;
    void Visit(FlowPtr<parse::VarDecl> n) override;
    void Visit(FlowPtr<parse::InlineAsm> n) override;
    void Visit(FlowPtr<parse::IfStmt> n) override;
    void Visit(FlowPtr<parse::WhileStmt> n) override;
    void Visit(FlowPtr<parse::ForStmt> n) override;
    void Visit(FlowPtr<parse::ForeachStmt> n) override;
    void Visit(FlowPtr<parse::BreakStmt> n) override;
    void Visit(FlowPtr<parse::ContinueStmt> n) override;
    void Visit(FlowPtr<parse::ReturnStmt> n) override;
    void Visit(FlowPtr<parse::ReturnIfStmt> n) override;
    void Visit(FlowPtr<parse::CaseStmt> n) override;
    void Visit(FlowPtr<parse::SwitchStmt> n) override;
    void Visit(FlowPtr<parse::TypedefStmt> n) override;
    void Visit(FlowPtr<parse::Function> n) override;
    void Visit(FlowPtr<parse::StructDef> n) override;
    void Visit(FlowPtr<parse::EnumDef> n) override;
    void Visit(FlowPtr<parse::ScopeStmt> n) override;
    void Visit(FlowPtr<parse::ExportStmt> n) override;

  public:
    CambrianFormatter(std::ostream& out, size_t the_tab_size = 2)
        : m_line(out), m_indent(0), m_tabSize(the_tab_size) {
      ResetAutomaton();
      (void)m_tabSize;
    }
    virtual ~CambrianFormatter() = default;

    auto Format(FlowPtr<parse::Base> root) -> bool override {
      root.Accept(*this);
      bool ok = !m_failed;
      ResetAutomaton();

      return ok;
    }
  };
}  // namespace lsp::fmt
