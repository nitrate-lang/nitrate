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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <source_location>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ncc {
  namespace trace {
    class Origin {
      const char *m_file_name, *m_function_name;
      unsigned m_line, m_column;

    public:
      constexpr Origin(std::source_location loc = std::source_location::current())
          : m_file_name(loc.file_name()),
            m_function_name(loc.function_name()),
            m_line(loc.line()),
            m_column(loc.column()) {}

      [[nodiscard]] constexpr auto File() const { return m_file_name; }
      [[nodiscard]] constexpr auto Function() const { return m_function_name; }
      [[nodiscard]] constexpr auto Line() const { return m_line; }
      [[nodiscard]] constexpr auto Column() const { return m_column; }

      constexpr auto operator==(const Origin &o) const -> bool {
        std::string_view a(m_file_name);
        std::string_view b(o.m_file_name);
        std::string_view c(m_function_name);
        std::string_view d(o.m_function_name);

        return m_line == o.m_line && m_column == o.m_column && a == b && c == d;
      }
    } __attribute__((packed));

    class Empty {
    public:
      constexpr Empty() = default;
      constexpr auto operator==(const Empty &) const -> bool { return true; }
    } __attribute__((packed));

    static inline trace::Empty NoTraceStatic;
  }  // namespace trace

#if NITRATE_FLOWPTR_TRACE
  using DefaultTracking = trace::Origin;
#else
  using DefaultTracking = trace::Empty;
#endif

  namespace flowptr_detail {
    template <class Pointee, class Tracking>
    class WithTracking {
      Pointee *m_raw_ptr;

    public:
      Tracking m_tracking;

      constexpr WithTracking(Pointee *ptr, Tracking tracking) : m_raw_ptr(ptr), m_tracking(std::move(tracking)) {}
      [[nodiscard]] constexpr auto GetPointer() const -> Pointee * { return m_raw_ptr; }
    } __attribute__((packed));

    template <class Pointee, class Tracking>
    class WithoutTracking {
      Pointee *m_raw_ptr;

    public:
      constexpr WithoutTracking(Pointee *ptr, Tracking) : m_raw_ptr(ptr) {}
      [[nodiscard]] constexpr auto GetPointer() const -> Pointee * { return m_raw_ptr; }
    } __attribute__((packed));

    template <class Pointee, class Tracking>
    class NullableFlowPtr;

    template <class Pointee, class Tracking>
    using flowptr_data_t = std::conditional_t<std::is_same_v<Tracking, trace::Empty>,
                                              WithoutTracking<Pointee, Tracking>, WithTracking<Pointee, Tracking>>;

    template <class Pointee, class Tracking = DefaultTracking>
    class FlowPtr {
      friend class NullableFlowPtr<Pointee, Tracking>;

      using SelfData = flowptr_data_t<Pointee, Tracking>;
      constexpr static bool kIsTracking = std::is_same_v<SelfData, WithTracking<Pointee, Tracking>>;

      SelfData m_s;

      constexpr FlowPtr() : m_s(nullptr, Tracking()) {}

      constexpr static auto CreateNullPtr() -> FlowPtr<Pointee, Tracking> {
        FlowPtr<Pointee, Tracking> ptr;
        return ptr;
      }

    public:
      using value_type = Pointee;

      ///=========================================================================
      /// Constructors

      template <class U>
      constexpr FlowPtr(U *ptr, Tracking tracking = Tracking()) : m_s(ptr, std::move(tracking)) {
        static_assert(std::is_convertible_v<U *, Pointee *>, "Cannot convert U* to Pointee*");
        assert(ptr != nullptr && "FlowPtr cannot be null");
      }

      constexpr FlowPtr(const FlowPtr &o) : m_s(o.m_s) {}
      constexpr FlowPtr(FlowPtr &&o) noexcept : m_s(std::move(o.m_s)) {}

      constexpr auto operator=(const FlowPtr &o) -> FlowPtr & {
        m_s = o.m_s;
        return *this;
      }

      constexpr auto operator=(FlowPtr &&o) noexcept -> FlowPtr & {
        m_s = std::move(o.m_s);
        return *this;
      }

      constexpr ~FlowPtr() = default;

      ///=========================================================================
      /// Comparison

      [[nodiscard, gnu::pure]] constexpr auto operator==(const FlowPtr &o) const -> bool {
        return m_s.GetPointer() == o.m_s.GetPointer();
      }
      [[nodiscard, gnu::pure]] constexpr auto operator!=(const FlowPtr &o) const -> bool {
        return m_s.GetPointer() != o.m_s.GetPointer();
      }
      [[nodiscard, gnu::pure]] constexpr auto operator==(std::nullptr_t) const -> bool {
        return m_s.GetPointer() == nullptr;
      }

      ///=========================================================================
      /// Accessors

      [[nodiscard, gnu::pure]] constexpr auto operator->() -> Pointee * { return Get(); }
      [[nodiscard, gnu::pure]] constexpr auto operator->() const -> const Pointee * { return Get(); }
      [[nodiscard, gnu::pure]] constexpr auto Get() -> Pointee * { return m_s.GetPointer(); }
      [[nodiscard, gnu::pure]] constexpr auto Get() const -> const Pointee * { return m_s.GetPointer(); }
      [[nodiscard, gnu::pure]] constexpr operator bool() const { return Get() != nullptr; }

      ///=========================================================================
      /// Casting

      template <class U>
      [[nodiscard, gnu::pure]] constexpr operator FlowPtr<const U>() const {
        static_assert(std::is_convertible_v<const Pointee *, const U *>, "Cannot convert Pointee* to U*");
        return FlowPtr<const U>(static_cast<const U *>(Get()), Trace());
      }

      template <class U>
      [[nodiscard, gnu::pure]] constexpr operator FlowPtr<U>() {
        static_assert(std::is_convertible_v<Pointee *, U *>, "Cannot convert Pointee* to U*");
        return FlowPtr<U>(static_cast<U *>(Get()), Trace());
      }

      template <class U>
      [[nodiscard, gnu::pure]] constexpr auto As() const {
        return FlowPtr<const U>(reinterpret_cast<const U *>(Get()), Trace());
      }

      template <class U>
      [[nodiscard, gnu::pure]] constexpr auto As() {
        return FlowPtr<U>(reinterpret_cast<U *>(Get()), Trace());
      }

      ///=========================================================================
      /// Data-Flow tracking

      [[nodiscard]] constexpr auto Trace() const -> Tracking {
        if constexpr (kIsTracking) {
          return m_s.m_tracking;
        } else {
          return trace::NoTraceStatic;
        }
      }

      constexpr void SetTracking(auto tracking) {
        if constexpr (kIsTracking) {
          m_s.m_tracking = std::move(tracking);
        }
      }

      ///=========================================================================
      /// Visitor pass-through

      template <class Vistor>
      constexpr void Accept(Vistor &v) {
        v.Dispatch(*this);
      }

      template <class Vistor>
      constexpr void Accept(Vistor &&v) {
        v.Dispatch(*this);
      }
    } __attribute__((packed));

    static_assert(sizeof(FlowPtr<void, trace::Empty>) <= sizeof(uintptr_t),
                  "FlowPtr<void> must be no larger then a pointer");
  }  // namespace flowptr_detail

  template <class Pointee, class Tracking = DefaultTracking>
  using FlowPtr = flowptr_detail::FlowPtr<Pointee, Tracking>;

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr auto MakeFlowPtr(Pointee *ptr, Tracking tracking = Tracking()) -> FlowPtr<Pointee, Tracking> {
    return FlowPtr<Pointee, Tracking>(ptr, std::move(tracking));
  }
}  // namespace ncc

namespace std {
  template <class Pointee, class Tracking>
  struct hash<ncc::FlowPtr<Pointee, Tracking>> {
    auto operator()(const ncc::FlowPtr<Pointee, Tracking> &ptr) const -> size_t {
      return std::hash<const Pointee *>()(ptr.Get());
    }
  };
}  // namespace std

#endif
