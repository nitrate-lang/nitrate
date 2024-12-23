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

#ifndef __NITRATE_CORE_FLOWPTR_H__
#define __NITRATE_CORE_FLOWPTR_H__

#include <boost/assert/source_location.hpp>
#include <nitrate-core/String.hh>
#include <source_location>

namespace ncc {
#ifdef NDEBUG
#define NITRATE_AST_TRACKING 0
#else
#define NITRATE_AST_TRACKING 1
#endif

  namespace dataflow {
    class FlowPtrTracking {};
  }  // namespace dataflow

  template <class T>
  class FlowPtr {
    union Ptr {
      T *m_tptr;
      uintptr_t m_ptr;
    } m_ref;

#if NITRATE_AST_TRACKING
    const char *m_origin_file_name, *m_origin_function_name;
    unsigned m_origin_line, m_origin_column;
#endif

    /* We could potentially embed tracking info for future debugging */

  public:
    using value_type = T;

    explicit constexpr FlowPtr(
        T *ptr = nullptr
#if NITRATE_AST_TRACKING
        ,
        boost::source_location origin = std::source_location::current()
#endif
            )
        : m_ref(ptr) {
#if NITRATE_AST_TRACKING
      m_origin_file_name = origin.file_name();
      m_origin_function_name = origin.function_name();
      m_origin_line = origin.line();
      m_origin_column = origin.column();
#endif
    }

    constexpr FlowPtr(
        std::nullptr_t
#if NITRATE_AST_TRACKING
        ,
        boost::source_location origin = std::source_location::current()
#endif
            )
        : m_ref(nullptr) {
#if NITRATE_AST_TRACKING
      m_origin_file_name = origin.file_name();
      m_origin_function_name = origin.function_name();
      m_origin_line = origin.line();
      m_origin_column = origin.column();
#endif
    }

    constexpr FlowPtr(const FlowPtr &other) {
      m_ref = other.m_ref;
#if NITRATE_AST_TRACKING
      m_origin_file_name = other.m_origin_file_name;
      m_origin_function_name = other.m_origin_function_name;
      m_origin_line = other.m_origin_line;
      m_origin_column = other.m_origin_column;
#endif
    }

    constexpr FlowPtr &operator=(const FlowPtr &other) {
      m_ref = other.m_ref;
#if NITRATE_AST_TRACKING
      m_origin_file_name = other.m_origin_file_name;
      m_origin_function_name = other.m_origin_function_name;
      m_origin_line = other.m_origin_line;
      m_origin_column = other.m_origin_column;
#endif

      return *this;
    }

    ///=========================================================================
    /// Accessors

    constexpr T *operator->() const { return m_ref.m_tptr; }
    constexpr T *get() const { return m_ref.m_tptr; }
    constexpr operator bool() const { return m_ref.m_ptr != 0; }
    constexpr operator T *() const { return m_ref.m_tptr; }

    ///=========================================================================
    /// Casting

    template <class U>
    constexpr operator FlowPtr<U>() const {
      return FlowPtr<U>(static_cast<U *>(get())
#if NITRATE_AST_TRACKING
                            ,
                        origin()
#endif
      );
    }

    template <class U>
    constexpr FlowPtr<U> as() const {
      return FlowPtr<U>(reinterpret_cast<U *>(get())
#if NITRATE_AST_TRACKING
                            ,
                        origin()
#endif
      );
    }

    ///=========================================================================
    /// Data-Flow tracking

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

    ///=========================================================================
    /// Visitor pass-through

    template <class BaseVistor>
    void accept(BaseVistor &v) const {
      v.dispatch(*this);
    }

    ///=========================================================================
    /// Reflection pass-through
    auto getKind() const { return get()->getKind(); }
  } __attribute__((packed));

  template <class T>
  constexpr FlowPtr<T> MakeFlowPtr(T *ptr) {
    return FlowPtr<T>(ptr);
  }
}  // namespace ncc

#endif  // __NITRATE_CORE_CACHE_H__
