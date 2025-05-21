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

#ifndef __NITRATE_CORE_NULLABLEFLOWPTR_H__
#define __NITRATE_CORE_NULLABLEFLOWPTR_H__

#include <nitrate-core/FlowPtr.hh>
#include <optional>

namespace ncc {
  namespace flowptr_detail {
    template <class Pointee, class Tracking = DefaultTracking>
    class NullableFlowPtr {
      FlowPtr<Pointee, Tracking> m_flow = FlowPtr<Pointee, Tracking>::CreateNullPtr();

    public:
      using value_type = Pointee;

      ///=========================================================================
      /// Constructors

      template <class U = Pointee>
      constexpr NullableFlowPtr(U *nullable_ptr = nullptr, Tracking tracking = Tracking()) {
        static_assert(std::is_convertible_v<U *, Pointee *>, "U* must be convertible to Pointee*");

        if (nullable_ptr) {
          m_flow = FlowPtr<Pointee, Tracking>(nullable_ptr, std::move(tracking));
        }
      }

      template <class U>
      constexpr NullableFlowPtr(std::optional<FlowPtr<U, Tracking>> ptr_opt) {
        static_assert(std::is_convertible_v<U *, Pointee *>, "U* must be convertible to Pointee*");

        if (ptr_opt.has_value()) {
          auto v = ptr_opt.value();
          m_flow = FlowPtr<Pointee, Tracking>(v.Get(), v.Trace());
        }
      }

      template <class U = Pointee>
      constexpr NullableFlowPtr(FlowPtr<U, Tracking> ptr) : NullableFlowPtr(ptr.Get(), ptr.Trace()) {
        static_assert(std::is_convertible_v<U *, Pointee *>, "U* must be convertible to Pointee*");
      }

      constexpr NullableFlowPtr(std::nullopt_t, Tracking tracking = Tracking())
          : NullableFlowPtr(static_cast<Pointee *>(nullptr), std::move(tracking)) {}

      constexpr NullableFlowPtr(std::nullptr_t, Tracking tracking = Tracking())
          : NullableFlowPtr(static_cast<Pointee *>(nullptr), std::move(tracking)) {}

      constexpr NullableFlowPtr(const NullableFlowPtr<Pointee, Tracking> &o) : m_flow(o.m_flow) {}

      constexpr NullableFlowPtr(NullableFlowPtr<Pointee, Tracking> &&o) noexcept : m_flow(std::move(o.m_flow)) {}

      constexpr auto operator=(const NullableFlowPtr<Pointee, Tracking> &o) -> NullableFlowPtr & {
        m_flow = o.m_flow;
        return *this;
      }

      constexpr auto operator=(NullableFlowPtr<Pointee, Tracking> &&o) noexcept -> NullableFlowPtr & {
        m_flow = std::move(o.m_flow);
        return *this;
      }

      constexpr ~NullableFlowPtr() = default;

      ///=========================================================================
      /// Comparison

      [[nodiscard, gnu::pure]] constexpr auto operator==(const NullableFlowPtr &o) const -> bool {
        return m_flow == o.m_flow;
      }
      [[nodiscard, gnu::pure]] constexpr auto operator!=(const NullableFlowPtr &o) const -> bool {
        return m_flow != o.m_flow;
      }
      [[nodiscard, gnu::pure]] constexpr auto operator==(std::nullptr_t) const -> bool { return m_flow == nullptr; }
      [[nodiscard, gnu::pure]] constexpr auto operator==(std::nullopt_t) const -> bool { return m_flow == nullptr; }

      ///=========================================================================
      /// Casting

      template <class U>
      [[nodiscard, gnu::pure]] constexpr operator NullableFlowPtr<const U>() const {
        static_assert(std::is_convertible_v<const Pointee *, const U *>, "Cannot convert Pointee* to U*");
        return NullableFlowPtr<const U>(static_cast<const U *>(m_flow.Get()), m_flow.Trace());
      }

      template <class U>
      [[nodiscard, gnu::pure]] constexpr operator NullableFlowPtr<U>() {
        static_assert(std::is_convertible_v<Pointee *, U *>, "Cannot convert Pointee* to U*");
        return NullableFlowPtr<U>(static_cast<U *>(m_flow.Get()), m_flow.Trace());
      }

      template <class U>
      [[nodiscard, gnu::pure]] [[nodiscard]] constexpr auto As() const {
        return NullableFlowPtr<const U>(reinterpret_cast<const U *>(m_flow.Get()), m_flow.Trace());
      }

      template <class U>
      [[nodiscard, gnu::pure]] constexpr auto As() {
        return NullableFlowPtr<U>(reinterpret_cast<U *>(m_flow.Get()), m_flow.Trace());
      }

      ///=========================================================================
      /// Accessors

      [[nodiscard, gnu::pure]] constexpr auto Isset() const -> bool { return m_flow != nullptr; }
      [[nodiscard, gnu::pure]] constexpr operator bool() const { return Isset(); }

      [[nodiscard, gnu::pure]] constexpr auto Unwrap() const -> FlowPtr<Pointee, Tracking> {
        assert(Isset() && "Attempted to dereference a nullptr.");
        return m_flow;
      }

      template <class U>
      [[nodiscard, gnu::pure]] constexpr auto ValueOr(U &&default_value) const -> Pointee * {
        return Isset() ? Unwrap().Get() : std::forward<U>(default_value);
      }

      ///=========================================================================
      /// Data-Flow tracking

      [[nodiscard, gnu::pure]] constexpr auto Trace() const -> Tracking { return m_flow.Trace(); }
      constexpr void SetTracking(auto tracking) { m_flow.SetTracking(std::move(tracking)); }
    } __attribute__((__packed__));

    static_assert(sizeof(FlowPtr<int>) == sizeof(NullableFlowPtr<int>));
  }  // namespace flowptr_detail

  template <class Pointee, class Tracking = DefaultTracking>
  using NullableFlowPtr = flowptr_detail::NullableFlowPtr<Pointee, Tracking>;

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr auto MakeNullableFlowPtr(Pointee *ptr,
                                     Tracking tracking = Tracking()) -> NullableFlowPtr<Pointee, Tracking> {
    return NullableFlowPtr<Pointee, Tracking>(ptr, std::move(tracking));
  }
}  // namespace ncc

namespace std {
  template <class Pointee, class Tracking>
  struct hash<ncc::NullableFlowPtr<Pointee, Tracking>> {
    auto operator()(const ncc::NullableFlowPtr<Pointee, Tracking> &ptr) const -> size_t {
      return std::hash<const Pointee *>()(ptr.value_or(nullptr));
    }
  };
}  // namespace std

#endif
