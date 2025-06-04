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

#define PLACEHOLDER_IMPL(name, type) \
  class name : public Expr {         \
  public:                            \
    name() : Expr(type) {}           \
  };

  PLACEHOLDER_IMPL(BinExpr, ASTKind::gBinExpr);
  PLACEHOLDER_IMPL(UnaryExpr, ASTKind::gUnaryExpr);
  PLACEHOLDER_IMPL(Number, ASTKind::gNumber);
  PLACEHOLDER_IMPL(FString, ASTKind::gFString);
  PLACEHOLDER_IMPL(String, ASTKind::gString);
  PLACEHOLDER_IMPL(Char, ASTKind::gChar);
  PLACEHOLDER_IMPL(List, ASTKind::gList);
  PLACEHOLDER_IMPL(Ident, ASTKind::gIdent);
  PLACEHOLDER_IMPL(Index, ASTKind::gIndex);
  PLACEHOLDER_IMPL(Slice, ASTKind::gSlice);
  PLACEHOLDER_IMPL(Call, ASTKind::gCall);
  PLACEHOLDER_IMPL(TemplateCall, ASTKind::gTemplateCall);
  PLACEHOLDER_IMPL(If, ASTKind::gIf);
  PLACEHOLDER_IMPL(Else, ASTKind::gElse);
  PLACEHOLDER_IMPL(For, ASTKind::gFor);
  PLACEHOLDER_IMPL(While, ASTKind::gWhile);
  PLACEHOLDER_IMPL(Do, ASTKind::gDo);
  PLACEHOLDER_IMPL(Switch, ASTKind::gSwitch);
  PLACEHOLDER_IMPL(Break, ASTKind::gBreak);
  PLACEHOLDER_IMPL(Continue, ASTKind::gContinue);
  PLACEHOLDER_IMPL(Return, ASTKind::gReturn);
  PLACEHOLDER_IMPL(Foreach, ASTKind::gForeach);
  PLACEHOLDER_IMPL(Try, ASTKind::gTry);
  PLACEHOLDER_IMPL(Catch, ASTKind::gCatch);
  PLACEHOLDER_IMPL(Throw, ASTKind::gThrow);
  PLACEHOLDER_IMPL(Await, ASTKind::gAwait);
  PLACEHOLDER_IMPL(Asm, ASTKind::gAsm);

  PLACEHOLDER_IMPL(InferTy, ASTKind::tInfer);
  PLACEHOLDER_IMPL(OpaqueTy, ASTKind::tOpaque);
  PLACEHOLDER_IMPL(NamedTy, ASTKind::tNamed);
  PLACEHOLDER_IMPL(RefTy, ASTKind::tRef);
  PLACEHOLDER_IMPL(PtrTy, ASTKind::tPtr);
  PLACEHOLDER_IMPL(ArrayTy, ASTKind::tArray);
  PLACEHOLDER_IMPL(TupleTy, ASTKind::tTuple);
  PLACEHOLDER_IMPL(TemplateTy, ASTKind::tTemplate);
  PLACEHOLDER_IMPL(LambdaTy, ASTKind::tLambda);

  PLACEHOLDER_IMPL(Let, ASTKind::sLet);
  PLACEHOLDER_IMPL(Var, ASTKind::sVar);
  PLACEHOLDER_IMPL(Fn, ASTKind::sFn);
  PLACEHOLDER_IMPL(Enum, ASTKind::sEnum);
  PLACEHOLDER_IMPL(Struct, ASTKind::sStruct);
  PLACEHOLDER_IMPL(Union, ASTKind::sUnion);
  PLACEHOLDER_IMPL(Contract, ASTKind::sContract);
  PLACEHOLDER_IMPL(Trait, ASTKind::sTrait);
  PLACEHOLDER_IMPL(TypeDef, ASTKind::sTypeDef);
  PLACEHOLDER_IMPL(Scope, ASTKind::sScope);
  PLACEHOLDER_IMPL(Import, ASTKind::sImport);
  PLACEHOLDER_IMPL(UnitTest, ASTKind::sUnitTest);

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

  W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(Visitor, ) W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(ConstVisitor, const)

#undef W_NITRATE_PARSER_EXPR_ACCEPT_METHOD
}  // namespace nitrate::compiler::parser
