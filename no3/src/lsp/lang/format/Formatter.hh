#pragma once

#include <lsp/lang/FmtInterface.hh>
#include <nitrate-core/String.hh>
#include <sstream>

namespace no3::lsp::fmt {
  using namespace ncc;

  template <typename T>
  using FlowPtr = ncc::FlowPtr<T>;

  class NCC_EXPORT CambrianFormatter final : public parse::ASTVisitor,
                                             public ICodeFormatter {
    class LineWriter final {
      std::stringstream m_line_buffer;
      std::ostream& m_file;

    public:
      LineWriter(std::ostream& out) : m_file(out) {}

      void Reset() {
        m_line_buffer.str("");
        m_line_buffer.clear();
      }

      auto Length() -> size_t { return m_line_buffer.tellp(); }

      auto operator<<(ncc::lex::Operator op) -> LineWriter&;
      auto operator<<(parse::Vis v) -> LineWriter&;
      auto operator<<(ncc::string str) -> LineWriter&;
      auto operator<<(std::ostream& (*func)(std::ostream&)) -> LineWriter&;

      template <typename T>
      auto operator<<(const T& val) -> LineWriter& {
        m_line_buffer << val;
        return *this;
      }
    };

    LineWriter m_line;
    size_t m_indent{};
    const size_t m_tabSize;
    bool m_failed, m_did_root;

    void ResetAutomaton() {
      m_line.Reset();
      m_indent = 0;
      m_failed = false;
      m_did_root = false;
    }

    auto GetIndent() const {
      if (m_indent == 0) {
        return std::string("");
      }

      return std::string(m_indent, ' ');
    }

    auto EscapeCharLiteral(char ch) const -> std::string;
    auto EscapeStringLiteralChunk(std::string_view str) const -> std::string;
    void EscapeStringLiteral(std::string_view str, bool put_quotes = true);
    void WriteFloatLiteralChunk(std::string_view float_str);
    void WriteFloatLiteral(std::string_view float_str);

    void FormatTypeMetadata(const FlowPtr<parse::Type>& n);

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

    void PrintLineComments(const FlowPtr<parse::Base>& n);
    void PrintMultilineComments(const FlowPtr<parse::Base>& n);

    void Visit(FlowPtr<parse::Base> n) override;
    void Visit(FlowPtr<parse::ExprStmt> n) override;
    void Visit(FlowPtr<parse::StmtExpr> n) override;
    void Visit(FlowPtr<parse::TypeExpr> n) override;
    void Visit(FlowPtr<parse::NamedTy> n) override;
    void Visit(FlowPtr<parse::InferTy> n) override;
    void Visit(FlowPtr<parse::TemplateType> n) override;
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
    void Visit(FlowPtr<parse::Unary> n) override;
    void Visit(FlowPtr<parse::Binary> n) override;
    void Visit(FlowPtr<parse::PostUnary> n) override;
    void Visit(FlowPtr<parse::Ternary> n) override;
    void Visit(FlowPtr<parse::Integer> n) override;
    void Visit(FlowPtr<parse::Float> n) override;
    void Visit(FlowPtr<parse::Boolean> n) override;
    void Visit(FlowPtr<parse::String> n) override;
    void Visit(FlowPtr<parse::Character> n) override;
    void Visit(FlowPtr<parse::Null> n) override;
    void Visit(FlowPtr<parse::Undefined> n) override;
    void Visit(FlowPtr<parse::Call> n) override;
    void Visit(FlowPtr<parse::TemplateCall> n) override;
    void Visit(FlowPtr<parse::List> n) override;
    void Visit(FlowPtr<parse::Assoc> n) override;
    void Visit(FlowPtr<parse::Index> n) override;
    void Visit(FlowPtr<parse::Slice> n) override;
    void Visit(FlowPtr<parse::FString> n) override;
    void Visit(FlowPtr<parse::Identifier> n) override;
    void Visit(FlowPtr<parse::Sequence> n) override;
    void Visit(FlowPtr<parse::Block> n) override;
    void Visit(FlowPtr<parse::Variable> n) override;
    void Visit(FlowPtr<parse::Assembly> n) override;
    void Visit(FlowPtr<parse::If> n) override;
    void Visit(FlowPtr<parse::While> n) override;
    void Visit(FlowPtr<parse::For> n) override;
    void Visit(FlowPtr<parse::Foreach> n) override;
    void Visit(FlowPtr<parse::Break> n) override;
    void Visit(FlowPtr<parse::Continue> n) override;
    void Visit(FlowPtr<parse::Return> n) override;
    void Visit(FlowPtr<parse::ReturnIf> n) override;
    void Visit(FlowPtr<parse::Case> n) override;
    void Visit(FlowPtr<parse::Switch> n) override;
    void Visit(FlowPtr<parse::Typedef> n) override;
    void Visit(FlowPtr<parse::Function> n) override;
    void Visit(FlowPtr<parse::Struct> n) override;
    void Visit(FlowPtr<parse::Enum> n) override;
    void Visit(FlowPtr<parse::Scope> n) override;
    void Visit(FlowPtr<parse::Export> n) override;

  public:
    CambrianFormatter(std::ostream& out, size_t the_tab_size = 2)
        : m_line(out), m_tabSize(the_tab_size) {
      ResetAutomaton();
      (void)m_tabSize;
    }
    ~CambrianFormatter() override = default;

    auto Format(FlowPtr<parse::Base> root) -> bool override {
      root.Accept(*this);
      bool ok = !m_failed;
      ResetAutomaton();

      return ok;
    }
  };
}  // namespace no3::lsp::fmt
