#pragma once

#include <nitrate-core/String.hh>
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFwd.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <sstream>

namespace no3::format {

  class CambrianFormatter final : public ncc::parse::ASTVisitor {
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
      auto operator<<(ncc::parse::Vis v) -> LineWriter&;
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

    void FormatTypeMetadata(const ncc::FlowPtr<ncc::parse::Type>& n);

    template <typename IterBegin, typename IterEnd>
    void IterateExceptLast(IterBegin beg, IterEnd end, auto body, auto if_not_last) {
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

    void PrintLineComments(const ncc::FlowPtr<ncc::parse::Expr>& n);
    void PrintMultilineComments(const ncc::FlowPtr<ncc::parse::Expr>& n);

    void Visit(ncc::FlowPtr<ncc::parse::NamedTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::InferTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::TemplateType> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::U1> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::U8> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::U16> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::U32> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::U64> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::U128> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::I8> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::I16> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::I32> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::I64> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::I128> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::F16> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::F32> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::F64> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::F128> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::VoidTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::PtrTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::OpaqueTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::TupleTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::ArrayTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::RefTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::FuncTy> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Unary> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Binary> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::PostUnary> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Ternary> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Integer> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Float> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Boolean> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::String> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Character> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Null> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Undefined> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Call> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::TemplateCall> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::List> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Assoc> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Index> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Slice> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::FString> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Identifier> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Sequence> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Block> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Variable> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Assembly> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::If> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::While> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::For> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Foreach> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Break> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Continue> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Return> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::ReturnIf> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Case> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Switch> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Typedef> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Function> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Struct> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Enum> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Scope> n) override;
    void Visit(ncc::FlowPtr<ncc::parse::Export> n) override;

  public:
    CambrianFormatter(std::ostream& out, size_t the_tab_size = 2) : m_line(out), m_tabSize(the_tab_size) { Reset(); }
    ~CambrianFormatter() override = default;

    [[nodiscard]] auto HasErrors() const { return m_failed; }

    void Reset() {
      m_line.Reset();
      m_indent = 0;
      m_failed = false;
      m_did_root = false;
    }
  };
}  // namespace no3::format
