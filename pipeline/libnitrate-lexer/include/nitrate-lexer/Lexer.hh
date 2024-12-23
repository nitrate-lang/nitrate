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

#ifndef __NITRATE_LEXER_LEX_HH__
#define __NITRATE_LEXER_LEX_HH__

#include <boost/bimap.hpp>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Token.hh>
#include <ostream>
#include <unordered_map>

///============================================================================///

#define QLEX_FLAG_NONE 0
#define QLEX_NO_COMMENTS 0x01

namespace ncc::lex {
  namespace detail {
    template <typename L, typename R>
    boost::bimap<L, R> make_bimap(
        std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
      return boost::bimap<L, R>(list.begin(), list.end());
    }
  }  // namespace detail

  enum class OpType { Both, Unary, Binary, Ternary };
  enum class OpMode { Binary, PreUnary, PostUnary, Ternary };
  enum class OpAssoc { Left, Right };

  struct OpConfig {
    OpType type;
    bool overloadable;

    OpConfig(OpType t, bool o) : type(t), overloadable(o) {}

    bool operator<(const OpConfig &rhs) const {
      if (type != rhs.type) {
        return type < rhs.type;
      }
      return overloadable < rhs.overloadable;
    }
  };

  inline static const boost::bimap<std::string_view, Keyword> LexicalKeywords =
      detail::make_bimap<std::string_view, Keyword>({
          {"scope", qKScope},     {"import", qKImport},
          {"pub", qKPub},         {"sec", qKSec},
          {"pro", qKPro},         {"type", qKType},
          {"let", qKLet},         {"var", qKVar},
          {"const", qKConst},     {"static", qKStatic},
          {"struct", qKStruct},   {"region", qKRegion},
          {"group", qKGroup},     {"class", qKClass},
          {"union", qKUnion},     {"opaque", qKOpaque},
          {"enum", qKEnum},       {"__fstring", qK__FString},
          {"fn", qKFn},           {"unsafe", qKUnsafe},
          {"safe", qKSafe},       {"promise", qKPromise},
          {"if", qKIf},           {"else", qKElse},
          {"for", qKFor},         {"while", qKWhile},
          {"do", qKDo},           {"switch", qKSwitch},
          {"break", qKBreak},     {"continue", qKContinue},
          {"ret", qKReturn},      {"retif", qKRetif},
          {"foreach", qKForeach}, {"try", qKTry},
          {"catch", qKCatch},     {"throw", qKThrow},
          {"async", qKAsync},     {"await", qKAwait},
          {"__asm__", qK__Asm__}, {"undef", qKUndef},
          {"null", qKNull},       {"true", qKTrue},
          {"false", qKFalse},
      });

  inline static const boost::bimap<std::string_view, Operator>
      LexicalOperators = detail::make_bimap<std::string_view, Operator>({
          {"+", qOpPlus},
          {"-", qOpMinus},
          {"*", qOpTimes},
          {"/", qOpSlash},
          {"%", qOpPercent},
          {"&", qOpBitAnd},
          {"|", qOpBitOr},
          {"^", qOpBitXor},
          {"~", qOpBitNot},
          {"<<", qOpLShift},
          {">>", qOpRShift},
          {"<<<", qOpROTL},
          {">>>", qOpROTR},
          {"&&", qOpLogicAnd},
          {"||", qOpLogicOr},
          {"^^", qOpLogicXor},
          {"!", qOpLogicNot},
          {"<", qOpLT},
          {">", qOpGT},
          {"<=", qOpLE},
          {">=", qOpGE},
          {"==", qOpEq},
          {"!=", qOpNE},
          {"=", qOpSet},
          {"+=", qOpPlusSet},
          {"-=", qOpMinusSet},
          {"*=", qOpTimesSet},
          {"/=", qOpSlashSet},
          {"%=", qOpPercentSet},
          {"&=", qOpBitAndSet},
          {"|=", qOpBitOrSet},
          {"^=", qOpBitXorSet},
          {"&&=", qOpLogicAndSet},
          {"||=", qOpLogicOrSet},
          {"^^=", qOpLogicXorSet},
          {"<<=", qOpLShiftSet},
          {">>=", qOpRShiftSet},
          {"<<<=", qOpROTLSet},
          {">>>=", qOpROTRSet},
          {"++", qOpInc},
          {"--", qOpDec},
          {"as", qOpAs},
          {"bitcast_as", qOpBitcastAs},
          {"in", qOpIn},
          {"out", qOpOut},
          {"sizeof", qOpSizeof},
          {"bitsizeof", qOpBitsizeof},
          {"alignof", qOpAlignof},
          {"typeof", qOpTypeof},
          {".", qOpDot},
          {"..", qOpRange},
          {"...", qOpEllipsis},
          {"=>", qOpArrow},
          {"?", qOpTernary},
      });

  inline static const boost::bimap<Operator, OpConfig> LexicalOperatorsConfig =
      detail::make_bimap<Operator, OpConfig>({
          {qOpPlus, {OpType::Both, true}},
          {qOpMinus, {OpType::Both, true}},
          {qOpTimes, {OpType::Both, true}},
          {qOpSlash, {OpType::Binary, true}},
          {qOpPercent, {OpType::Binary, true}},
          {qOpBitAnd, {OpType::Both, true}},
          {qOpBitOr, {OpType::Binary, true}},
          {qOpBitXor, {OpType::Binary, true}},
          {qOpBitNot, {OpType::Unary, true}},
          {qOpLShift, {OpType::Binary, true}},
          {qOpRShift, {OpType::Binary, true}},
          {qOpROTL, {OpType::Binary, true}},
          {qOpROTR, {OpType::Binary, true}},
          {qOpLogicAnd, {OpType::Binary, true}},
          {qOpLogicOr, {OpType::Binary, true}},
          {qOpLogicXor, {OpType::Binary, true}},
          {qOpLogicNot, {OpType::Unary, true}},
          {qOpLT, {OpType::Binary, true}},
          {qOpGT, {OpType::Binary, true}},
          {qOpLE, {OpType::Binary, true}},
          {qOpGE, {OpType::Binary, true}},
          {qOpEq, {OpType::Binary, true}},
          {qOpNE, {OpType::Binary, true}},
          {qOpSet, {OpType::Binary, true}},
          {qOpPlusSet, {OpType::Binary, true}},
          {qOpMinusSet, {OpType::Binary, true}},
          {qOpTimesSet, {OpType::Binary, true}},
          {qOpSlashSet, {OpType::Binary, true}},
          {qOpPercentSet, {OpType::Binary, true}},
          {qOpBitAndSet, {OpType::Binary, true}},
          {qOpBitOrSet, {OpType::Binary, true}},
          {qOpBitXorSet, {OpType::Binary, true}},
          {qOpLogicAndSet, {OpType::Binary, true}},
          {qOpLogicOrSet, {OpType::Binary, true}},
          {qOpLogicXorSet, {OpType::Binary, true}},
          {qOpLShiftSet, {OpType::Binary, true}},
          {qOpRShiftSet, {OpType::Binary, true}},
          {qOpROTLSet, {OpType::Binary, true}},
          {qOpROTRSet, {OpType::Binary, true}},
          {qOpInc, {OpType::Unary, true}},
          {qOpDec, {OpType::Unary, true}},
          {qOpAs, {OpType::Binary, true}},
          {qOpBitcastAs, {OpType::Binary, false}},
          {qOpIn, {OpType::Both, false}},
          {qOpOut, {OpType::Both, false}},
          {qOpSizeof, {OpType::Unary, false}},
          {qOpBitsizeof, {OpType::Unary, false}},
          {qOpAlignof, {OpType::Unary, false}},
          {qOpTypeof, {OpType::Unary, false}},
          {qOpDot, {OpType::Binary, false}},
          {qOpRange, {OpType::Binary, true}},
          {qOpEllipsis, {OpType::Unary, false}},
          {qOpArrow, {OpType::Binary, false}},
          {qOpTernary, {OpType::Ternary, false}},
      });

  inline static const boost::bimap<std::string_view, Punctor> LexicalPunctors =
      detail::make_bimap<std::string_view, Punctor>({
          {"(", qPuncLPar},
          {")", qPuncRPar},
          {"[", qPuncLBrk},
          {"]", qPuncRBrk},
          {"{", qPuncLCur},
          {"}", qPuncRCur},
          {",", qPuncComa},
          {":", qPuncColn},
          {";", qPuncSemi},
      });

  short GetOperatorPrecedence(Operator op, OpMode type);
  OpAssoc GetOperatorAssociativity(Operator op, OpMode type);

  const char *qlex_ty_str(TokenType ty);

  class ISourceFile {
  public:
    virtual ~ISourceFile() = default;
  };

  struct ScannerEOF final {};

  class IScanner {
    friend class LocationID;

    static constexpr size_t TOKEN_BUFFER_SIZE = 256;

    std::deque<Token> m_ready;
    std::optional<Token> m_current, m_last;
    bool m_skip_comments = false, m_ebit = false;

    std::unordered_map<LocationID, Location> m_location_interned;
    LocationID::Counter m_location_id = 0;
    std::vector<std::pair<LocationID, Location>> m_location_interned_buffer;

    class StaticImpl;
    friend class StaticImpl;

  protected:
    inline LocationID InternLocation(Location loc) {
      m_location_interned_buffer.push_back({m_location_id, loc});
      return m_location_id++;
    }

    void SetFailBit() { m_ebit = true; }
    virtual Token GetNext() = 0;

  public:
    IScanner() { m_location_interned_buffer.reserve(0xffff); }
    virtual ~IScanner() = default;

    Token Next();
    Token Peek();
    void Undo();

    std::optional<Token> Current() { return m_current; }

    constexpr bool IsEof() const {
      return m_current.has_value() && m_current->is(qEofF);
    }

    constexpr bool HasError() const { return m_ebit; }

    Location Start(Token t);
    Location End(Token t);

    uint32_t StartLine(Token t);
    uint32_t StartColumn(Token t);
    uint32_t EndLine(Token t);
    uint32_t EndColumn(Token t);

    virtual void SkipCommentsState(bool skip) { m_skip_comments = skip; }
    bool GetSkipCommentsState() const { return m_skip_comments; }
  };

  class CPP_EXPORT Tokenizer final : public IScanner {
    static constexpr size_t GETC_BUFFER_SIZE = 256;
    uint32_t m_offset = 0, m_line = 0, m_column = 0;

    std::istream &m_file;
    std::deque<char> m_pushback;

    std::array<char, GETC_BUFFER_SIZE> m_getc_buffer;
    size_t m_getc_buffer_pos = GETC_BUFFER_SIZE;
    bool m_eof = false;

    std::shared_ptr<core::Environment> m_env;

    class StaticImpl;
    friend class StaticImpl;

    void reset_state();

    Token GetNext() override;

  public:
    Tokenizer(std::istream &source_file, std::shared_ptr<core::Environment> env)
        : IScanner(), m_file(source_file), m_env(env) {}
    virtual ~Tokenizer() override {}

    std::shared_ptr<core::Environment> GetEnvironment() const { return m_env; }
  };

  const char *op_repr(Operator op);
  const char *kw_repr(Keyword kw);
  const char *punct_repr(Punctor punct);

  std::ostream &operator<<(std::ostream &os, TokenType ty);
  std::ostream &operator<<(std::ostream &os, Token tok);

  inline std::ostream &operator<<(std::ostream &os, Operator op) {
    os << op_repr(op);
    return os;
  }

  inline std::ostream &operator<<(std::ostream &os, Keyword kw) {
    os << kw_repr(kw);
    return os;
  }

  inline std::ostream &operator<<(std::ostream &os, Punctor punct) {
    os << punct_repr(punct);
    return os;
  }
}  // namespace ncc::lex

#endif