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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/flyweight.hpp>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ParseTreeFwd.hh>
#include <nitrate-parser/Visitor.hh>
#include <ostream>

namespace nitrate::compiler::parser {
  class Expr {
    static constexpr size_t M_KIND_BITS = 6;  // 64 kinds max
    static_assert(static_cast<size_t>(ASTKIND_MAX) <= (1 << M_KIND_BITS) - 1,
                  "ASTKind must fit in 6 bits for Expr::m_kind");

    ASTKind m_kind : M_KIND_BITS;
    bool m_is_parenthesized : 1 = false;
    boost::flyweight<lexer::FileSourceRange> m_source_range;

  public:
    Expr(ASTKind kind) : m_kind(kind) {}

    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;
    auto operator=(const Expr&) -> Expr& = delete;
    auto operator=(Expr&&) -> Expr& = delete;
    ~Expr();

    [[nodiscard]] constexpr auto get_kind() const -> ASTKind { return m_kind; }
    [[nodiscard]] constexpr auto is_discarded() const -> bool { return m_kind == ASTKind::Discarded; }
    constexpr auto discard() -> void { m_kind = ASTKind::Discarded; }

    [[nodiscard]] constexpr auto is_parenthesized() const -> bool { return m_is_parenthesized; }
    constexpr auto set_parenthesized(bool is_parenthesized) -> void { m_is_parenthesized = is_parenthesized; }

    [[nodiscard]] constexpr auto source_range() const -> const lexer::FileSourceRange& { return m_source_range.get(); }

    /** Perform minimal required semantic analysis */
    [[nodiscard]] auto check(const SymbolTable& symbol_table) const -> bool;

    constexpr auto accept(ConstVisitor& visitor) const -> void;
    constexpr auto accept(Visitor& visitor) -> void;

    auto dump(std::ostream& os) const -> std::ostream&;
  };

#define W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(visitor_name, constness)     \
  constexpr auto Expr::accept(visitor_name& visitor) constness -> void { \
    constness Expr& node = *this;                                        \
                                                                         \
    switch (get_kind()) {                                                \
      case ASTKind::Discarded:                                           \
        visitor.visit(static_cast<constness Expr&>(node));               \
        break;                                                           \
      case ASTKind::gBinExpr:                                            \
        visitor.visit(static_cast<constness BinExpr&>(node));            \
        break;                                                           \
      case ASTKind::gUnaryExpr:                                          \
        visitor.visit(static_cast<constness UnaryExpr&>(node));          \
        break;                                                           \
      case ASTKind::gNumber:                                             \
        visitor.visit(static_cast<constness Number&>(node));             \
        break;                                                           \
      case ASTKind::gFString:                                            \
        visitor.visit(static_cast<constness FString&>(node));            \
        break;                                                           \
      case ASTKind::gString:                                             \
        visitor.visit(static_cast<constness String&>(node));             \
        break;                                                           \
      case ASTKind::gChar:                                               \
        visitor.visit(static_cast<constness Char&>(node));               \
        break;                                                           \
      case ASTKind::gList:                                               \
        visitor.visit(static_cast<constness List&>(node));               \
        break;                                                           \
      case ASTKind::gIdent:                                              \
        visitor.visit(static_cast<constness Ident&>(node));              \
        break;                                                           \
      case ASTKind::gIndex:                                              \
        visitor.visit(static_cast<constness Index&>(node));              \
        break;                                                           \
      case ASTKind::gSlice:                                              \
        visitor.visit(static_cast<constness Slice&>(node));              \
        break;                                                           \
      case ASTKind::gCall:                                               \
        visitor.visit(static_cast<constness Call&>(node));               \
        break;                                                           \
      case ASTKind::gTemplateCall:                                       \
        visitor.visit(static_cast<constness TemplateCall&>(node));       \
        break;                                                           \
      case ASTKind::gIf:                                                 \
        visitor.visit(static_cast<constness If&>(node));                 \
        break;                                                           \
      case ASTKind::gElse:                                               \
        visitor.visit(static_cast<constness Else&>(node));               \
        break;                                                           \
      case ASTKind::gFor:                                                \
        visitor.visit(static_cast<constness For&>(node));                \
        break;                                                           \
      case ASTKind::gWhile:                                              \
        visitor.visit(static_cast<constness While&>(node));              \
        break;                                                           \
      case ASTKind::gDo:                                                 \
        visitor.visit(static_cast<constness Do&>(node));                 \
        break;                                                           \
      case ASTKind::gSwitch:                                             \
        visitor.visit(static_cast<constness Switch&>(node));             \
        break;                                                           \
      case ASTKind::gBreak:                                              \
        visitor.visit(static_cast<constness Break&>(node));              \
        break;                                                           \
      case ASTKind::gContinue:                                           \
        visitor.visit(static_cast<constness Continue&>(node));           \
        break;                                                           \
      case ASTKind::gReturn:                                             \
        visitor.visit(static_cast<constness Return&>(node));             \
        break;                                                           \
      case ASTKind::gForeach:                                            \
        visitor.visit(static_cast<constness Foreach&>(node));            \
        break;                                                           \
      case ASTKind::gTry:                                                \
        visitor.visit(static_cast<constness Try&>(node));                \
        break;                                                           \
      case ASTKind::gCatch:                                              \
        visitor.visit(static_cast<constness Catch&>(node));              \
        break;                                                           \
      case ASTKind::gThrow:                                              \
        visitor.visit(static_cast<constness Throw&>(node));              \
        break;                                                           \
      case ASTKind::gAwait:                                              \
        visitor.visit(static_cast<constness Await&>(node));              \
        break;                                                           \
      case ASTKind::gAsm:                                                \
        visitor.visit(static_cast<constness Asm&>(node));                \
        break;                                                           \
                                                                         \
      case ASTKind::tInfer:                                              \
        visitor.visit(static_cast<constness InferTy&>(node));            \
        break;                                                           \
      case ASTKind::tOpaque:                                             \
        visitor.visit(static_cast<constness OpaqueTy&>(node));           \
        break;                                                           \
      case ASTKind::tNamed:                                              \
        visitor.visit(static_cast<constness NamedTy&>(node));            \
        break;                                                           \
      case ASTKind::tRef:                                                \
        visitor.visit(static_cast<constness RefTy&>(node));              \
        break;                                                           \
      case ASTKind::tPtr:                                                \
        visitor.visit(static_cast<constness PtrTy&>(node));              \
        break;                                                           \
      case ASTKind::tArray:                                              \
        visitor.visit(static_cast<constness ArrayTy&>(node));            \
        break;                                                           \
      case ASTKind::tTuple:                                              \
        visitor.visit(static_cast<constness TupleTy&>(node));            \
        break;                                                           \
      case ASTKind::tTemplate:                                           \
        visitor.visit(static_cast<constness TemplateTy&>(node));         \
        break;                                                           \
      case ASTKind::tLambda:                                             \
        visitor.visit(static_cast<constness LambdaTy&>(node));           \
        break;                                                           \
                                                                         \
      case ASTKind::sLet:                                                \
        visitor.visit(static_cast<constness Let&>(node));                \
        break;                                                           \
      case ASTKind::sVar:                                                \
        visitor.visit(static_cast<constness Var&>(node));                \
        break;                                                           \
      case ASTKind::sFn:                                                 \
        visitor.visit(static_cast<constness Fn&>(node));                 \
        break;                                                           \
      case ASTKind::sEnum:                                               \
        visitor.visit(static_cast<constness Enum&>(node));               \
        break;                                                           \
      case ASTKind::sStruct:                                             \
        visitor.visit(static_cast<constness Struct&>(node));             \
        break;                                                           \
      case ASTKind::sUnion:                                              \
        visitor.visit(static_cast<constness Union&>(node));              \
        break;                                                           \
      case ASTKind::sContract:                                           \
        visitor.visit(static_cast<constness Contract&>(node));           \
        break;                                                           \
      case ASTKind::sTrait:                                              \
        visitor.visit(static_cast<constness Trait&>(node));              \
        break;                                                           \
      case ASTKind::sTypeDef:                                            \
        visitor.visit(static_cast<constness TypeDef&>(node));            \
        break;                                                           \
      case ASTKind::sScope:                                              \
        visitor.visit(static_cast<constness Scope&>(node));              \
        break;                                                           \
      case ASTKind::sImport:                                             \
        visitor.visit(static_cast<constness Import&>(node));             \
        break;                                                           \
      case ASTKind::sUnitTest:                                           \
        visitor.visit(static_cast<constness UnitTest&>(node));           \
        break;                                                           \
    }                                                                    \
  }

  W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(Visitor, )
  W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(ConstVisitor, const)

#undef W_NITRATE_PARSER_EXPR_ACCEPT_METHOD
}  // namespace nitrate::compiler::parser
