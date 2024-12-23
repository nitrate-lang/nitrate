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

#include <boost/flyweight.hpp>
#include <boost/flyweight/no_tracking.hpp>
#include <nitrate-core/String.hh>
#include <source_location>
#include <type_traits>

namespace ncc {
#ifdef NDEBUG
#define NITRATE_AST_TRACKING 0
#else
#define NITRATE_AST_TRACKING 1
#endif

  namespace dataflow {
    class GenesisTrace {
      const char *m_file_name, *m_function_name;
      unsigned m_line, m_column;

    public:
      constexpr GenesisTrace(
          std::source_location loc = std::source_location::current())
          : m_file_name(loc.file_name()),
            m_function_name(loc.function_name()),
            m_line(loc.line()),
            m_column(loc.column()) {}

      constexpr auto file_name() const { return m_file_name; }
      constexpr auto function_name() const { return m_function_name; }
      constexpr auto line() const { return m_line; }
      constexpr auto column() const { return m_column; }

      constexpr bool operator==(const GenesisTrace &other) const {
        return m_file_name == other.m_file_name &&
               m_function_name == other.m_function_name &&
               m_line == other.m_line && m_column == other.m_column;
      }
    } __attribute__((packed));

    class VerboseTrace {
    public:
      constexpr VerboseTrace() {}

      constexpr bool operator==(const VerboseTrace &) const { return true; }
    } __attribute__((packed));

    class NoTrace {
    public:
      constexpr NoTrace() {}

      constexpr bool operator==(const NoTrace &) const { return true; }
    } __attribute__((packed));

    static constexpr inline std::size_t hash_value(const GenesisTrace &trace) {
      return std::hash<const char *>()(trace.file_name()) ^
             std::hash<const char *>()(trace.function_name()) ^
             std::hash<unsigned>()(trace.line()) ^
             std::hash<unsigned>()(trace.column());
    }

    static constexpr inline std::size_t hash_value(const VerboseTrace &) {
      return 0;
    }

    static constexpr inline std::size_t hash_value(const NoTrace &) {
      return 0;
    }

    static inline dataflow::NoTrace g_notrace_instance;
  }  // namespace dataflow

#if NITRATE_AST_TRACKING
  using DefaultTracking = dataflow::GenesisTrace;
#else
  using DefaultTracking = dataflow::NoTrace;
#endif

  template <class Pointee, class Tracking = DefaultTracking>
  class FlowPtr {
    struct WithTracking {
      union Ptr {
        Pointee *m_tptr;
        uintptr_t m_ptr;
      } m_ref;

      FlowPtr<const Tracking, dataflow::NoTrace> m_tracking;

      ///=========================================================================

      constexpr static inline FlowPtr<const Tracking, dataflow::NoTrace>
      FlyweightFromValue(Tracking tracking) {
        // Basically use boost::flyweight as a generic object interner
        // Don't deallocate the object when the last reference is gone
        // just leak it. It will be cleaned up when the program exits.
        // This is meant to be used in debug mode, so this is an acceptable
        // method.

        return FlowPtr<const Tracking, dataflow::NoTrace>(
            &boost::flyweight<Tracking, boost::flyweights::no_tracking>(
                 std::move(tracking))
                 .get());
      }

      constexpr WithTracking() : m_ref(nullptr), m_tracking() {}
      constexpr WithTracking(Pointee *ptr, Tracking tracking)
          : m_ref(ptr),
            m_tracking(WithTracking::FlyweightFromValue(std::move(tracking))) {}
    } __attribute__((packed));

    struct WithoutTracking {
      union Ptr {
        Pointee *m_tptr;
        uintptr_t m_ptr;
      } m_ref;

      constexpr WithoutTracking() : m_ref(nullptr) {}
      constexpr WithoutTracking(Pointee *ptr, Tracking) : m_ref(ptr) {}
    } __attribute__((packed));

    using Kernel =
        std::conditional_t<std::is_same_v<Tracking, dataflow::NoTrace>,
                           WithoutTracking, WithTracking>;

    Kernel m_s;

  public:
    using value_type = Pointee;

    ///=========================================================================
    /// Constructors

    constexpr explicit FlowPtr(Pointee *ptr = nullptr,
                               Tracking tracking = Tracking())
        : m_s(ptr, std::move(tracking)) {}

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
        return *m_s.m_tracking.get();
      } else {
        return dataflow::g_notrace_instance;
      }
    }

    constexpr void set_tracking(Tracking tracking) {
      if constexpr (std::is_same_v<Kernel, WithTracking>) {
        m_s.m_tracking = WithTracking::FlyweightFromValue(std::move(tracking));
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

  constexpr size_t FlowPtrStructSize = sizeof(FlowPtr<int>);

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr FlowPtr<Pointee> MakeFlowPtr(Pointee *ptr,
                                         Tracking tracking = Tracking()) {
    return FlowPtr<Pointee>(ptr, std::move(tracking));
  }
}  // namespace ncc

#endif  // __NITRATE_CORE_CACHE_H__
