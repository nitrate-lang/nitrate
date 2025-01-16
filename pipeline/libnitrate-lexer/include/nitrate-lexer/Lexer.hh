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

namespace ncc::lex {
  namespace detail {
    template <typename L, typename R>
    auto MakeBimap(
        std::initializer_list<typename boost::bimap<L, R>::value_type> list) -> boost::bimap<L, R> {
      return boost::bimap<L, R>(list.begin(), list.end());
    }

    struct ScannerEOF final {};

    enum OpType : uint8_t { Both, Unary, Binary, Ternary };

    struct OpConfig {
      OpType m_type;
      bool m_overloadable;

      OpConfig(OpType t, bool o) : m_type(t), m_overloadable(o) {}

      auto operator<(const OpConfig &rhs) const -> bool {
        if (m_type != rhs.m_type) {
          return m_type < rhs.m_type;
        }

        return (int)m_overloadable < (int)rhs.m_overloadable;
      }
    };
  }  // namespace detail

  enum class OpMode : uint8_t { Binary, PreUnary, PostUnary, Ternary };
  enum Associativity : uint8_t { Left, Right };

  inline static const auto LEXICAL_KEYWORDS =
      detail::MakeBimap<std::string, Keyword>({
          {"scope", Scope},
          {"pub", Pub},
          {"sec", Sec},
          {"pro", Pro},
          {"type", Type},
          {"let", Let},
          {"var", Var},
          {"const", Const},
          {"static", Static},
          {"struct", Struct},
          {"region", Region},
          {"group", Group},
          {"class", Class},
          {"union", Union},
          {"opaque", Opaque},
          {"enum", Enum},
          {"__fstring", __FString},
          {"fn", Fn},
          {"unsafe", Unsafe},
          {"safe", Safe},
          {"promise", Promise},
          {"if", If},
          {"else", Else},
          {"for", For},
          {"while", While},
          {"do", Do},
          {"switch", Switch},
          {"break", Break},
          {"continue", Continue},
          {"ret", Return},
          {"retif", Retif},
          {"foreach", Foreach},
          {"try", Try},
          {"catch", Catch},
          {"throw", Throw},
          {"async", Async},
          {"await", Await},
          {"__asm__", __Asm__},
          {"undef", Undef},
          {"null", Null},
          {"true", True},
          {"false", False},
      });

  inline static const auto LEXICAL_OPERATORS =
      detail::MakeBimap<std::string, Operator>({
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
          {"comptime", OpComptime},
          {".", OpDot},
          {"..", OpRange},
          {"...", OpEllipsis},
          {"=>", OpArrow},
          {"?", OpTernary},
      });

  inline static const auto LEXICAL_OPERATORS_CONFIG =
      detail::MakeBimap<Operator, detail::OpConfig>({
          {OpPlus, {detail::Both, true}},
          {OpMinus, {detail::Both, true}},
          {OpTimes, {detail::Both, true}},
          {OpSlash, {detail::Binary, true}},
          {OpPercent, {detail::Binary, true}},
          {OpBitAnd, {detail::Both, true}},
          {OpBitOr, {detail::Binary, true}},
          {OpBitXor, {detail::Binary, true}},
          {OpBitNot, {detail::Unary, true}},
          {OpLShift, {detail::Binary, true}},
          {OpRShift, {detail::Binary, true}},
          {OpROTL, {detail::Binary, true}},
          {OpROTR, {detail::Binary, true}},
          {OpLogicAnd, {detail::Binary, true}},
          {OpLogicOr, {detail::Binary, true}},
          {OpLogicXor, {detail::Binary, true}},
          {OpLogicNot, {detail::Unary, true}},
          {OpLT, {detail::Binary, true}},
          {OpGT, {detail::Binary, true}},
          {OpLE, {detail::Binary, true}},
          {OpGE, {detail::Binary, true}},
          {OpEq, {detail::Binary, true}},
          {OpNE, {detail::Binary, true}},
          {OpSet, {detail::Binary, true}},
          {OpPlusSet, {detail::Binary, true}},
          {OpMinusSet, {detail::Binary, true}},
          {OpTimesSet, {detail::Binary, true}},
          {OpSlashSet, {detail::Binary, true}},
          {OpPercentSet, {detail::Binary, true}},
          {OpBitAndSet, {detail::Binary, true}},
          {OpBitOrSet, {detail::Binary, true}},
          {OpBitXorSet, {detail::Binary, true}},
          {OpLogicAndSet, {detail::Binary, true}},
          {OpLogicOrSet, {detail::Binary, true}},
          {OpLogicXorSet, {detail::Binary, true}},
          {OpLShiftSet, {detail::Binary, true}},
          {OpRShiftSet, {detail::Binary, true}},
          {OpROTLSet, {detail::Binary, true}},
          {OpROTRSet, {detail::Binary, true}},
          {OpInc, {detail::Unary, true}},
          {OpDec, {detail::Unary, true}},
          {OpAs, {detail::Binary, true}},
          {OpBitcastAs, {detail::Binary, false}},
          {OpIn, {detail::Both, false}},
          {OpOut, {detail::Both, false}},
          {OpSizeof, {detail::Unary, false}},
          {OpBitsizeof, {detail::Unary, false}},
          {OpAlignof, {detail::Unary, false}},
          {OpTypeof, {detail::Unary, false}},
          {OpComptime, {detail::Unary, false}},
          {OpDot, {detail::Binary, false}},
          {OpRange, {detail::Binary, true}},
          {OpEllipsis, {detail::Unary, false}},
          {OpArrow, {detail::Binary, false}},
          {OpTernary, {detail::Ternary, false}},
      });

  inline static const auto LEXICAL_PUNCTORS =
      detail::MakeBimap<std::string, Punctor>({
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

  auto GetOperatorPrecedence(Operator op, OpMode type) -> short;
  auto GetOperatorAssociativity(Operator op, OpMode type) -> Associativity;
  const char *qlex_ty_str(TokenType ty);  /// NOLINT

  class ISourceFile {
  public:
    virtual ~ISourceFile() = default;
  };

  class NCC_EXPORT IScanner {
    static constexpr size_t kTokenBufferSize = 256;

    std::deque<Token> m_ready;
    std::vector<Token> m_comments;
    std::vector<Location> m_location_interned;
    Token m_current;
    string m_filename;
    bool m_skip = false, m_ebit = false, m_eof = false;

    class StaticImpl;
    friend class StaticImpl;

    auto GetEofLocation() -> Location;

  protected:
    std::shared_ptr<Environment> m_env;

    virtual auto GetNext() -> Token = 0;

    /**
     * @brief Provide fallback resolution for location IDs.
     * @note The internal map is checked first, it the ID is not found, this
     *       method is called.
     */
    virtual auto GetLocationFallback(LocationID) -> std::optional<Location> {
      return std::nullopt;
    };

  public:
    IScanner(std::shared_ptr<Environment> env) : m_env(std::move(env)) {
      constexpr size_t kInitialLocationReserve = 0xffff;
      m_location_interned.reserve(kInitialLocationReserve);
      m_location_interned.emplace_back();
    }

    virtual ~IScanner() = default;

    auto Next() -> Token;
    auto Peek() -> Token;
    void Insert(Token tok);

    constexpr auto Current() { return m_current; }
    [[nodiscard]] constexpr auto IsEof() const { return m_eof; }
    [[nodiscard]] constexpr auto HasError() const { return m_ebit; }
    void SetFailBit() { m_ebit = true; }

    auto Start(Token t) -> Location;
    auto End(Token t) -> Location;

    auto StartLine(Token t) -> uint32_t;
    auto StartColumn(Token t) -> uint32_t;
    auto EndLine(Token t) -> uint32_t;
    auto EndColumn(Token t) -> uint32_t;

    auto InternLocation(Location loc) {
      m_location_interned.push_back(loc);
      return LocationID(m_location_interned.size() - 1);
    }

    constexpr void SkipCommentsState(bool skip) { m_skip = skip; }
    [[nodiscard]] constexpr auto GetSkipCommentsState() const -> bool { return m_skip; }

    constexpr void SetCurrentFilename(auto filename) { m_filename = filename; }
    [[nodiscard]] constexpr auto GetCurrentFilename() const {
      return m_filename;
    }

    auto GetLocation(LocationID id) -> Location;

    struct Point {
      long x = 0, y = 0;  /// NOLINT
    };

    virtual auto GetSourceWindow(
        Point start, Point end, char fillchar = ' ') -> std::optional<std::vector<std::string>> {
      (void)start;
      (void)end;
      (void)fillchar;
      return std::nullopt;
    }

    [[nodiscard]] auto GetEnvironment() const { return m_env; }

    auto CommentBuffer() -> const std::vector<Token> & { return m_comments; }
    void ClearCommentBuffer() { m_comments.clear(); }
  };

  class NCC_EXPORT Tokenizer final : public IScanner {
    static constexpr size_t kGetcBufferSize = 1024;

    uint64_t m_offset = 0, m_line = 0, m_column = 0;
    std::queue<char> m_fifo;
    size_t m_getc_buffer_pos = kGetcBufferSize;
    std::array<char, kGetcBufferSize> m_getc_buffer;
    std::istream &m_file;
    bool m_eof = false;

    void RefillCharacterBuffer();
    auto GetChar() -> char;

    class StaticImpl;
    friend class StaticImpl;

    void ResetAutomaton();

    auto GetNext() -> Token override;

  public:
    Tokenizer(std::istream &source_file, std::shared_ptr<Environment> env)
        : IScanner(std::move(env)), m_file(source_file) {
      if (auto filename = m_env->Get("FILE"); filename.has_value()) {
        SetCurrentFilename(filename.value());
      }
    }

    ~Tokenizer() override = default;

    auto GetSourceWindow(
        Point start, Point end, char fillchar = ' ') -> std::optional<std::vector<std::string>> override;
  };

  static inline const char *op_repr(Operator op) {  /// NOLINT
    return LEXICAL_OPERATORS.right.at(op).c_str();
  }

  static inline const char *kw_repr(Keyword kw) {  /// NOLINT
    return LEXICAL_KEYWORDS.right.at(kw).c_str();
  }

  static inline const char *punct_repr(Punctor punct) {  /// NOLINT
    return LEXICAL_PUNCTORS.right.at(punct).c_str();
  }

  auto operator<<(std::ostream &os, TokenType ty) -> std::ostream &;
  auto operator<<(std::ostream &os, Token tok) -> std::ostream &;

  inline auto operator<<(std::ostream &os, Operator op) -> std::ostream & {
    os << op_repr(op);
    return os;
  }

  inline auto operator<<(std::ostream &os, Keyword kw) -> std::ostream & {
    os << kw_repr(kw);
    return os;
  }

  inline auto operator<<(std::ostream &os, Punctor punct) -> std::ostream & {
    os << punct_repr(punct);
    return os;
  }
}  // namespace ncc::lex

#endif