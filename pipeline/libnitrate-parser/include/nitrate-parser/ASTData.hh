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

#ifndef __NITRATE_PARSER_ASTDATA_H__
#define __NITRATE_PARSER_ASTDATA_H__

#ifndef __cplusplus
#error "This code requires c++"
#endif

#include <nitrate-core/Macro.h>

#include <map>
#include <nitrate-core/Classes.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <set>
#include <tuple>
#include <variant>
#include <vector>

namespace npar {
  class ArenaAllocatorImpl {
    qcore_arena m_arena;

  public:
    ArenaAllocatorImpl() = default;

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr);

    void swap(qcore_arena_t &arena);

    qcore_arena_t &get() { return *m_arena.get(); }
  };

  extern thread_local ArenaAllocatorImpl npar_arena;

  template <class T>
  struct Arena {
    using value_type = T;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(npar_arena.allocate(sizeof(T) * n));
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

  SmallString SaveString(std::string_view str);
};  // namespace npar

namespace npar {
  using ExpressionList = std::vector<Expr *, Arena<Expr *>>;

  using TupleTyItems = std::vector<Type *, Arena<Type *>>;

  using CallArg = std::pair<SmallString, Expr *>;
  using CallArgs = std::vector<CallArg, Arena<CallArg>>;

  using FStringItems = std::vector<std::variant<SmallString, Expr *>,
                                   Arena<std::variant<SmallString, Expr *>>>;

  using TemplateArgs = std::map<SmallString, Expr *, std::less<SmallString>,
                                Arena<std::pair<const SmallString, Expr *>>>;
  using TemplateParameter = std::tuple<SmallString, Type *, Expr *>;
  using TemplateParameters =
      std::vector<TemplateParameter, Arena<TemplateParameter>>;

  using BlockItems = std::vector<Stmt *, Arena<Stmt *>>;
  using ScopeDeps =
      std::set<SmallString, std::less<SmallString>, Arena<SmallString>>;

  using VarDeclAttributes = std::set<Expr *, std::less<Expr *>, Arena<Expr *>>;

  using SwitchCases = std::vector<CaseStmt *, Arena<CaseStmt *>>;
  using SymbolAttributes = std::set<Expr *, std::less<Expr *>, Arena<Expr *>>;
  using EnumItem = std::pair<SmallString, Expr *>;
  using EnumDefItems = std::vector<EnumItem, Arena<EnumItem>>;

  class StructField {
    Vis m_vis;
    SmallString m_name;
    Type *m_type;
    Expr *m_value;

  public:
    StructField(Vis vis, SmallString name, Type *type, Expr *value)
        : m_vis(vis), m_name(name), m_type(type), m_value(value) {}

    let get_vis() const { return m_vis; }
    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
    let get_value() const { return m_value; }
  };
  using StructDefFields = std::vector<StructField, Arena<StructField>>;
  using StructDefMethods = std::vector<FnDef *, Arena<FnDef *>>;
  using StructDefStaticMethods = std::vector<FnDef *, Arena<FnDef *>>;

  using FuncParam = std::tuple<SmallString, Type *, Expr *>;
  using FuncParams = std::vector<FuncParam, Arena<FuncParam>>;
  using FnCaptures = std::vector<std::pair<SmallString, bool>,
                                 Arena<std::pair<SmallString, bool>>>;
}  // namespace npar

#endif
