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

#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <set>
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

  static inline ncc::core::str_alias SaveString(std::string_view str) {
    return ncc::core::StringMemory::Get(str);
  }
};  // namespace ncc::parse

namespace ncc::parse {
  using ExpressionList = std::vector<Expr *, Arena<Expr *>>;

  using TupleTyItems = std::vector<Type *, Arena<Type *>>;

  using CallArg = std::pair<ncc::core::str_alias, Expr *>;
  using CallArgs = std::vector<CallArg, Arena<CallArg>>;

  using FStringItems =
      std::vector<std::variant<ncc::core::str_alias, Expr *>,
                  Arena<std::variant<ncc::core::str_alias, Expr *>>>;

  using TemplateParameter = std::tuple<ncc::core::str_alias, Type *, Expr *>;
  using TemplateParameters =
      std::vector<TemplateParameter, Arena<TemplateParameter>>;

  using BlockItems = std::vector<Stmt *, Arena<Stmt *>>;
  using ScopeDeps =
      std::set<ncc::core::str_alias, std::less<ncc::core::str_alias>,
               Arena<ncc::core::str_alias>>;

  using SwitchCases = std::vector<CaseStmt *, Arena<CaseStmt *>>;
  using EnumItem = std::pair<ncc::core::str_alias, Expr *>;
  using EnumDefItems = std::vector<EnumItem, Arena<EnumItem>>;

  class StructField {
    ncc::core::str_alias m_name;
    std::optional<Expr *> m_value;
    Type *m_type;
    Vis m_vis;
    bool m_is_static;

  public:
    StructField(Vis vis, bool is_static, ncc::core::str_alias name, Type *type,
                std::optional<Expr *> value)
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
    Stmt *func;

    StructFunction(Vis vis, Stmt *func) : vis(vis), func(func) {}
  };

  using StructDefFields = std::vector<StructField, Arena<StructField>>;
  using StructDefMethods = std::vector<StructFunction, Arena<StructFunction>>;
  using StructDefStaticMethods =
      std::vector<StructFunction, Arena<StructFunction>>;
  using StructDefNames =
      std::vector<ncc::core::str_alias, Arena<ncc::core::str_alias>>;

  using FuncParam = std::tuple<ncc::core::str_alias, Type *, Expr *>;
  struct FuncParams {
    std::vector<FuncParam, Arena<FuncParam>> params;
    bool is_variadic;

    FuncParams() : is_variadic(false) {}
  };
  using FnCaptures = std::vector<std::pair<ncc::core::str_alias, bool>,
                                 Arena<std::pair<ncc::core::str_alias, bool>>>;
}  // namespace ncc::parse

#endif
