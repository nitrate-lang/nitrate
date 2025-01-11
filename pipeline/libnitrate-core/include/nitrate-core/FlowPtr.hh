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

#include <cstddef>
#include <cstdint>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <optional>
#include <source_location>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ncc {
  namespace trace {
    class origin {
      const char *m_file_name, *m_function_name;
      unsigned m_line, m_column;

    public:
      constexpr origin(
          std::source_location loc = std::source_location::current())
          : m_file_name(loc.file_name()),
            m_function_name(loc.function_name()),
            m_line(loc.line()),
            m_column(loc.column()) {}

      constexpr auto file_name() const { return m_file_name; }
      constexpr auto function_name() const { return m_function_name; }
      constexpr auto line() const { return m_line; }
      constexpr auto column() const { return m_column; }

      constexpr bool operator==(const origin &O) const {
        std::string_view a(m_file_name), b(O.m_file_name);
        std::string_view c(m_function_name), d(O.m_function_name);

        return m_line == O.m_line && m_column == O.m_column && a == b && c == d;
      }
    };

    class none {
    public:
      constexpr none() {}
      constexpr bool operator==(const none &) const { return true; }
    };

    static inline trace::none g_notrace_instance;
  }  // namespace trace

#if NITRATE_FLOWPTR_TRACE
  using DefaultTracking = trace::origin;
#else
  using DefaultTracking = trace::none;
#endif

  namespace flowptr_detail {
    template <class Pointee, class Tracking>
    struct WithTracking {
      union {
        Pointee *m_tptr;
        uintptr_t m_ptr;
      } m_ref;
      Tracking m_tracking;

      constexpr WithTracking(Pointee *ptr, Tracking tracking)
          : m_ref(ptr), m_tracking(std::move(tracking)) {}
    };

    template <class Pointee, class Tracking>
    struct WithoutTracking {
      union Ptr {
        Pointee *m_tptr;
        uintptr_t m_ptr;
      } m_ref;

      constexpr WithoutTracking(Pointee *ptr, Tracking) : m_ref(ptr) {}
    };

    template <class Pointee, class Tracking>
    using flowptr_data_t =
        std::conditional_t<std::is_same_v<Tracking, trace::none>,
                           WithoutTracking<Pointee, Tracking>,
                           WithTracking<Pointee, Tracking>>;

    template <class Pointee, class Tracking = DefaultTracking>
    class FlowPtr {
      using SelfData = flowptr_data_t<Pointee, Tracking>;

      SelfData m_s;

    public:
      using value_type = Pointee;

      ///=========================================================================
      /// Constructors

      constexpr FlowPtr(Tracking tracking = Tracking())
          : m_s(nullptr, std::move(tracking)) {}

      constexpr explicit FlowPtr(Pointee *ptr, Tracking tracking = Tracking())
          : m_s(ptr, std::move(tracking)) {}

      constexpr FlowPtr(std::nullptr_t, Tracking tracking = Tracking())
          : m_s(nullptr, std::move(tracking)) {}

      template <class U>
      constexpr FlowPtr(U *ptr, Tracking tracking = Tracking())
          : m_s(ptr, std::move(tracking)) {}

      constexpr FlowPtr(const FlowPtr &O) : m_s(O.m_s) {}
      constexpr FlowPtr(FlowPtr &&O) : m_s(std::move(O.m_s)) {}

      constexpr FlowPtr &operator=(const FlowPtr &O) {
        m_s = O.m_s;
        return *this;
      }

      constexpr FlowPtr &operator=(FlowPtr &&O) {
        m_s = std::move(O.m_s);
        return *this;
      }

      constexpr ~FlowPtr() = default;

      ///=========================================================================
      /// Comparison

      constexpr bool operator==(const FlowPtr &O) const {
        return m_s.m_ref.m_ptr == O.m_s.m_ref.m_ptr;
      }

      constexpr bool operator!=(const FlowPtr &O) const {
        return m_s.m_ref.m_ptr != O.m_s.m_ref.m_ptr;
      }

      constexpr bool operator==(std::nullptr_t) const {
        return m_s.m_ref.m_ptr == 0;
      }

      ///=========================================================================
      /// Accessors

      constexpr auto operator->() const { return m_s.m_ref.m_tptr; }
      constexpr auto get() const { return m_s.m_ref.m_tptr; }
      constexpr operator bool() const { return m_s.m_ref.m_ptr != 0; }

      ///=========================================================================
      /// Casting

      template <class U>
      constexpr operator FlowPtr<U>() {
        return FlowPtr<U>(static_cast<U *>(get()), trace());
      }

      template <class U>
      constexpr auto as() {
        return FlowPtr<U>(reinterpret_cast<U *>(get()), trace());
      }

      ///=========================================================================
      /// Data-Flow tracking

      constexpr const Tracking &trace() const {
        if constexpr (std::is_same_v<SelfData,
                                     WithTracking<Pointee, Tracking>>) {
          return m_s.m_tracking;
        } else {
          return trace::g_notrace_instance;
        }
      }

      constexpr void set_tracking(Tracking tracking) {
        if constexpr (std::is_same_v<SelfData,
                                     WithTracking<Pointee, Tracking>>) {
          m_s.m_tracking = std::move(tracking);
        }
      }

      ///=========================================================================
      /// Visitor pass-through

      template <class Vistor>
      constexpr void accept(Vistor &v) {
        v.dispatch(*this);
      }
    };

    template <class Pointee, class Tracking = DefaultTracking>
    class NullableFlowPtr {
      FlowPtr<Pointee, Tracking> m_ptr;

    public:
      using value_type = Pointee;

      constexpr NullableFlowPtr() : m_ptr(nullptr) {}

      constexpr explicit NullableFlowPtr(FlowPtr<Pointee, Tracking> O)
          : m_ptr(O) {}

      template <class U>
      constexpr NullableFlowPtr(U *ptr, Tracking tracking = Tracking())
          : m_ptr(ptr, std::move(tracking)) {}

      constexpr NullableFlowPtr(std::nullopt_t, Tracking tracking = Tracking())
          : m_ptr(nullptr, std::move(tracking)) {}

      constexpr NullableFlowPtr(std::nullptr_t, Tracking tracking = Tracking())
          : m_ptr(nullptr, std::move(tracking)) {}

      constexpr NullableFlowPtr(std::optional<FlowPtr<Pointee, Tracking>> O)
          : m_ptr(O.value_or(nullptr)) {}

      template <class U = Pointee>
      constexpr NullableFlowPtr(FlowPtr<U, Tracking> O) : m_ptr(O) {}

      template <class U>
      constexpr NullableFlowPtr(const NullableFlowPtr<U, Tracking> &O)
          : m_ptr(O) {}

      template <class U>
      constexpr NullableFlowPtr(NullableFlowPtr<U, Tracking> &&O)
          : m_ptr(std::move(O)) {}

      template <class U>
      constexpr NullableFlowPtr &operator=(
          const NullableFlowPtr<U, Tracking> &O) {
        m_ptr = O;
        return *this;
      }

      template <class U>
      constexpr NullableFlowPtr &operator=(NullableFlowPtr<U, Tracking> &&O) {
        m_ptr = std::move(O);
        return *this;
      }

      constexpr bool operator==(const NullableFlowPtr &O) const {
        return m_ptr == O.m_ptr;
      }

      constexpr bool operator!=(const NullableFlowPtr &O) const {
        return m_ptr != O.m_ptr;
      }

      constexpr bool operator==(std::nullptr_t) const { return !m_ptr; }

      constexpr operator bool() const { return m_ptr; }

      constexpr bool has_value() const { return m_ptr != nullptr; }

      constexpr FlowPtr<Pointee, Tracking> &value() {
        if (!has_value()) [[unlikely]] {
          qcore_panicf("Attempted to dereference a nullptr. this=%p", this);
        }

        return m_ptr;
      }

      constexpr const FlowPtr<Pointee, Tracking> &value() const {
        if (!has_value()) [[unlikely]] {
          qcore_panicf("Attempted to dereference a nullptr. this=%p", this);
        }

        return m_ptr;
      }

      template <class U>
      constexpr Pointee *value_or(U &&default_value) const {
        return m_ptr ? m_ptr.get() : std::forward<U>(default_value);
      }
    };

    constexpr auto FlowPtrStructSize = sizeof(FlowPtr<int>);
    constexpr auto NullableFlowPtrStructSize = sizeof(NullableFlowPtr<int>);
    static_assert(sizeof(FlowPtr<int>) == sizeof(NullableFlowPtr<int>));

  }  // namespace flowptr_detail

  template <class Pointee, class Tracking = DefaultTracking>
  using FlowPtr = flowptr_detail::FlowPtr<Pointee, Tracking>;

  template <class Pointee, class Tracking = DefaultTracking>
  using NullableFlowPtr = flowptr_detail::NullableFlowPtr<Pointee, Tracking>;

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr FlowPtr<Pointee, Tracking> MakeFlowPtr(
      Pointee *ptr, Tracking tracking = Tracking()) {
    return FlowPtr<Pointee, Tracking>(ptr, std::move(tracking));
  }
}  // namespace ncc

namespace std {
  template <class Pointee, class Tracking>
  struct hash<ncc::FlowPtr<Pointee, Tracking>> {
    size_t operator()(const ncc::FlowPtr<Pointee, Tracking> &ptr) const {
      return std::hash<Pointee *>()(ptr.get());
    }
  };

  template <class Pointee, class Tracking>
  struct hash<ncc::NullableFlowPtr<Pointee, Tracking>> {
    size_t operator()(
        const ncc::NullableFlowPtr<Pointee, Tracking> &ptr) const {
      return std::hash<Pointee *>()(ptr.value_or(nullptr));
    }
  };
}  // namespace std

#endif
