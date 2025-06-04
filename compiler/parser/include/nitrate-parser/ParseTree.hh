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
#include <iostream>
#include <memory>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ParseTreeFwd.hh>
#include <nitrate-parser/Visitor.hh>

namespace nitrate::compiler::parser {
  class Expr {
    static constexpr size_t M_KIND_BITS = 6;  // 64 kinds max
    static_assert(static_cast<size_t>(ASTKIND_MAX) <= (1 << M_KIND_BITS) - 1,
                  "ASTKind must fit in 6 bits for Expr::m_kind");

    class SourceLocationTag {
      static std::unordered_map<uint64_t, boost::flyweight<lexer::FileSourceRange>> SOURCE_RANGES_GLOBAL;
      static uint64_t SOURCE_RANGES_ID_CTR_GLOBAL;
      static std::mutex SOURCE_RANGES_LOCK_GLOBAL;

      uint64_t m_id : 56;

    public:
      SourceLocationTag() : m_id(0) {}
      SourceLocationTag(boost::flyweight<lexer::FileSourceRange> source_range) {
        std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
        m_id = ++SOURCE_RANGES_ID_CTR_GLOBAL;
        SOURCE_RANGES_GLOBAL.emplace(static_cast<uint64_t>(m_id), std::move(source_range));
      }

      SourceLocationTag(const SourceLocationTag&) = delete;
      SourceLocationTag(SourceLocationTag&& o) : m_id(o.m_id) { o.m_id = 0; }
      auto operator=(const SourceLocationTag&) -> SourceLocationTag& = delete;
      auto operator=(SourceLocationTag&& o) -> SourceLocationTag& = default;

      ~SourceLocationTag() {
        if (m_id != 0) {
          std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
          SOURCE_RANGES_GLOBAL.erase(m_id);
        }
      }

      [[nodiscard]] auto get() const -> const lexer::FileSourceRange& {
        std::lock_guard lock(SOURCE_RANGES_LOCK_GLOBAL);
        return SOURCE_RANGES_GLOBAL.at(m_id).get();
      }
    } __attribute__((packed));

    ASTKind m_kind : M_KIND_BITS;
    bool m_is_discarded : 1 = false;
    bool m_is_parenthesized : 1 = false;
    SourceLocationTag m_source_range;

  public:
    Expr(ASTKind kind) : m_kind(kind) {}

    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;
    auto operator=(const Expr&) -> Expr& = delete;
    auto operator=(Expr&&) -> Expr& = delete;
    virtual ~Expr();

    [[nodiscard]] constexpr auto get_kind() const -> ASTKind { return m_kind; }
    [[nodiscard]] constexpr auto is_discarded() const -> bool { return m_is_discarded; }
    constexpr auto discard() -> void { m_is_discarded = true; }

    [[nodiscard]] constexpr auto is_parenthesized() const -> bool { return m_is_parenthesized; }
    constexpr auto set_parenthesized(bool b) -> void { m_is_parenthesized = b; }

    [[nodiscard]] constexpr auto source_range() const -> const lexer::FileSourceRange& { return m_source_range.get(); }
    auto set_source_range(lexer::FileSourceRange source_range) -> void {
      m_source_range = SourceLocationTag(boost::flyweight<lexer::FileSourceRange>(std::move(source_range)));
    }

    /** Perform minimal required semantic analysis */
    [[nodiscard]] auto check(const SymbolTable& symbol_table) const -> bool;

    constexpr auto accept(ConstVisitor& visitor) const -> void;
    constexpr auto accept(Visitor& visitor) -> void;

    auto dump(std::ostream& os = std::cout) const -> std::ostream&;
  } __attribute__((packed));

#define PLACEHOLDER_IMPL(name, type) \
  class name : public Expr {         \
  public:                            \
    name() : Expr(type) {}           \
  };

  class BinExpr : public Expr {
    using BinOp = lexer::Operator;

    std::unique_ptr<Expr> m_lhs, m_rhs;
    BinOp m_op;

  public:
    BinExpr(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs, BinOp op)
        : Expr(ASTKind::gBinExpr), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_op(op) {}

    [[nodiscard]] constexpr auto get_lhs() const -> const Expr& { return *m_lhs; }
    [[nodiscard]] constexpr auto get_lhs() -> Expr& { return *m_lhs; }
    auto set_lhs(std::unique_ptr<Expr> lhs) -> void { m_lhs = std::move(lhs); }

    [[nodiscard]] constexpr auto get_rhs() const -> const Expr& { return *m_rhs; }
    [[nodiscard]] constexpr auto get_rhs() -> Expr& { return *m_rhs; }
    auto set_rhs(std::unique_ptr<Expr> rhs) -> void { m_rhs = std::move(rhs); }

    [[nodiscard]] constexpr auto get_op() const -> BinOp { return m_op; }
    auto set_op(BinOp op) -> void { m_op = op; }
  };

  PLACEHOLDER_IMPL(UnaryExpr, ASTKind::gUnaryExpr);        // TODO: Implement node
  PLACEHOLDER_IMPL(Number, ASTKind::gNumber);              // TODO: Implement node
  PLACEHOLDER_IMPL(FString, ASTKind::gFString);            // TODO: Implement node
  PLACEHOLDER_IMPL(String, ASTKind::gString);              // TODO: Implement node
  PLACEHOLDER_IMPL(Char, ASTKind::gChar);                  // TODO: Implement node
  PLACEHOLDER_IMPL(List, ASTKind::gList);                  // TODO: Implement node
  PLACEHOLDER_IMPL(Ident, ASTKind::gIdent);                // TODO: Implement node
  PLACEHOLDER_IMPL(Index, ASTKind::gIndex);                // TODO: Implement node
  PLACEHOLDER_IMPL(Slice, ASTKind::gSlice);                // TODO: Implement node
  PLACEHOLDER_IMPL(Call, ASTKind::gCall);                  // TODO: Implement node
  PLACEHOLDER_IMPL(TemplateCall, ASTKind::gTemplateCall);  // TODO: Implement node
  PLACEHOLDER_IMPL(If, ASTKind::gIf);                      // TODO: Implement node
  PLACEHOLDER_IMPL(Else, ASTKind::gElse);                  // TODO: Implement node
  PLACEHOLDER_IMPL(For, ASTKind::gFor);                    // TODO: Implement node
  PLACEHOLDER_IMPL(While, ASTKind::gWhile);                // TODO: Implement node
  PLACEHOLDER_IMPL(Do, ASTKind::gDo);                      // TODO: Implement node
  PLACEHOLDER_IMPL(Switch, ASTKind::gSwitch);              // TODO: Implement node
  PLACEHOLDER_IMPL(Break, ASTKind::gBreak);                // TODO: Implement node
  PLACEHOLDER_IMPL(Continue, ASTKind::gContinue);          // TODO: Implement node
  PLACEHOLDER_IMPL(Return, ASTKind::gReturn);              // TODO: Implement node
  PLACEHOLDER_IMPL(Foreach, ASTKind::gForeach);            // TODO: Implement node
  PLACEHOLDER_IMPL(Try, ASTKind::gTry);                    // TODO: Implement node
  PLACEHOLDER_IMPL(Catch, ASTKind::gCatch);                // TODO: Implement node
  PLACEHOLDER_IMPL(Throw, ASTKind::gThrow);                // TODO: Implement node
  PLACEHOLDER_IMPL(Await, ASTKind::gAwait);                // TODO: Implement node
  PLACEHOLDER_IMPL(Asm, ASTKind::gAsm);                    // TODO: Implement node

  PLACEHOLDER_IMPL(InferTy, ASTKind::tInfer);        // TODO: Implement node
  PLACEHOLDER_IMPL(OpaqueTy, ASTKind::tOpaque);      // TODO: Implement node
  PLACEHOLDER_IMPL(NamedTy, ASTKind::tNamed);        // TODO: Implement node
  PLACEHOLDER_IMPL(RefTy, ASTKind::tRef);            // TODO: Implement node
  PLACEHOLDER_IMPL(PtrTy, ASTKind::tPtr);            // TODO: Implement node
  PLACEHOLDER_IMPL(ArrayTy, ASTKind::tArray);        // TODO: Implement node
  PLACEHOLDER_IMPL(TupleTy, ASTKind::tTuple);        // TODO: Implement node
  PLACEHOLDER_IMPL(TemplateTy, ASTKind::tTemplate);  // TODO: Implement node
  PLACEHOLDER_IMPL(LambdaTy, ASTKind::tLambda);      // TODO: Implement node

  PLACEHOLDER_IMPL(Let, ASTKind::sLet);            // TODO: Implement node
  PLACEHOLDER_IMPL(Var, ASTKind::sVar);            // TODO: Implement node
  PLACEHOLDER_IMPL(Fn, ASTKind::sFn);              // TODO: Implement node
  PLACEHOLDER_IMPL(Enum, ASTKind::sEnum);          // TODO: Implement node
  PLACEHOLDER_IMPL(Struct, ASTKind::sStruct);      // TODO: Implement node
  PLACEHOLDER_IMPL(Union, ASTKind::sUnion);        // TODO: Implement node
  PLACEHOLDER_IMPL(Contract, ASTKind::sContract);  // TODO: Implement node
  PLACEHOLDER_IMPL(Trait, ASTKind::sTrait);        // TODO: Implement node
  PLACEHOLDER_IMPL(TypeDef, ASTKind::sTypeDef);    // TODO: Implement node
  PLACEHOLDER_IMPL(Scope, ASTKind::sScope);        // TODO: Implement node
  PLACEHOLDER_IMPL(Import, ASTKind::sImport);      // TODO: Implement node
  PLACEHOLDER_IMPL(UnitTest, ASTKind::sUnitTest);  // TODO: Implement node

#define W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(visitor_name, constness)     \
  constexpr auto Expr::accept(visitor_name& visitor) constness -> void { \
    constness Expr& node = *this;                                        \
                                                                         \
    switch (get_kind()) {                                                \
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

  W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(Visitor, );
  W_NITRATE_PARSER_EXPR_ACCEPT_METHOD(ConstVisitor, const);

#undef W_NITRATE_PARSER_EXPR_ACCEPT_METHOD
}  // namespace nitrate::compiler::parser
