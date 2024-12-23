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

#include <cstdarg>
#include <cstdint>
#include <source_location>
#include <type_traits>
#include <utility>

namespace ncc {
#ifdef NDEBUG
#define NITRATE_AST_TRACKING 0
#else
#define NITRATE_AST_TRACKING 1
#endif

  namespace trace {
    enum class DataFlowEvent {
      Construct_FromPtr,
      Construct_FromNull,
      Construct_Copy,
      Construct_Move,

      Assign_Copy,
      Assign_Move,

      Cast_ToRaw,
      Cast_ToBool,
      Cast_Static,
      Cast_Reinterpret,

      Compare_Equal,
      Compare_NotEqual,
      Compare_Less,
      Compare_LessEqual,
      Compare_Greater,
      Compare_GreaterEqual,

      Access_Arrow,
      Access_Get,

      Visitor_Accept,

      Reflection_GetKind,

      Tracking_Get,
      Tracking_Set,
    };

    class genesis {
      const char *m_file_name, *m_function_name;
      unsigned m_line, m_column;

    public:
      constexpr genesis(
          std::source_location loc = std::source_location::current())
          : m_file_name(loc.file_name()),
            m_function_name(loc.function_name()),
            m_line(loc.line()),
            m_column(loc.column()) {}

      constexpr auto file_name() const { return m_file_name; }
      constexpr auto function_name() const { return m_function_name; }
      constexpr auto line() const { return m_line; }
      constexpr auto column() const { return m_column; }

      constexpr bool operator==(const genesis &other) const {
        return m_file_name == other.m_file_name &&
               m_function_name == other.m_function_name &&
               m_line == other.m_line && m_column == other.m_column;
      }

      genesis dispatch(DataFlowEvent, va_list) const {
        /// This tracking method does not track events
        return *this;
      }
    } __attribute__((packed));

    class verbose {
    public:
      constexpr verbose() {}
      constexpr verbose(const verbose &) = default;

      constexpr bool operator==(const verbose &) const { return true; }

      verbose dispatch(DataFlowEvent e, va_list va) const {
        (void)e;
        (void)va;
        /// TODO: Implement event tracking

        return *this;
      }
    } __attribute__((packed));

    class none {
    public:
      constexpr none() {}

      constexpr bool operator==(const none &) const { return true; }

      none dispatch(DataFlowEvent, va_list) const { return *this; }
    } __attribute__((packed));

    static inline trace::none g_notrace_instance;
  }  // namespace trace

#if NITRATE_AST_TRACKING
  using DefaultTracking = trace::genesis;
#else
  using DefaultTracking = trace::none;
#endif

  template <class Pointee, class Tracking = DefaultTracking>
  class FlowPtr {
    struct WithTracking {
      union Ptr {
        Pointee *m_tptr;
        uintptr_t m_ptr;
      } m_ref;

      Tracking m_tracking;

      ///=========================================================================

      constexpr WithTracking() : m_ref(nullptr), m_tracking() {}
      constexpr WithTracking(Pointee *ptr, Tracking tracking)
          : m_ref(ptr), m_tracking(std::move(tracking)) {}

      constexpr void dispatch(trace::DataFlowEvent e, va_list va) {
        m_tracking = m_tracking.dispatch(e, va);
      }
    } __attribute__((packed));

    struct WithoutTracking {
      union Ptr {
        Pointee *m_tptr;
        uintptr_t m_ptr;
      } m_ref;

      constexpr WithoutTracking() : m_ref(nullptr) {}
      constexpr WithoutTracking(Pointee *ptr, Tracking) : m_ref(ptr) {}

      constexpr void dispatch(trace::DataFlowEvent, va_list) {}
    } __attribute__((packed));

    using Kernel = std::conditional_t<std::is_same_v<Tracking, trace::none>,
                                      WithoutTracking, WithTracking>;

    Kernel m_s;

    constexpr void dispatch(trace::DataFlowEvent e, ...) const {
      va_list va;
      va_start(va, e);

      const_cast<FlowPtr *>(this)->m_s.dispatch(e, va);

      va_end(va);
    }

  public:
    using value_type = Pointee;

    ///=========================================================================
    /// Constructors

    constexpr explicit FlowPtr(Pointee *ptr = nullptr,
                               Tracking tracking = Tracking())
        : m_s(ptr, std::move(tracking)) {
      dispatch(trace::DataFlowEvent::Construct_FromPtr);
    }

    constexpr FlowPtr(std::nullptr_t, Tracking tracking = Tracking())
        : m_s(nullptr, std::move(tracking)) {}

    constexpr FlowPtr(const FlowPtr &other) { m_s = std::move(other.m_s); }
    constexpr FlowPtr(FlowPtr &&other) { m_s = other.m_s; }

    constexpr FlowPtr &operator=(const FlowPtr &other) {
      m_s = other.m_s;
      return *this;
    }

    constexpr FlowPtr &operator=(FlowPtr &&other) {
      m_s = std::move(other.m_s);
      return *this;
    }

    ///=========================================================================
    /// Helpers

    constexpr bool operator==(const FlowPtr &other) const {
      return m_s.m_ref.m_ptr == other.m_s.m_ref.m_ptr;
    }

    constexpr bool operator!=(const FlowPtr &other) const {
      return m_s.m_ref.m_ptr != other.m_s.m_ref.m_ptr;
    }

    constexpr bool operator==(std::nullptr_t) const {
      return m_s.m_ref.m_ptr == 0;
    }

    ///=========================================================================
    /// Accessors

    constexpr auto operator->() const { return m_s.m_ref.m_tptr; }
    constexpr auto get() const { return m_s.m_ref.m_tptr; }
    constexpr operator bool() const { return m_s.m_ref.m_ptr != 0; }
    constexpr operator auto() const { return m_s.m_ref.m_tptr; }

    ///=========================================================================
    /// Casting

    template <class U>
    constexpr operator FlowPtr<U>() const {
      return FlowPtr<U>(static_cast<U *>(get()), trace());
    }

    template <class U>
    constexpr FlowPtr<U> as() const {
      return FlowPtr<U>(reinterpret_cast<U *>(get()), trace());
    }

    ///=========================================================================
    /// Data-Flow tracking

    constexpr const Tracking &trace() const {
      if constexpr (std::is_same_v<Kernel, WithTracking>) {
        return m_s.m_tracking;
      } else {
        return trace::g_notrace_instance;
      }
    }

    constexpr void set_tracking(Tracking tracking) {
      if constexpr (std::is_same_v<Kernel, WithTracking>) {
        m_s.m_tracking = std::move(tracking);
      }
    }

    ///=========================================================================
    /// Visitor pass-through

    template <class Vistor>
    constexpr void accept(Vistor &v) const {
      v.dispatch(*this);
    }

    ///=========================================================================
    /// Reflection pass-through
    constexpr auto getKind() const { return get()->getKind(); }
  } __attribute__((packed));

  constexpr auto FlowPtrStructSize = sizeof(FlowPtr<int>);

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr FlowPtr<Pointee, Tracking> MakeFlowPtr(
      Pointee *ptr, Tracking tracking = Tracking()) {
    return FlowPtr<Pointee, Tracking>(ptr, std::move(tracking));
  }
}  // namespace ncc

#endif
