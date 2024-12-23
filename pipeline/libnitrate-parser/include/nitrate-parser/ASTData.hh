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

#ifndef __NITRATE_AST_ASTDATA_H__
#define __NITRATE_AST_ASTDATA_H__

#include <boost/assert/source_location.hpp>
#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <optional>
#include <set>
#include <source_location>
#include <tuple>
#include <variant>
#include <vector>

namespace ncc::parse {
  extern thread_local std::unique_ptr<ncc::core::IMemory> npar_allocator;

  template <class T>
  struct Arena {
    using value_type = T;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(npar_allocator->alloc(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t n) {
      (void)n;
      (void)p;
    }
  };

  template <class T, class U>
  bool operator==(const Arena<T> &, const Arena<U> &) {
    return true;
  }
  template <class T, class U>
  bool operator!=(const Arena<T> &, const Arena<U> &) {
    return false;
  }

  static inline ncc::string SaveString(std::string_view str) { return str; }
};  // namespace ncc::parse

namespace ncc::parse {
#ifdef NDEBUG
#define NITRATE_AST_TRACKING 0
#else
#define NITRATE_AST_TRACKING 1
#endif

  template <class T>
  class RefNode {
    uintptr_t m_ptr = 0;

#if NITRATE_AST_TRACKING
    const char *m_origin_file_name, *m_origin_function_name;
    unsigned m_origin_line, m_origin_column;
#endif

    /* We could potentially embed tracking info for future debugging */

  public:
    using value_type = T;

    constexpr RefNode(
        T *ptr = nullptr
#if NITRATE_AST_TRACKING
        ,
        boost::source_location origin = std::source_location::current()
#endif
            )
        : m_ptr(reinterpret_cast<uintptr_t>(ptr)) {
#if NITRATE_AST_TRACKING
      m_origin_file_name = origin.file_name();
      m_origin_function_name = origin.function_name();
      m_origin_line = origin.line();
      m_origin_column = origin.column();
#endif
    }

    constexpr RefNode(const RefNode &other) {
      m_ptr = other.m_ptr;
#if NITRATE_AST_TRACKING
      m_origin_file_name = other.m_origin_file_name;
      m_origin_function_name = other.m_origin_function_name;
      m_origin_line = other.m_origin_line;
      m_origin_column = other.m_origin_column;
#endif
    }

    constexpr RefNode &operator=(const RefNode &other) {
      m_ptr = other.m_ptr;
#if NITRATE_AST_TRACKING
      m_origin_file_name = other.m_origin_file_name;
      m_origin_function_name = other.m_origin_function_name;
      m_origin_line = other.m_origin_line;
      m_origin_column = other.m_origin_column;
#endif

      return *this;
    }

    constexpr T *operator->() const { return reinterpret_cast<T *>(m_ptr); }
    constexpr T &operator*() const { return *reinterpret_cast<T *>(m_ptr); }
    constexpr T *get() const { return reinterpret_cast<T *>(m_ptr); }

    constexpr operator bool() const { return m_ptr != 0; }
    constexpr operator T *() const { return reinterpret_cast<T *>(m_ptr); }

    template <class U>
    constexpr operator RefNode<U>() const {
      return RefNode<U>(static_cast<U *>(get())
#if NITRATE_AST_TRACKING
                            ,
                        origin()
#endif
      );
    }

    constexpr boost::source_location origin() const {
#if NITRATE_AST_TRACKING
      return boost::source_location(m_origin_file_name, m_origin_line,
                                    m_origin_function_name, m_origin_column);
#else
      return boost::source_location();
#endif
    }

    constexpr void set_origin(std::source_location origin) {
#if NITRATE_AST_TRACKING
      m_origin_file_name = origin.file_name();
      m_origin_function_name = origin.function_name();
      m_origin_line = origin.line();
      m_origin_column = origin.column();
#else
      (void)origin;
#endif
    }

    template <class U>
    constexpr RefNode<U> as() const {
      return RefNode<U>(reinterpret_cast<U *>(get())
#if NITRATE_AST_TRACKING
                            ,
                        origin()
#endif
      );
    }

    ///======================================================================

    constexpr void accept(ASTVisitor &v) const {
      using namespace ncc::parse;

      switch (get()->getKind()) {
        case QAST_BASE: {
          v.visit(as<const Base>());
          break;
        }
        case QAST_BINEXPR: {
          v.visit(as<const BinExpr>());
          break;
        }
        case QAST_UNEXPR: {
          v.visit(as<const UnaryExpr>());
          break;
        }
        case QAST_TEREXPR: {
          v.visit(as<const TernaryExpr>());
          break;
        }
        case QAST_INT: {
          v.visit(as<const ConstInt>());
          break;
        }
        case QAST_FLOAT: {
          v.visit(as<const ConstFloat>());
          break;
        }
        case QAST_STRING: {
          v.visit(as<const ConstString>());
          break;
        }
        case QAST_CHAR: {
          v.visit(as<const ConstChar>());
          break;
        }
        case QAST_BOOL: {
          v.visit(as<const ConstBool>());
          break;
        }
        case QAST_NULL: {
          v.visit(as<const ConstNull>());
          break;
        }
        case QAST_UNDEF: {
          v.visit(as<const ConstUndef>());
          break;
        }
        case QAST_CALL: {
          v.visit(as<const Call>());
          break;
        }
        case QAST_LIST: {
          v.visit(as<const List>());
          break;
        }
        case QAST_ASSOC: {
          v.visit(as<const Assoc>());
          break;
        }
        case QAST_INDEX: {
          v.visit(as<const Index>());
          break;
        }
        case QAST_SLICE: {
          v.visit(as<const Slice>());
          break;
        }
        case QAST_FSTRING: {
          v.visit(as<const FString>());
          break;
        }
        case QAST_IDENT: {
          v.visit(as<const Ident>());
          break;
        }
        case QAST_SEQ: {
          v.visit(as<const SeqPoint>());
          break;
        }
        case QAST_POST_UNEXPR: {
          v.visit(as<const PostUnaryExpr>());
          break;
        }
        case QAST_SEXPR: {
          v.visit(as<const StmtExpr>());
          break;
        }
        case QAST_TEXPR: {
          v.visit(as<const TypeExpr>());
          break;
        }
        case QAST_TEMPL_CALL: {
          v.visit(as<const TemplCall>());
          break;
        }
        case QAST_REF: {
          v.visit(as<const RefTy>());
          break;
        }
        case QAST_U1: {
          v.visit(as<const U1>());
          break;
        }
        case QAST_U8: {
          v.visit(as<const U8>());
          break;
        }
        case QAST_U16: {
          v.visit(as<const U16>());
          break;
        }
        case QAST_U32: {
          v.visit(as<const U32>());
          break;
        }
        case QAST_U64: {
          v.visit(as<const U64>());
          break;
        }
        case QAST_U128: {
          v.visit(as<const U128>());
          break;
        }
        case QAST_I8: {
          v.visit(as<const I8>());
          break;
        }
        case QAST_I16: {
          v.visit(as<const I16>());
          break;
        }
        case QAST_I32: {
          v.visit(as<const I32>());
          break;
        }
        case QAST_I64: {
          v.visit(as<const I64>());
          break;
        }
        case QAST_I128: {
          v.visit(as<const I128>());
          break;
        }
        case QAST_F16: {
          v.visit(as<const F16>());
          break;
        }
        case QAST_F32: {
          v.visit(as<const F32>());
          break;
        }
        case QAST_F64: {
          v.visit(as<const F64>());
          break;
        }
        case QAST_F128: {
          v.visit(as<const F128>());
          break;
        }
        case QAST_VOID: {
          v.visit(as<const VoidTy>());
          break;
        }
        case QAST_PTR: {
          v.visit(as<const PtrTy>());
          break;
        }
        case QAST_OPAQUE: {
          v.visit(as<const OpaqueTy>());
          break;
        }
        case QAST_ARRAY: {
          v.visit(as<const ArrayTy>());
          break;
        }
        case QAST_TUPLE: {
          v.visit(as<const TupleTy>());
          break;
        }
        case QAST_FUNCTOR: {
          v.visit(as<const FuncTy>());
          break;
        }
        case QAST_NAMED: {
          v.visit(as<const NamedTy>());
          break;
        }
        case QAST_INFER: {
          v.visit(as<const InferTy>());
          break;
        }
        case QAST_TEMPLATE: {
          v.visit(as<const TemplType>());
          break;
        }
        case QAST_TYPEDEF: {
          v.visit(as<const TypedefStmt>());
          break;
        }
        case QAST_STRUCT: {
          v.visit(as<const StructDef>());
          break;
        }
        case QAST_ENUM: {
          v.visit(as<const EnumDef>());
          break;
        }
        case QAST_FUNCTION: {
          v.visit(as<const Function>());
          break;
        }
        case QAST_SCOPE: {
          v.visit(as<const ScopeStmt>());
          break;
        }
        case QAST_EXPORT: {
          v.visit(as<const ExportStmt>());
          break;
        }
        case QAST_BLOCK: {
          v.visit(as<const Block>());
          break;
        }
        case QAST_VAR: {
          v.visit(as<const VarDecl>());
          break;
        }
        case QAST_INLINE_ASM: {
          v.visit(as<const InlineAsm>());
          break;
        }
        case QAST_RETURN: {
          v.visit(as<const ReturnStmt>());
          break;
        }
        case QAST_RETIF: {
          v.visit(as<const ReturnIfStmt>());
          break;
        }
        case QAST_BREAK: {
          v.visit(as<const BreakStmt>());
          break;
        }
        case QAST_CONTINUE: {
          v.visit(as<const ContinueStmt>());
          break;
        }
        case QAST_IF: {
          v.visit(as<const IfStmt>());
          break;
        }
        case QAST_WHILE: {
          v.visit(as<const WhileStmt>());
          break;
        }
        case QAST_FOR: {
          v.visit(as<const ForStmt>());
          break;
        }
        case QAST_FOREACH: {
          v.visit(as<const ForeachStmt>());
          break;
        }
        case QAST_CASE: {
          v.visit(as<const CaseStmt>());
          break;
        }
        case QAST_SWITCH: {
          v.visit(as<const SwitchStmt>());
          break;
        }
        case QAST_ESTMT: {
          v.visit(as<const ExprStmt>());
          break;
        }
      }
    };
  } __attribute__((packed));

  using ExpressionList = std::vector<RefNode<Expr>, Arena<RefNode<Expr>>>;
  using TupleTyItems = std::vector<RefNode<Type>, Arena<RefNode<Type>>>;
  using CallArg = std::pair<ncc::string, RefNode<Expr>>;
  using CallArgs = std::vector<CallArg, Arena<CallArg>>;
  using FStringItem = std::variant<string, RefNode<Expr>>;
  using FStringItems = std::vector<FStringItem, Arena<FStringItem>>;

  using TemplateParameter =
      std::tuple<ncc::string, RefNode<Type>, RefNode<Expr>>;
  using TemplateParameters =
      std::vector<TemplateParameter, Arena<TemplateParameter>>;

  using BlockItems = std::vector<RefNode<Stmt>, Arena<RefNode<Stmt>>>;
  using ScopeDeps =
      std::set<ncc::string, std::less<ncc::string>, Arena<ncc::string>>;

  using SwitchCases = std::vector<RefNode<CaseStmt>, Arena<RefNode<CaseStmt>>>;
  using EnumItem = std::pair<ncc::string, RefNode<Expr>>;
  using EnumDefItems = std::vector<EnumItem, Arena<EnumItem>>;

  class StructField {
    ncc::string m_name;
    std::optional<RefNode<Expr>> m_value;
    RefNode<Type> m_type;
    Vis m_vis;
    bool m_is_static;

  public:
    StructField(Vis vis, bool is_static, ncc::string name, RefNode<Type> type,
                std::optional<RefNode<Expr>> value)
        : m_name(std::move(name)),
          m_value(std::move(value)),
          m_type(type),
          m_vis(vis),
          m_is_static(is_static) {}

    let get_vis() const { return m_vis; }
    let is_static() const { return m_is_static; }
    auto get_name() const { return m_name.get(); }
    let get_type() const { return m_type; }
    let get_value() const { return m_value; }
  };

  struct StructFunction {
    Vis vis;
    RefNode<Stmt> func;

    StructFunction(Vis vis, RefNode<Stmt> func) : vis(vis), func(func) {}
  };

  using StructDefFields = std::vector<StructField, Arena<StructField>>;
  using StructDefMethods = std::vector<StructFunction, Arena<StructFunction>>;
  using StructDefStaticMethods =
      std::vector<StructFunction, Arena<StructFunction>>;
  using StructDefNames = std::vector<ncc::string, Arena<ncc::string>>;

  using FuncParam = std::tuple<ncc::string, RefNode<Type>, RefNode<Expr>>;
  struct FuncParams {
    std::vector<FuncParam, Arena<FuncParam>> params;
    bool is_variadic;

    FuncParams() : is_variadic(false) {}
  };
  using FnCaptures = std::vector<std::pair<ncc::string, bool>,
                                 Arena<std::pair<ncc::string, bool>>>;
}  // namespace ncc::parse

#endif
