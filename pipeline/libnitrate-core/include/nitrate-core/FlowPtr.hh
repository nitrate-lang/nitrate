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
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ncc {
#ifdef NDEBUG
#define NITRATE_AST_TRACKING 0
#else
#define NITRATE_AST_TRACKING 1
#endif

  namespace trace {
    enum class DataFlowEvent {
      Construct_FromNull,

      Cast_ToRaw,
      Cast_Reinterpret,

      Visitor_Accept,

      Reflection_GetKind,

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

      constexpr bool operator==(const genesis &O) const {
        std::string_view a(m_file_name), b(O.m_file_name);
        std::string_view c(m_function_name), d(O.m_function_name);

        return m_line == O.m_line && m_column == O.m_column && a == b && c == d;
      }

      genesis dispatch(DataFlowEvent, std::source_location) {
        /// This tracking method does not track events
        return *this;
      }
    };

    static inline bool operator==(const std::source_location &a,
                                  const std::source_location &b) {
      return a.file_name() == b.file_name() &&
             a.function_name() == b.function_name() && a.line() == b.line() &&
             a.column() == b.column();
    }

    struct Event {
      DataFlowEvent m_event;
      std::source_location m_loc;

      constexpr bool operator==(const Event &O) const {
        return m_event == O.m_event && m_loc == O.m_loc;
      }
    };

    using EventList = std::vector<Event>;

    class verbose {
      const char *m_file_name, *m_function_name;
      unsigned m_line, m_column;
      EventList m_events;

    public:
      constexpr verbose(
          std::source_location loc = std::source_location::current())
          : m_file_name(loc.file_name()),
            m_function_name(loc.function_name()),
            m_line(loc.line()),
            m_column(loc.column()) {
        m_events = {};

        qcore_panic("Verbose tracking is not implemented yet");
      }

      constexpr auto file_name() const { return m_file_name; }
      constexpr auto function_name() const { return m_function_name; }
      constexpr auto line() const { return m_line; }
      constexpr auto column() const { return m_column; }
      constexpr std::span<const Event> events() const { return m_events; }

      bool operator==(const verbose &O) const {
        std::string_view a(m_file_name), b(O.m_file_name);
        std::string_view c(m_function_name), d(O.m_function_name);

        return m_line == O.m_line && m_column == O.m_column && a == b &&
               c == d && m_events == O.m_events;

        return true;
      }

      verbose dispatch(DataFlowEvent e, std::source_location loc) {
        /// TODO: Debug this UAF bug??

        m_events.push_back({e, loc});
        return *this;
      }
    };

    class none {
    public:
      constexpr none() {}

      constexpr bool operator==(const none &) const { return true; }

      none dispatch(DataFlowEvent, std::source_location) { return *this; }
    };

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

      constexpr void dispatch(trace::DataFlowEvent e,
                              std::source_location loc) {
        m_tracking = m_tracking.dispatch(e, loc);
      }
    };

    struct WithoutTracking {
      union Ptr {
        Pointee *m_tptr;
        uintptr_t m_ptr;
      } m_ref;

      constexpr WithoutTracking() : m_ref(nullptr) {}
      constexpr WithoutTracking(Pointee *ptr, Tracking) : m_ref(ptr) {}

      constexpr void dispatch(trace::DataFlowEvent, std::source_location) {}
    };

    using Kernel = std::conditional_t<std::is_same_v<Tracking, trace::none>,
                                      WithoutTracking, WithTracking>;

    Kernel m_s;

    constexpr void publish(trace::DataFlowEvent e, std::source_location loc) {
      m_s.dispatch(e, loc);
    }

  public:
    using value_type = Pointee;

    ///=========================================================================
    /// Constructors

    constexpr explicit FlowPtr(Pointee *ptr = nullptr,
                               Tracking tracking = Tracking())
        : m_s(ptr, std::move(tracking)) {}

    constexpr FlowPtr(
        std::nullptr_t, Tracking tracking = Tracking(),
        std::source_location loc = std::source_location::current())
        : m_s(nullptr, std::move(tracking)) {
      publish(trace::DataFlowEvent::Construct_FromNull, loc);
    }

    template <class U>
    constexpr FlowPtr(U *ptr, Tracking tracking = Tracking())
        : m_s(ptr, std::move(tracking)) {}

    constexpr FlowPtr(const FlowPtr &O) { m_s = std::move(O.m_s); }

    constexpr FlowPtr(FlowPtr &&O) { m_s = O.m_s; }

    constexpr FlowPtr &operator=(const FlowPtr &O) {
      m_s = O.m_s;

      return *this;
    }

    constexpr FlowPtr &operator=(FlowPtr &&O) {
      m_s = std::move(O.m_s);

      return *this;
    }

    ///=========================================================================
    /// Helpers

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

    constexpr auto get(
        std::source_location loc = std::source_location::current()) {
      publish(trace::DataFlowEvent::Visitor_Accept, loc);

      return m_s.m_ref.m_tptr;
    }

    constexpr operator bool() const { return m_s.m_ref.m_ptr != 0; }

    ///=========================================================================
    /// Casting

    template <class U>
    constexpr operator FlowPtr<U>() {
      return FlowPtr<U>(static_cast<U *>(get()), trace());
    }

    template <class U>
    constexpr FlowPtr<U> as(
        std::source_location loc = std::source_location::current()) {
      publish(trace::DataFlowEvent::Cast_Reinterpret, loc);

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

    constexpr void set_tracking(
        Tracking tracking,
        std::source_location loc = std::source_location::current()) {
      publish(trace::DataFlowEvent::Tracking_Set, loc);

      if constexpr (std::is_same_v<Kernel, WithTracking>) {
        m_s.m_tracking = std::move(tracking);
      }
    }

    ///=========================================================================
    /// Visitor pass-through

    template <class Vistor>
    constexpr void accept(
        Vistor &v, std::source_location loc = std::source_location::current()) {
      publish(trace::DataFlowEvent::Visitor_Accept, loc);

      v.dispatch(*this);
    }

    ///=========================================================================
    /// Reflection pass-through
    constexpr auto getKind(
        std::source_location loc = std::source_location::current()) {
      publish(trace::DataFlowEvent::Reflection_GetKind, loc);

      return get()->getKind();
    }
  };

  constexpr auto FlowPtrStructSize = sizeof(FlowPtr<int>);

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr FlowPtr<Pointee, Tracking> MakeFlowPtr(
      Pointee *ptr, Tracking tracking = Tracking()) {
    return FlowPtr<Pointee, Tracking>(ptr, std::move(tracking));
  }

  template <class Pointee, class Tracking = DefaultTracking>
  class NullableFlowPtr {
    FlowPtr<Pointee, Tracking> m_ptr;

  public:
    using value_type = Pointee;

    constexpr NullableFlowPtr() : m_ptr(nullptr) {}

    constexpr explicit NullableFlowPtr(FlowPtr<Pointee, Tracking> O,
                                       Tracking tracking = Tracking())
        : m_ptr(O) {
      m_ptr.set_tracking(tracking);
    }

    constexpr NullableFlowPtr(
        std::nullopt_t, Tracking tracking = Tracking(),
        std::source_location loc = std::source_location::current())
        : m_ptr(nullptr, std::move(tracking), loc) {}

    constexpr NullableFlowPtr(
        std::nullptr_t, Tracking tracking = Tracking(),
        std::source_location loc = std::source_location::current())
        : m_ptr(nullptr, std::move(tracking), loc) {}

    template <class U = Pointee>
    constexpr NullableFlowPtr(FlowPtr<U, Tracking> O,
                              Tracking tracking = Tracking())
        : m_ptr(O) {
      m_ptr.set_tracking(tracking);
    }

    template <class U>
    constexpr NullableFlowPtr(const NullableFlowPtr<U, Tracking> &O,
                              Tracking tracking = Tracking())
        : m_ptr(O) {
      m_ptr.set_tracking(tracking);
    }

    template <class U>
    constexpr NullableFlowPtr(NullableFlowPtr<U, Tracking> &&O,
                              Tracking tracking = Tracking())
        : m_ptr(std::move(O)) {
      m_ptr.set_tracking(tracking);
    }

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
    constexpr Pointee *value_or(U &&default_value) {
      return m_ptr ? m_ptr.get() : std::forward<U>(default_value);
    }
  };

  constexpr auto NullableFlowPtrStructSize = sizeof(NullableFlowPtr<int>);

  static_assert(sizeof(FlowPtr<int>) == sizeof(NullableFlowPtr<int>));

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr NullableFlowPtr<Pointee, Tracking> MakeNullableFlowPtr(
      Pointee *ptr, Tracking tracking = Tracking()) {
    return NullableFlowPtr<Pointee, Tracking>(ptr, std::move(tracking));
  }
}  // namespace ncc

#endif
