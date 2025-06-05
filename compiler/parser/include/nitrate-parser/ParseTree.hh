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
#include <deque>
#include <iostream>
#include <memory>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ParseTreeFwd.hh>
#include <nitrate-parser/Visitor.hh>
#include <span>

namespace nitrate::compiler::parser {
  class Expr {
    static constexpr size_t M_KIND_BITS = 6;  // 64 kinds max
    static_assert(static_cast<size_t>(ASTKIND_MAX) <= (1 << M_KIND_BITS) - 1,
                  "ASTKind must fit in 6 bits for Expr::m_kind");

    class SourceLocationTag {
      uint64_t m_id : 56;

    public:
      constexpr SourceLocationTag() : m_id(0) {}
      SourceLocationTag(boost::flyweight<lexer::FileSourceRange> source_range);
      SourceLocationTag(const SourceLocationTag&) = delete;
      SourceLocationTag(SourceLocationTag&& o) : m_id(o.m_id) { o.m_id = 0; }
      auto operator=(const SourceLocationTag&) -> SourceLocationTag& = delete;
      auto operator=(SourceLocationTag&& o) -> SourceLocationTag& = default;
      ~SourceLocationTag();

      [[nodiscard]] auto get() const -> const lexer::FileSourceRange&;
    } __attribute__((packed));

    ASTKind m_kind : M_KIND_BITS;
    bool m_is_discarded : 1 = false;
    bool m_is_parenthesized : 1 = false;
    SourceLocationTag m_source_range;

  public:
    constexpr Expr(ASTKind kind) : m_kind(kind) {}

    Expr(const Expr&) = delete;
    Expr(Expr&&) = default;
    auto operator=(const Expr&) -> Expr& = delete;
    auto operator=(Expr&&) -> Expr& = default;
    virtual ~Expr() = default;

    [[nodiscard]] auto operator==(const Expr& o) const -> bool;
    [[nodiscard]] auto hash() const -> size_t;

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

  static constexpr auto hash_value(const Expr& node) -> size_t { return node.hash(); }

  template <class T>
  class Nullable {
    T m_value;

  public:
    Nullable() = default;
    Nullable(T value) : m_value(std::move(value)) {}
    Nullable(const Nullable&) = delete;
    Nullable(Nullable&&) = default;
    auto operator=(const Nullable&) -> Nullable& = delete;
    auto operator=(Nullable&&) -> Nullable& = default;

    [[nodiscard]] constexpr auto has_value() const -> bool { return m_value != nullptr; }
    [[nodiscard]] constexpr operator bool() const { return has_value(); }

    [[nodiscard]] constexpr auto value() const -> const T& {
      assert(has_value() && "Nullable<T>::value() called on empty Nullable");
      return *m_value;
    }

    [[nodiscard]] constexpr auto value() -> T& {
      assert(has_value() && "Nullable<T>::value() called on empty Nullable");
      return *m_value;
    }
  };

#define W_PLACEHOLDER_IMPL(name, type) \
  class name : public Expr {           \
  public:                              \
    name() : Expr(type) {}             \
  };

  class BinExpr : public Expr {
  public:
    using LHS = std::unique_ptr<Expr>;
    using RHS = std::unique_ptr<Expr>;
    using Op = lexer::Operator;

    BinExpr(LHS lhs, RHS rhs, Op op)
        : Expr(ASTKind::gBinExpr), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_op(op) {}

    [[nodiscard]] constexpr auto get_lhs() const -> const Expr& { return *m_lhs; }
    [[nodiscard]] constexpr auto get_lhs() -> Expr& { return *m_lhs; }
    auto set_lhs(LHS lhs) -> void { m_lhs = std::move(lhs); }

    [[nodiscard]] constexpr auto get_rhs() const -> const Expr& { return *m_rhs; }
    [[nodiscard]] constexpr auto get_rhs() -> Expr& { return *m_rhs; }
    auto set_rhs(RHS rhs) -> void { m_rhs = std::move(rhs); }

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    auto set_op(Op op) -> void { m_op = op; }

  private:
    LHS m_lhs;
    RHS m_rhs;
    Op m_op;
  };

  class UnaryExpr : public Expr {
  public:
    using Op = lexer::Operator;
    using Operand = std::unique_ptr<Expr>;

    UnaryExpr(Operand operand, Op op, bool is_postfix)
        : Expr(ASTKind::gUnaryExpr), m_operand(std::move(operand)), m_op(op), m_is_postfix(is_postfix) {}

    [[nodiscard]] constexpr auto get_operand() const -> const Expr& { return *m_operand; }
    [[nodiscard]] constexpr auto get_operand() -> Expr& { return *m_operand; }
    auto set_operand(Operand operand) -> void { m_operand = std::move(operand); }

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    constexpr auto set_op(Op op) -> void { m_op = op; }

    [[nodiscard]] constexpr auto is_postfix() const -> bool { return m_is_postfix; }
    constexpr auto set_postfix(bool is_postfix) -> void { m_is_postfix = is_postfix; }

  private:
    Operand m_operand;
    Op m_op;
    bool m_is_postfix : 1;
  };

  W_PLACEHOLDER_IMPL(Number, ASTKind::gNumber);  // TODO: Implement node

  class FString : public Expr {
    using FStringPart = std::variant<std::string, std::unique_ptr<Expr>>;  // String or Expr
    using PartsList = std::pmr::vector<FStringPart>;

    PartsList m_parts;

  public:
    FString(PartsList parts) : Expr(ASTKind::gFString), m_parts(std::move(parts)) {}

    [[nodiscard]] constexpr auto get_parts() const -> const PartsList& { return m_parts; }
    [[nodiscard]] constexpr auto get_parts() -> PartsList& { return m_parts; }
    [[nodiscard]] constexpr auto is_empty() const -> bool { return m_parts.empty(); }
    [[nodiscard]] constexpr auto size() const -> size_t { return m_parts.size(); }

    auto set_parts(PartsList parts) -> void { m_parts = std::move(parts); }
    auto push_back(FStringPart part) -> void { m_parts.push_back(std::move(part)); }
    auto push_front(FStringPart part) -> void { m_parts.insert(m_parts.begin(), std::move(part)); }
    auto clear() -> void { m_parts.clear(); }

    [[nodiscard]] auto begin() -> PartsList::iterator { return m_parts.begin(); }
    [[nodiscard]] auto begin() const -> PartsList::const_iterator { return m_parts.begin(); }

    [[nodiscard]] auto end() -> PartsList::iterator { return m_parts.end(); }
    [[nodiscard]] auto end() const -> PartsList::const_iterator { return m_parts.end(); }
  };

  class String : public Expr {
    std::pmr::string m_value;

  public:
    String(std::pmr::string value) : Expr(ASTKind::gString), m_value(std::move(value)) {}

    [[nodiscard]] constexpr auto get_value() const -> const std::pmr::string& { return m_value; }
    auto set_value(std::pmr::string value) -> void { m_value = std::move(value); }
  };

  class Char : public Expr {
    using Codepoint = uint32_t;

  public:
    Char(Codepoint value) : Expr(ASTKind::gChar), m_value(value) {}

    [[nodiscard]] constexpr auto get_value() const -> Codepoint { return m_value; }
    auto set_value(Codepoint value) -> void { m_value = value; }

  private:
    Codepoint m_value;
  };

  class List : public Expr {
  public:
    using ElementsList = std::pmr::deque<std::unique_ptr<Expr>>;

    List(ElementsList elements) : Expr(ASTKind::gList), m_elements(std::move(elements)) {}

    [[nodiscard]] constexpr auto get_elements() const -> const ElementsList& { return m_elements; }
    [[nodiscard]] constexpr auto get_elements() -> ElementsList& { return m_elements; }
    [[nodiscard]] constexpr auto is_empty() const -> bool { return m_elements.empty(); }
    [[nodiscard]] constexpr auto size() const -> size_t { return m_elements.size(); }

    auto set_elements(ElementsList elements) -> void { m_elements = std::move(elements); }
    auto push_back(std::unique_ptr<Expr> element) -> void { m_elements.push_back(std::move(element)); }
    auto push_front(std::unique_ptr<Expr> element) -> void { m_elements.push_front(std::move(element)); }
    auto clear() -> void { m_elements.clear(); }

    [[nodiscard]] auto begin() -> ElementsList::iterator { return m_elements.begin(); }
    [[nodiscard]] auto begin() const -> ElementsList::const_iterator { return m_elements.begin(); }

    [[nodiscard]] auto end() -> ElementsList::iterator { return m_elements.end(); }
    [[nodiscard]] auto end() const -> ElementsList::const_iterator { return m_elements.end(); }

  private:
    ElementsList m_elements;
  };

  class Ident : public Expr {
    boost::flyweight<std::string> m_name;

  public:
    Ident(boost::flyweight<std::string> name) : Expr(ASTKind::gIdent), m_name(std::move(name)) {}
    Ident(std::string name) : Expr(ASTKind::gIdent), m_name(std::move(name)) {}

    [[nodiscard]] constexpr auto get_name() const -> const std::string& { return m_name.get(); }
    auto set_name(const boost::flyweight<std::string>& name) -> void { m_name = name; }
    auto set_name(std::string name) -> void { m_name = std::move(name); }
  };

  class Index : public Expr {
    std::unique_ptr<Expr> m_target, m_index;

  public:
    Index(std::unique_ptr<Expr> target, std::unique_ptr<Expr> index)
        : Expr(ASTKind::gIndex), m_target(std::move(target)), m_index(std::move(index)) {}

    [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }
    [[nodiscard]] constexpr auto get_target() -> Expr& { return *m_target; }
    auto set_target(std::unique_ptr<Expr> target) -> void { m_target = std::move(target); }

    [[nodiscard]] constexpr auto get_index() const -> const Expr& { return *m_index; }
    [[nodiscard]] constexpr auto get_index() -> Expr& { return *m_index; }
    auto set_index(std::unique_ptr<Expr> index) -> void { m_index = std::move(index); }
  };

  class Slice : public Expr {
    std::unique_ptr<Expr> m_target, m_start, m_end;

  public:
    Slice(std::unique_ptr<Expr> target, std::unique_ptr<Expr> start, std::unique_ptr<Expr> end)
        : Expr(ASTKind::gSlice), m_target(std::move(target)), m_start(std::move(start)), m_end(std::move(end)) {}

    [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }
    [[nodiscard]] constexpr auto get_target() -> Expr& { return *m_target; }
    auto set_target(std::unique_ptr<Expr> target) -> void { m_target = std::move(target); }

    [[nodiscard]] constexpr auto get_start() const -> const Expr& { return *m_start; }
    [[nodiscard]] constexpr auto get_start() -> Expr& { return *m_start; }
    auto set_start(std::unique_ptr<Expr> start) -> void { m_start = std::move(start); }

    [[nodiscard]] constexpr auto get_end() const -> const Expr& { return *m_end; }
    [[nodiscard]] constexpr auto get_end() -> Expr& { return *m_end; }
    auto set_end(std::unique_ptr<Expr> end) -> void { m_end = std::move(end); }
  };

  W_PLACEHOLDER_IMPL(Call, ASTKind::gCall);                  // TODO: Implement node
  W_PLACEHOLDER_IMPL(TemplateCall, ASTKind::gTemplateCall);  // TODO: Implement node

  class If : public Expr {
    std::unique_ptr<Expr> m_condition, m_then_branch;
    Nullable<std::unique_ptr<Expr>> m_else_branch;

  public:
    using ElseNodePtr = Nullable<std::unique_ptr<Expr>>;

    If(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> then_branch, Nullable<std::unique_ptr<Expr>> else_branch)
        : Expr(ASTKind::gIf),
          m_condition(std::move(condition)),
          m_then_branch(std::move(then_branch)),
          m_else_branch(std::move(else_branch)) {}

    [[nodiscard]] constexpr auto get_condition() const -> const Expr& { return *m_condition; }
    [[nodiscard]] constexpr auto get_condition() -> Expr& { return *m_condition; }
    auto set_condition(std::unique_ptr<Expr> condition) -> void { m_condition = std::move(condition); }

    [[nodiscard]] constexpr auto get_then_branch() const -> const Expr& { return *m_then_branch; }
    [[nodiscard]] constexpr auto get_then_branch() -> Expr& { return *m_then_branch; }
    auto set_then_branch(std::unique_ptr<Expr> then_branch) -> void { m_then_branch = std::move(then_branch); }

    [[nodiscard]] constexpr auto get_else_branch() const -> const ElseNodePtr& { return m_else_branch; }
    [[nodiscard]] constexpr auto get_else_branch() -> ElseNodePtr& { return m_else_branch; }
    [[nodiscard]] constexpr auto has_else_branch() const -> bool { return m_else_branch.has_value(); }
    auto set_else_branch(ElseNodePtr else_branch) -> void { m_else_branch = std::move(else_branch); }
  };

  W_PLACEHOLDER_IMPL(Else, ASTKind::gElse);  // TODO: Implement node
  W_PLACEHOLDER_IMPL(For, ASTKind::gFor);    // TODO: Implement node

  class While : public Expr {
    Nullable<std::unique_ptr<Expr>> m_condition;
    std::unique_ptr<Expr> m_body;

  public:
    While(Nullable<std::unique_ptr<Expr>> condition, std::unique_ptr<Expr> body)
        : Expr(ASTKind::gWhile), m_condition(std::move(condition)), m_body(std::move(body)) {}

    [[nodiscard]] constexpr auto get_condition() const -> const Nullable<std::unique_ptr<Expr>>& { return m_condition; }
    [[nodiscard]] constexpr auto get_condition() -> Nullable<std::unique_ptr<Expr>>& { return m_condition; }
    [[nodiscard]] constexpr auto has_condition() const -> bool { return m_condition.has_value(); }
    auto set_condition(Nullable<std::unique_ptr<Expr>> condition) -> void { m_condition = std::move(condition); }

    [[nodiscard]] constexpr auto get_body() const -> const Expr& { return *m_body; }
    [[nodiscard]] constexpr auto get_body() -> Expr& { return *m_body; }
    auto set_body(std::unique_ptr<Expr> body) -> void { m_body = std::move(body); }
  };

  W_PLACEHOLDER_IMPL(Do, ASTKind::gDo);          // TODO: Implement node
  W_PLACEHOLDER_IMPL(Switch, ASTKind::gSwitch);  // TODO: Implement node

  class Break : public Expr {
  public:
    constexpr Break() : Expr(ASTKind::gBreak) {}
  };

  class Continue : public Expr {
  public:
    constexpr Continue() : Expr(ASTKind::gContinue) {}
  };

  class Return : public Expr {
    Nullable<std::unique_ptr<Expr>> m_value;

  public:
    Return(Nullable<std::unique_ptr<Expr>> value) : Expr(ASTKind::gReturn), m_value(std::move(value)) {}

    [[nodiscard]] constexpr auto get_value() const -> const Nullable<std::unique_ptr<Expr>>& { return m_value; }
    [[nodiscard]] constexpr auto get_value() -> Nullable<std::unique_ptr<Expr>>& { return m_value; }
    [[nodiscard]] constexpr auto has_value() const -> bool { return m_value.has_value(); }
    auto set_value(Nullable<std::unique_ptr<Expr>> value) -> void { m_value = std::move(value); }
  };

  W_PLACEHOLDER_IMPL(Foreach, ASTKind::gForeach);  // TODO: Implement node
  W_PLACEHOLDER_IMPL(Try, ASTKind::gTry);          // TODO: Implement node
  W_PLACEHOLDER_IMPL(Catch, ASTKind::gCatch);      // TODO: Implement node
  W_PLACEHOLDER_IMPL(Throw, ASTKind::gThrow);      // TODO: Implement node

  W_PLACEHOLDER_IMPL(Await, ASTKind::gAwait);  // TODO: Implement node
  W_PLACEHOLDER_IMPL(Asm, ASTKind::gAsm);      // TODO: Implement node

  class InferTy : public Expr {
  public:
    constexpr InferTy() : Expr(ASTKind::tInfer) {}
  };

  class OpaqueTy : public Expr {
    boost::flyweight<std::string> m_name;

  public:
    OpaqueTy(boost::flyweight<std::string> name) : Expr(ASTKind::tOpaque), m_name(std::move(name)) {}
    OpaqueTy(std::string name) : Expr(ASTKind::tOpaque), m_name(std::move(name)) {}

    [[nodiscard]] constexpr auto get_name() const -> const std::string& { return m_name.get(); }
  };

  class NamedTy : public Expr {
    boost::flyweight<std::string> m_name;

  public:
    NamedTy(boost::flyweight<std::string> name) : Expr(ASTKind::tNamed), m_name(std::move(name)) {}
    NamedTy(std::string name) : Expr(ASTKind::tNamed), m_name(std::move(name)) {}

    [[nodiscard]] constexpr auto get_name() const -> const std::string& { return m_name.get(); }
  };

  class RefTy : public Expr {
    std::unique_ptr<Expr> m_target;

  public:
    RefTy(std::unique_ptr<Expr> target) : Expr(ASTKind::tRef), m_target(std::move(target)) {}

    [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }
  };

  class PtrTy : public Expr {
    std::unique_ptr<Expr> m_target;

  public:
    PtrTy(std::unique_ptr<Expr> target) : Expr(ASTKind::tPtr), m_target(std::move(target)) {}

    [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }
  };

  class ArrayTy : public Expr {
    std::unique_ptr<Expr> m_element_type, m_size;

  public:
    ArrayTy(std::unique_ptr<Expr> element_type, std::unique_ptr<Expr> size)
        : Expr(ASTKind::tArray), m_element_type(std::move(element_type)), m_size(std::move(size)) {}

    [[nodiscard]] constexpr auto get_element_type() const -> const Expr& { return *m_element_type; }
    [[nodiscard]] constexpr auto get_size() const -> const Expr& { return *m_size; }
  };

  class TupleTy : public Expr {
  public:
    using ElementsList = std::vector<std::unique_ptr<Expr>>;
    using ElementsSpan = std::span<const std::unique_ptr<Expr>>;

    TupleTy(ElementsList elements) : Expr(ASTKind::tTuple), m_elements(std::move(elements)) {}

    [[nodiscard]] constexpr auto get_elements() const -> ElementsSpan { return m_elements; }
    [[nodiscard]] constexpr auto is_empty() const -> bool { return m_elements.empty(); }
    [[nodiscard]] constexpr auto size() const -> size_t { return m_elements.size(); }

    [[nodiscard]] auto begin() const -> ElementsSpan::iterator { return get_elements().begin(); }
    [[nodiscard]] auto end() const -> ElementsSpan::iterator { return get_elements().end(); }

  private:
    ElementsList m_elements;
  };

  class TemplateTy : public Expr {
  public:
    TemplateTy() : Expr(ASTKind::tTemplate) {}
    // TODO: Implement TemplateTy node
  };

  class LambdaTy : public Expr {
  public:
    LambdaTy() : Expr(ASTKind::tLambda) {}

    // TODO: Implement LambdaTy node
  };

  W_PLACEHOLDER_IMPL(Let, ASTKind::sLet);            // TODO: Implement node
  W_PLACEHOLDER_IMPL(Var, ASTKind::sVar);            // TODO: Implement node
  W_PLACEHOLDER_IMPL(Fn, ASTKind::sFn);              // TODO: Implement node
  W_PLACEHOLDER_IMPL(Enum, ASTKind::sEnum);          // TODO: Implement node
  W_PLACEHOLDER_IMPL(Struct, ASTKind::sStruct);      // TODO: Implement node
  W_PLACEHOLDER_IMPL(Union, ASTKind::sUnion);        // TODO: Implement node
  W_PLACEHOLDER_IMPL(Contract, ASTKind::sContract);  // TODO: Implement node
  W_PLACEHOLDER_IMPL(Trait, ASTKind::sTrait);        // TODO: Implement node
  W_PLACEHOLDER_IMPL(TypeDef, ASTKind::sTypeDef);    // TODO: Implement node
  W_PLACEHOLDER_IMPL(Scope, ASTKind::sScope);        // TODO: Implement node
  W_PLACEHOLDER_IMPL(Import, ASTKind::sImport);      // TODO: Implement node
  W_PLACEHOLDER_IMPL(UnitTest, ASTKind::sUnitTest);  // TODO: Implement node

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
