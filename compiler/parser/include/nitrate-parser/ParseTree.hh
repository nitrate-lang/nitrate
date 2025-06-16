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
#include <memory>
#include <nitrate-lexer/Token.hh>

namespace nitrate::compiler::parser {
  enum class ASTKind : uint8_t {
    /* General Expression Nodes */
    gBinExpr,
    gUnaryExpr,
    gNumber,
    gFString,
    gString,
    gChar,
    gList,
    gIdent,
    gIndex,
    gSlice,
    gCall,
    gTemplateCall,
    gIf,
    gElse,
    gFor,
    gWhile,
    gDo,
    gSwitch,
    gBreak,
    gContinue,
    gReturn,
    gForeach,
    gTry,
    gCatch,
    gThrow,
    gAwait,
    gAsm,

    /* Type Nodes */
    tInfer,
    tOpaque,
    tNamed,
    tRef,
    tPtr,
    tArray,
    tTuple,
    tTemplate,
    tLambda,

    /* Symbol Nodes */
    sVar,
    sFn,
    sEnum,
    sStruct,
    sUnion,
    sContract,
    sTrait,
    sTypeDef,
    sScope,
    sImport,
    sUnitTest,
  };

  class BinExpr;
  class UnaryExpr;
  class Number;
  class FString;
  class String;
  class Char;
  class List;
  class Ident;
  class Index;
  class Slice;
  class Call;
  class TemplateCall;
  class If;
  class Else;
  class For;
  class While;
  class Do;
  class Switch;
  class Break;
  class Continue;
  class Return;
  class Foreach;
  class Try;
  class Catch;
  class Throw;
  class Await;
  class Asm;
  class InferTy;
  class OpaqueTy;
  class NamedTy;
  class RefTy;
  class PtrTy;
  class ArrayTy;
  class TupleTy;
  class TemplateTy;
  class LambdaTy;

  using Expr = std::variant<BinExpr, UnaryExpr>;

  using Type = std::variant<InferTy>;

  namespace detail {
    class SourceLocationTag {
      uint64_t m_id : 48;

    public:
      constexpr SourceLocationTag() : m_id(0) {}
      SourceLocationTag(boost::flyweight<lexer::FileSourceRange> source_range);
      SourceLocationTag(const SourceLocationTag&);
      SourceLocationTag(SourceLocationTag&& o) : m_id(o.m_id) { o.m_id = 0; }
      auto operator=(const SourceLocationTag&) -> SourceLocationTag&;
      auto operator=(SourceLocationTag&& o) -> SourceLocationTag& {
        m_id = o.m_id;
        o.m_id = 0;
        return *this;
      }
      ~SourceLocationTag();

      [[nodiscard]] constexpr auto operator<=>(const SourceLocationTag&) const -> std::weak_ordering = default;

      [[nodiscard]] auto get() const -> const lexer::FileSourceRange&;
    } __attribute__((packed));

#define W_NITRATE_AST_PARENTHESIZED_TRAIT()                                                    \
  [[nodiscard]] constexpr auto is_parenthesized() const -> bool { return m_is_parenthesized; } \
  constexpr auto set_parenthesized(bool b) -> void { m_is_parenthesized = b; }

#define W_NITRATE_AST_SOURCE_RANGE_TRAIT()                                                                            \
  [[nodiscard]] constexpr auto source_range() const -> const lexer::FileSourceRange& { return m_source_range.get(); } \
  auto set_source_range(lexer::FileSourceRange source_range) -> void {                                                \
    m_source_range = detail::SourceLocationTag(boost::flyweight<lexer::FileSourceRange>(std::move(source_range)));    \
  }

    auto clone_expr(const std::unique_ptr<Expr>& expr) -> std::unique_ptr<Expr>;
  }  // namespace detail

#define W_PLACEHOLDER_IMPL(name, type) \
  class name {                         \
  public:                              \
    name() {}                          \
  };

  class BinExpr final {
  public:
    using LHS = Expr;
    using RHS = Expr;
    using Op = lexer::Operator;

    BinExpr() = delete;
    BinExpr(LHS lhs, RHS rhs, Op op);
    BinExpr(const BinExpr& o)
        : m_source_range(o.m_source_range),
          m_is_parenthesized(o.m_is_parenthesized),
          m_op(o.m_op),
          m_lhs(detail::clone_expr(o.m_lhs)),
          m_rhs(detail::clone_expr(o.m_rhs)) {}
    BinExpr(BinExpr&&) = default;
    auto operator=(const BinExpr& o) -> BinExpr& {
      m_source_range = o.m_source_range;
      m_is_parenthesized = o.m_is_parenthesized;
      m_op = o.m_op;
      m_lhs = detail::clone_expr(o.m_lhs);
      m_rhs = detail::clone_expr(o.m_rhs);
      return *this;
    }
    auto operator=(BinExpr&& o) -> BinExpr& = default;
    ~BinExpr() = default;

    [[nodiscard]] constexpr auto operator==(const BinExpr& o) const -> bool;
    [[nodiscard]] constexpr auto operator<=>(const BinExpr& o) const -> std::weak_ordering;

    [[nodiscard]] constexpr auto get_lhs() const -> const LHS&;
    [[nodiscard]] constexpr auto get_lhs() -> LHS&;
    auto set_lhs(LHS lhs) -> void;

    [[nodiscard]] constexpr auto get_rhs() const -> const RHS&;
    [[nodiscard]] constexpr auto get_rhs() -> RHS&;
    auto set_rhs(RHS rhs) -> void;

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    auto set_op(Op op) -> void { m_op = op; }

    W_NITRATE_AST_PARENTHESIZED_TRAIT();
    W_NITRATE_AST_SOURCE_RANGE_TRAIT();

  protected:
    using MemberTuple = std::tuple<const detail::SourceLocationTag&, const bool&, const Op&, const LHS&, const RHS&>;

    [[nodiscard]] constexpr auto state() const -> MemberTuple;

  private:
    detail::SourceLocationTag m_source_range;
    bool m_is_parenthesized : 1 = false;
    Op m_op : 7;
    std::unique_ptr<LHS> m_lhs;
    std::unique_ptr<RHS> m_rhs;
  };

  class UnaryExpr final {
  public:
    using Op = lexer::Operator;
    using Operand = Expr;

    UnaryExpr() = delete;
    UnaryExpr(Operand operand, Op op, bool is_postfix)
        : m_op(op), m_is_postfix(is_postfix), m_operand(std::make_unique<Operand>(std::move(operand))) {}
    UnaryExpr(const UnaryExpr& o)
        : m_source_range(o.m_source_range),
          m_is_parenthesized(o.m_is_parenthesized),
          m_op(o.m_op),
          m_is_postfix(o.m_is_postfix),
          m_operand(detail::clone_expr(o.m_operand)) {}
    UnaryExpr(UnaryExpr&&) = default;
    auto operator=(const UnaryExpr& o) -> UnaryExpr& {
      m_source_range = o.m_source_range;
      m_is_parenthesized = o.m_is_parenthesized;
      m_op = o.m_op;
      m_is_postfix = o.m_is_postfix;
      m_operand = detail::clone_expr(o.m_operand);
      return *this;
    }
    auto operator=(UnaryExpr&&) -> UnaryExpr& = default;
    ~UnaryExpr() = default;

    [[nodiscard]] constexpr auto operator==(const UnaryExpr&) const -> bool;
    [[nodiscard]] constexpr auto operator<=>(const UnaryExpr&) const -> std::weak_ordering;

    [[nodiscard]] constexpr auto get_operand() const -> const Operand&;
    [[nodiscard]] constexpr auto get_operand() -> Operand&;
    auto set_operand(Operand operand) -> void;

    [[nodiscard]] constexpr auto get_op() const -> Op { return m_op; }
    constexpr auto set_op(Op op) -> void { m_op = op; }

    [[nodiscard]] constexpr auto is_postfix() const -> bool { return m_is_postfix; }
    constexpr auto set_postfix(bool is_postfix) -> void { m_is_postfix = is_postfix; }

    W_NITRATE_AST_PARENTHESIZED_TRAIT();
    W_NITRATE_AST_SOURCE_RANGE_TRAIT();

  protected:
    using MemberTuple =
        std::tuple<const detail::SourceLocationTag&, const bool&, const Op&, const bool&, const Operand&>;

    [[nodiscard]] constexpr auto state() const -> MemberTuple;

  private:
    detail::SourceLocationTag m_source_range;
    bool m_is_parenthesized : 1 = false;
    Op m_op : 7;
    bool m_is_postfix : 1;
    std::unique_ptr<Operand> m_operand;
  };

  // W_PLACEHOLDER_IMPL(Number, ASTKind::gNumber);  // TODO: Implement node number

  // class FString final {
  // public:
  //   using Element = std::variant<std::string, Expr>;
  //   using ElementsList = std::pmr::vector<Element>;

  //   FString(ElementsList elements) : m_elements(std::move(elements)) {}

  //   [[nodiscard]] constexpr auto get_elements() const -> const ElementsList& { return m_elements; }
  //   [[nodiscard]] constexpr auto get_elements() -> ElementsList& { return m_elements; }
  //   [[nodiscard]] constexpr auto is_empty() const -> bool { return m_elements.empty(); }
  //   [[nodiscard]] constexpr auto size() const -> size_t { return m_elements.size(); }

  //   auto set_elements(ElementsList elements) -> void { m_elements = std::move(elements); }
  //   auto push_back(Element part) -> void { m_elements.push_back(std::move(part)); }
  //   auto push_front(Element part) -> void { m_elements.insert(m_elements.begin(), std::move(part)); }
  //   auto clear() -> void { m_elements.clear(); }

  //   [[nodiscard]] auto begin() -> ElementsList::iterator { return m_elements.begin(); }
  //   [[nodiscard]] auto begin() const -> ElementsList::const_iterator { return m_elements.begin(); }

  //   [[nodiscard]] auto end() -> ElementsList::iterator { return m_elements.end(); }
  //   [[nodiscard]] auto end() const -> ElementsList::const_iterator { return m_elements.end(); }

  // private:
  //   ElementsList m_elements;
  // };

  class String {
  public:
    using ValueType = std::pmr::string;

    String() = default;
    String(ValueType value) : m_value(std::move(value)) {}
    String(const String&) = delete;
    String(String&&) = default;
    auto operator=(const String&) -> String& = delete;
    auto operator=(String&&) -> String& = default;
    ~String() = default;

    [[nodiscard]] constexpr auto get_value() const -> const ValueType& { return m_value; }
    [[nodiscard]] constexpr auto get_value() -> ValueType& { return m_value; }
    [[nodiscard]] constexpr auto is_empty() const -> bool { return m_value.empty(); }
    [[nodiscard]] constexpr auto size() const -> size_t { return m_value.size(); }

    auto set_value(ValueType value) -> void { m_value = std::move(value); }

  private:
    ValueType m_value;
  };

  // class Char {
  // public:
  //   using Codepoint = uint32_t;

  //   Char(Codepoint value) : m_value(value) {}

  //   [[nodiscard]] constexpr auto get_value() const -> Codepoint { return m_value; }
  //   auto set_value(Codepoint value) -> void { m_value = value; }

  // private:
  //   Codepoint m_value;
  // };

  // class List {
  // public:
  //   using Element = std::unique_ptr<Expr>;
  //   using ElementsList = std::pmr::deque<Element>;

  //   List(ElementsList elements) : m_elements(std::move(elements)) {}

  //   [[nodiscard]] constexpr auto get_elements() const -> const ElementsList& { return m_elements; }
  //   [[nodiscard]] constexpr auto get_elements() -> ElementsList& { return m_elements; }
  //   [[nodiscard]] constexpr auto is_empty() const -> bool { return m_elements.empty(); }
  //   [[nodiscard]] constexpr auto size() const -> size_t { return m_elements.size(); }

  //   auto set_elements(ElementsList elements) -> void { m_elements = std::move(elements); }
  //   auto push_back(Element element) -> void { m_elements.push_back(std::move(element)); }
  //   auto push_front(Element element) -> void { m_elements.push_front(std::move(element)); }
  //   auto clear() -> void { m_elements.clear(); }

  //   [[nodiscard]] auto begin() -> ElementsList::iterator { return m_elements.begin(); }
  //   [[nodiscard]] auto begin() const -> ElementsList::const_iterator { return m_elements.begin(); }

  //   [[nodiscard]] auto end() -> ElementsList::iterator { return m_elements.end(); }
  //   [[nodiscard]] auto end() const -> ElementsList::const_iterator { return m_elements.end(); }

  // private:
  //   ElementsList m_elements;
  // };

  // class Ident {
  // public:
  //   using NameType = boost::flyweight<std::string>;

  //   Ident(NameType name) : m_name(std::move(name)) {}
  //   Ident(std::string name) : m_name(std::move(name)) {}

  //   [[nodiscard]] constexpr auto get_name() const -> const std::string& { return m_name.get(); }
  //   [[nodiscard]] constexpr auto is_empty() const -> bool { return m_name->empty(); }
  //   [[nodiscard]] constexpr auto size() const -> size_t { return m_name->size(); }

  //   auto set_name(const NameType& name) -> void { m_name = name; }
  //   auto set_name(std::string name) -> void { m_name = std::move(name); }

  // private:
  //   NameType m_name;
  // };

  // class Index {
  // public:
  //   using Target = std::unique_ptr<Expr>;
  //   using IndexExpr = std::unique_ptr<Expr>;

  //   Index(Target target, IndexExpr index) : m_target(std::move(target)), m_index(std::move(index)) {}

  //   [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }
  //   [[nodiscard]] constexpr auto get_target() -> Expr& { return *m_target; }
  //   auto set_target(Target target) -> void { m_target = std::move(target); }

  //   [[nodiscard]] constexpr auto get_index() const -> const Expr& { return *m_index; }
  //   [[nodiscard]] constexpr auto get_index() -> Expr& { return *m_index; }
  //   auto set_index(IndexExpr index) -> void { m_index = std::move(index); }

  // private:
  //   Target m_target;
  //   IndexExpr m_index;
  // };

  // class Slice {
  // public:
  //   using Target = std::unique_ptr<Expr>;
  //   using Start = std::unique_ptr<Expr>;
  //   using End = std::unique_ptr<Expr>;

  //   Slice(Target target, Start start, End end)
  //       : m_target(std::move(target)), m_start(std::move(start)), m_end(std::move(end)) {}

  //   [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }
  //   [[nodiscard]] constexpr auto get_target() -> Expr& { return *m_target; }
  //   auto set_target(Target target) -> void { m_target = std::move(target); }

  //   [[nodiscard]] constexpr auto get_start() const -> const Expr& { return *m_start; }
  //   [[nodiscard]] constexpr auto get_start() -> Expr& { return *m_start; }
  //   auto set_start(Start start) -> void { m_start = std::move(start); }

  //   [[nodiscard]] constexpr auto get_end() const -> const Expr& { return *m_end; }
  //   [[nodiscard]] constexpr auto get_end() -> Expr& { return *m_end; }
  //   auto set_end(End end) -> void { m_end = std::move(end); }

  // private:
  //   Target m_target;
  //   Start m_start;
  //   End m_end;
  // };

  // W_PLACEHOLDER_IMPL(Call, ASTKind::gCall);                  // TODO: Implement node call
  // W_PLACEHOLDER_IMPL(TemplateCall, ASTKind::gTemplateCall);  // TODO: Implement node template call

  // class If {
  // public:
  //   using Condition = std::unique_ptr<Expr>;
  //   using ThenBranch = std::unique_ptr<Expr>;
  //   using ElseBranch = Nullable<std::unique_ptr<Expr>>;

  //   If(Condition condition, ThenBranch then_branch, ElseBranch else_branch)
  //       : m_condition(std::move(condition)),
  //         m_then_branch(std::move(then_branch)),
  //         m_else_branch(std::move(else_branch)) {}

  //   [[nodiscard]] constexpr auto get_condition() const -> const Expr& { return *m_condition; }
  //   [[nodiscard]] constexpr auto get_condition() -> Expr& { return *m_condition; }
  //   auto set_condition(Condition condition) -> void { m_condition = std::move(condition); }

  //   [[nodiscard]] constexpr auto get_then_branch() const -> const Expr& { return *m_then_branch; }
  //   [[nodiscard]] constexpr auto get_then_branch() -> Expr& { return *m_then_branch; }
  //   auto set_then_branch(ThenBranch then_branch) -> void { m_then_branch = std::move(then_branch); }

  //   [[nodiscard]] constexpr auto get_else_branch() const -> const ElseBranch& { return m_else_branch; }
  //   [[nodiscard]] constexpr auto get_else_branch() -> ElseBranch& { return m_else_branch; }
  //   [[nodiscard]] constexpr auto has_else_branch() const -> bool { return m_else_branch.has_value(); }
  //   auto set_else_branch(ElseBranch else_branch) -> void { m_else_branch = std::move(else_branch); }

  // private:
  //   Condition m_condition;
  //   ThenBranch m_then_branch;
  //   ElseBranch m_else_branch;
  // };

  // W_PLACEHOLDER_IMPL(Else, ASTKind::gElse);  // TODO: Implement node else
  // W_PLACEHOLDER_IMPL(For, ASTKind::gFor);    // TODO: Implement node for

  // class While {
  // public:
  //   using Condition = Nullable<std::unique_ptr<Expr>>;
  //   using Body = std::unique_ptr<Expr>;

  //   While(Condition condition, Body body) : m_condition(std::move(condition)), m_body(std::move(body)) {}

  //   [[nodiscard]] constexpr auto get_condition() const -> const Condition& { return m_condition; }
  //   [[nodiscard]] constexpr auto get_condition() -> Condition& { return m_condition; }
  //   [[nodiscard]] constexpr auto has_condition() const -> bool { return m_condition.has_value(); }
  //   auto set_condition(Condition condition) -> void { m_condition = std::move(condition); }

  //   [[nodiscard]] constexpr auto get_body() const -> const Expr& { return *m_body; }
  //   [[nodiscard]] constexpr auto get_body() -> Expr& { return *m_body; }
  //   auto set_body(Body body) -> void { m_body = std::move(body); }

  // private:
  //   Condition m_condition;
  //   Body m_body;
  // };

  // W_PLACEHOLDER_IMPL(Do, ASTKind::gDo);          // TODO: Implement node do
  // W_PLACEHOLDER_IMPL(Switch, ASTKind::gSwitch);  // TODO: Implement node switch

  // class Break {
  // public:
  //   constexpr Break() = default;
  // };

  // class Continue {
  // public:
  //   constexpr Continue() = default;
  // };

  // class Return {
  // public:
  //   using ValueType = Nullable<std::unique_ptr<Expr>>;

  //   Return(ValueType value) : m_value(std::move(value)) {}

  //   [[nodiscard]] constexpr auto get_value() const -> const ValueType& { return m_value; }
  //   [[nodiscard]] constexpr auto get_value() -> ValueType& { return m_value; }
  //   [[nodiscard]] constexpr auto has_value() const -> bool { return m_value.has_value(); }
  //   auto set_value(ValueType value) -> void { m_value = std::move(value); }

  // private:
  //   ValueType m_value;
  // };

  // W_PLACEHOLDER_IMPL(Foreach, ASTKind::gForeach);  // TODO: Implement node foreach
  // W_PLACEHOLDER_IMPL(Try, ASTKind::gTry);          // TODO: Implement node try
  // W_PLACEHOLDER_IMPL(Catch, ASTKind::gCatch);      // TODO: Implement node catch
  // W_PLACEHOLDER_IMPL(Throw, ASTKind::gThrow);      // TODO: Implement node throw

  // W_PLACEHOLDER_IMPL(Await, ASTKind::gAwait);  // TODO: Implement node await
  // W_PLACEHOLDER_IMPL(Asm, ASTKind::gAsm);      // TODO: Implement node asm

  class InferTy {
  public:
    constexpr InferTy() = default;
  };

  // class OpaqueTy {
  // public:
  //   using NameType = boost::flyweight<std::string>;

  //   OpaqueTy(NameType name) : m_name(std::move(name)) {}
  //   OpaqueTy(std::string name) : m_name(std::move(name)) {}

  //   [[nodiscard]] constexpr auto get_name() const -> const std::string& { return m_name.get(); }

  // private:
  //   NameType m_name;
  // };

  // class NamedTy {
  // public:
  //   using NameType = boost::flyweight<std::string>;

  //   NamedTy(NameType name) : m_name(std::move(name)) {}
  //   NamedTy(std::string name) : m_name(std::move(name)) {}

  //   [[nodiscard]] constexpr auto get_name() const -> const std::string& { return m_name.get(); }

  // private:
  //   NameType m_name;
  // };

  // class RefTy {
  // public:
  //   using Target = std::unique_ptr<Expr>;

  //   RefTy(Target target) : m_target(std::move(target)) {}

  //   [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }

  // private:
  //   Target m_target;
  // };

  // class PtrTy {
  // public:
  //   using Target = std::unique_ptr<Expr>;

  //   PtrTy(Target target) : m_target(std::move(target)) {}

  //   [[nodiscard]] constexpr auto get_target() const -> const Expr& { return *m_target; }

  // private:
  //   Target m_target;
  // };

  // class ArrayTy {
  // public:
  //   using ElementType = std::unique_ptr<Expr>;
  //   using Size = std::unique_ptr<Expr>;

  //   ArrayTy(ElementType element_type, Size size) : m_element_type(std::move(element_type)), m_size(std::move(size))
  //   {}

  //   [[nodiscard]] constexpr auto get_element_type() const -> const Expr& { return *m_element_type; }
  //   [[nodiscard]] constexpr auto get_size() const -> const Expr& { return *m_size; }

  // private:
  //   ElementType m_element_type;
  //   Size m_size;
  // };

  // class TupleTy {
  // public:
  //   using ElementsList = std::vector<std::unique_ptr<Expr>>;
  //   using ElementsSpan = std::span<const std::unique_ptr<Expr>>;

  //   TupleTy(ElementsList elements) : m_elements(std::move(elements)) {}

  //   [[nodiscard]] constexpr auto get_elements() const -> ElementsSpan { return m_elements; }
  //   [[nodiscard]] constexpr auto is_empty() const -> bool { return m_elements.empty(); }
  //   [[nodiscard]] constexpr auto size() const -> size_t { return m_elements.size(); }

  //   [[nodiscard]] auto begin() const -> ElementsSpan::iterator { return get_elements().begin(); }
  //   [[nodiscard]] auto end() const -> ElementsSpan::iterator { return get_elements().end(); }

  // private:
  //   ElementsList m_elements;
  // };

  // class TemplateTy {
  // public:
  //   TemplateTy() = default;
  //   // TODO: Implement TemplateTy node
  // };

  // class LambdaTy {
  // public:
  //   LambdaTy() = default;

  //   // TODO: Implement LambdaTy node
  // };

  // W_PLACEHOLDER_IMPL(Var, ASTKind::sVar);            // TODO: Implement node Var
  // W_PLACEHOLDER_IMPL(Fn, ASTKind::sFn);              // TODO: Implement node Fn
  // W_PLACEHOLDER_IMPL(Enum, ASTKind::sEnum);          // TODO: Implement node Enum
  // W_PLACEHOLDER_IMPL(Struct, ASTKind::sStruct);      // TODO: Implement node Struct
  // W_PLACEHOLDER_IMPL(Union, ASTKind::sUnion);        // TODO: Implement node Union
  // W_PLACEHOLDER_IMPL(Contract, ASTKind::sContract);  // TODO: Implement node Contract
  // W_PLACEHOLDER_IMPL(Trait, ASTKind::sTrait);        // TODO: Implement node Trait
  // W_PLACEHOLDER_IMPL(TypeDef, ASTKind::sTypeDef);    // TODO: Implement node TypeDef
  // W_PLACEHOLDER_IMPL(Scope, ASTKind::sScope);        // TODO: Implement node Scope
  // W_PLACEHOLDER_IMPL(Import, ASTKind::sImport);      // TODO: Implement node Import
  // W_PLACEHOLDER_IMPL(UnitTest, ASTKind::sUnitTest);  // TODO: Implement node UnitTest

  constexpr auto BinExpr::state() const -> MemberTuple {
    return std::tie(m_source_range, m_is_parenthesized, m_op, *m_lhs, *m_rhs);
  }

  constexpr auto BinExpr::operator==(const BinExpr& o) const -> bool { return state() == o.state(); }
  constexpr auto BinExpr::operator<=>(const BinExpr& o) const -> std::weak_ordering { return state() <=> o.state(); }

  constexpr auto UnaryExpr::state() const -> MemberTuple {
    return std::tie(m_source_range, m_is_parenthesized, m_op, m_is_postfix, *m_operand);
  }

  constexpr auto UnaryExpr::operator==(const UnaryExpr& o) const -> bool { return state() == o.state(); }
  constexpr auto UnaryExpr::operator<=>(const UnaryExpr& o) const -> std::weak_ordering {
    return state() <=> o.state();
  }

  namespace detail {
#undef W_NITRATE_AST_SOURCE_RANGE_TRAIT
#undef W_NITRATE_AST_PARENTHESIZED_TRAIT
  }  // namespace detail
}  // namespace nitrate::compiler::parser
