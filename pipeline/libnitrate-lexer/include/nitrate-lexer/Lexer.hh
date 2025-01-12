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
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Token.hh>
#include <ostream>
#include <queue>

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
          {"scope", Scope},     {"import", Import}, {"pub", Pub},
          {"sec", Sec},         {"pro", Pro},       {"type", Type},
          {"let", Let},         {"var", Var},       {"const", Const},
          {"static", Static},   {"struct", Struct}, {"region", Region},
          {"group", Group},     {"class", Class},   {"union", Union},
          {"opaque", Opaque},   {"enum", Enum},     {"__fstring", __FString},
          {"fn", Fn},           {"unsafe", Unsafe}, {"safe", Safe},
          {"promise", Promise}, {"if", If},         {"else", Else},
          {"for", For},         {"while", While},   {"do", Do},
          {"switch", Switch},   {"break", Break},   {"continue", Continue},
          {"ret", Return},      {"retif", Retif},   {"foreach", Foreach},
          {"try", Try},         {"catch", Catch},   {"throw", Throw},
          {"async", Async},     {"await", Await},   {"__asm__", __Asm__},
          {"undef", Undef},     {"null", Null},     {"true", True},
          {"false", False},
      });

  inline static const boost::bimap<std::string_view, Operator>
      LexicalOperators = detail::make_bimap<std::string_view, Operator>({
          {"+", OpPlus},
          {"-", OpMinus},
          {"*", OpTimes},
          {"/", OpSlash},
          {"%", OpPercent},
          {"&", OpBitAnd},
          {"|", OpBitOr},
          {"^", OpBitXor},
          {"~", OpBitNot},
          {"<<", OpLShift},
          {">>", OpRShift},
          {"<<<", OpROTL},
          {">>>", OpROTR},
          {"&&", OpLogicAnd},
          {"||", OpLogicOr},
          {"^^", OpLogicXor},
          {"!", OpLogicNot},
          {"<", OpLT},
          {">", OpGT},
          {"<=", OpLE},
          {">=", OpGE},
          {"==", OpEq},
          {"!=", OpNE},
          {"=", OpSet},
          {"+=", OpPlusSet},
          {"-=", OpMinusSet},
          {"*=", OpTimesSet},
          {"/=", OpSlashSet},
          {"%=", OpPercentSet},
          {"&=", OpBitAndSet},
          {"|=", OpBitOrSet},
          {"^=", OpBitXorSet},
          {"&&=", OpLogicAndSet},
          {"||=", OpLogicOrSet},
          {"^^=", OpLogicXorSet},
          {"<<=", OpLShiftSet},
          {">>=", OpRShiftSet},
          {"<<<=", OpROTLSet},
          {">>>=", OpROTRSet},
          {"++", OpInc},
          {"--", OpDec},
          {"as", OpAs},
          {"bitcast_as", OpBitcastAs},
          {"in", OpIn},
          {"out", OpOut},
          {"sizeof", OpSizeof},
          {"bitsizeof", OpBitsizeof},
          {"alignof", OpAlignof},
          {"typeof", OpTypeof},
          {".", OpDot},
          {"..", OpRange},
          {"...", OpEllipsis},
          {"=>", OpArrow},
          {"?", OpTernary},
      });

  inline static const boost::bimap<Operator, OpConfig> LexicalOperatorsConfig =
      detail::make_bimap<Operator, OpConfig>({
          {OpPlus, {OpType::Both, true}},
          {OpMinus, {OpType::Both, true}},
          {OpTimes, {OpType::Both, true}},
          {OpSlash, {OpType::Binary, true}},
          {OpPercent, {OpType::Binary, true}},
          {OpBitAnd, {OpType::Both, true}},
          {OpBitOr, {OpType::Binary, true}},
          {OpBitXor, {OpType::Binary, true}},
          {OpBitNot, {OpType::Unary, true}},
          {OpLShift, {OpType::Binary, true}},
          {OpRShift, {OpType::Binary, true}},
          {OpROTL, {OpType::Binary, true}},
          {OpROTR, {OpType::Binary, true}},
          {OpLogicAnd, {OpType::Binary, true}},
          {OpLogicOr, {OpType::Binary, true}},
          {OpLogicXor, {OpType::Binary, true}},
          {OpLogicNot, {OpType::Unary, true}},
          {OpLT, {OpType::Binary, true}},
          {OpGT, {OpType::Binary, true}},
          {OpLE, {OpType::Binary, true}},
          {OpGE, {OpType::Binary, true}},
          {OpEq, {OpType::Binary, true}},
          {OpNE, {OpType::Binary, true}},
          {OpSet, {OpType::Binary, true}},
          {OpPlusSet, {OpType::Binary, true}},
          {OpMinusSet, {OpType::Binary, true}},
          {OpTimesSet, {OpType::Binary, true}},
          {OpSlashSet, {OpType::Binary, true}},
          {OpPercentSet, {OpType::Binary, true}},
          {OpBitAndSet, {OpType::Binary, true}},
          {OpBitOrSet, {OpType::Binary, true}},
          {OpBitXorSet, {OpType::Binary, true}},
          {OpLogicAndSet, {OpType::Binary, true}},
          {OpLogicOrSet, {OpType::Binary, true}},
          {OpLogicXorSet, {OpType::Binary, true}},
          {OpLShiftSet, {OpType::Binary, true}},
          {OpRShiftSet, {OpType::Binary, true}},
          {OpROTLSet, {OpType::Binary, true}},
          {OpROTRSet, {OpType::Binary, true}},
          {OpInc, {OpType::Unary, true}},
          {OpDec, {OpType::Unary, true}},
          {OpAs, {OpType::Binary, true}},
          {OpBitcastAs, {OpType::Binary, false}},
          {OpIn, {OpType::Both, false}},
          {OpOut, {OpType::Both, false}},
          {OpSizeof, {OpType::Unary, false}},
          {OpBitsizeof, {OpType::Unary, false}},
          {OpAlignof, {OpType::Unary, false}},
          {OpTypeof, {OpType::Unary, false}},
          {OpDot, {OpType::Binary, false}},
          {OpRange, {OpType::Binary, true}},
          {OpEllipsis, {OpType::Unary, false}},
          {OpArrow, {OpType::Binary, false}},
          {OpTernary, {OpType::Ternary, false}},
      });

  inline static const boost::bimap<std::string_view, Punctor> LexicalPunctors =
      detail::make_bimap<std::string_view, Punctor>({
          {"(", PuncLPar},
          {")", PuncRPar},
          {"[", PuncLBrk},
          {"]", PuncRBrk},
          {"{", PuncLCur},
          {"}", PuncRCur},
          {",", PuncComa},
          {":", PuncColn},
          {";", PuncSemi},
      });

  short GetOperatorPrecedence(Operator op, OpMode type);
  OpAssoc GetOperatorAssociativity(Operator op, OpMode type);

  const char *qlex_ty_str(TokenType ty);

  class ISourceFile {
  public:
    virtual ~ISourceFile() = default;
  };

  struct ScannerEOF final {};

  class CPP_EXPORT IScanner {
    static constexpr size_t TOKEN_BUFFER_SIZE = 256;

    std::deque<Token> m_ready;
    std::optional<Token> m_last;
    Token m_current;
    bool m_skip_comments = false, m_ebit = false, m_eof = false;
    string m_current_filename;

    // 0 is reserved for invalid location
    LocationID::Counter m_location_id = 1;
    std::vector<Location> m_location_interned;

    class StaticImpl;
    friend class StaticImpl;

    Location GetEofLocation();

  protected:
    std::shared_ptr<Environment> m_env;

    inline LocationID InternLocation(Location loc) {
      m_location_interned.push_back(loc);
      return m_location_id++;
    }

    void SetFailBit() { m_ebit = true; }
    virtual Token GetNext() = 0;

    /**
     * @brief Provide fallback resolution for location IDs.
     * @note The internal map is checked first, it the ID is not found, this
     *       method is called.
     */
    virtual std::optional<Location> GetLocationFallback(LocationID) {
      return std::nullopt;
    };

  public:
    IScanner(std::shared_ptr<Environment> env) : m_env(env) {
      m_location_interned.reserve(0xffff);
    }
    virtual ~IScanner() = default;

    Token Next();
    Token Peek();
    void Undo();

    constexpr Token Current() { return m_current; }
    constexpr bool IsEof() const { return m_eof; }
    constexpr bool HasError() const { return m_ebit; }

    Location Start(Token t);
    Location End(Token t);

    uint32_t StartLine(Token t);
    uint32_t StartColumn(Token t);
    uint32_t EndLine(Token t);
    uint32_t EndColumn(Token t);

    virtual void SkipCommentsState(bool skip) { m_skip_comments = skip; }
    bool GetSkipCommentsState() const { return m_skip_comments; }

    void SetCurrentFilename(string filename) { m_current_filename = filename; }
    string GetCurrentFilename() const { return m_current_filename; }

    Location GetLocation(LocationID id);

    struct Point {
      long x = 0, y = 0;
    };

    virtual std::optional<std::vector<std::string>> GetSourceWindow(
        Point start, Point end, char fillchar = ' ') {
      (void)start;
      (void)end;
      (void)fillchar;
      return std::nullopt;
    }

    std::shared_ptr<ncc::Environment> GetEnvironment() const { return m_env; }
  };

  class CPP_EXPORT Tokenizer final : public IScanner {
    static constexpr size_t GETC_BUFFER_SIZE = 256;
    uint32_t m_offset = 0, m_line = 0, m_column = 0;

    std::istream &m_file;
    std::queue<char> m_fifo;

    std::array<char, GETC_BUFFER_SIZE> m_getc_buffer;
    size_t m_getc_buffer_pos = GETC_BUFFER_SIZE;
    bool m_eof = false;

    class StaticImpl;
    friend class StaticImpl;

    void reset_state();

    Token GetNext() override;

  public:
    Tokenizer(std::istream &source_file, std::shared_ptr<Environment> env)
        : IScanner(env), m_file(source_file) {
      if (auto filename = env->get("FILE"); filename.has_value()) {
        SetCurrentFilename(filename.value());
      }
    }
    virtual ~Tokenizer() override {}

    virtual std::optional<std::vector<std::string>> GetSourceWindow(
        Point start, Point end, char fillchar = ' ') override;
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