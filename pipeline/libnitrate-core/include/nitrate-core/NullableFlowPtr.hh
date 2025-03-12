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
      FlowPtr<Pointee, Tracking> m_ptr = FlowPtr<Pointee, Tracking>::CreateNullPtr();

      static_assert(sizeof(m_ptr) >= sizeof(uintptr_t));

    public:
      using value_type = Pointee;

      ///=========================================================================
      /// Constructors

      template <class U = Pointee>
      constexpr NullableFlowPtr(U *nullable_ptr = nullptr, Tracking tracking = Tracking()) {
        static_assert(std::is_convertible_v<U *, Pointee *>, "U* must be convertible to Pointee*");

        if (nullable_ptr) {
          m_ptr = FlowPtr<Pointee, Tracking>(nullable_ptr, std::move(tracking));
        }
      }

      template <class U>
      constexpr NullableFlowPtr(std::optional<FlowPtr<U, Tracking>> ptr_opt) {
        static_assert(std::is_convertible_v<U *, Pointee *>, "U* must be convertible to Pointee*");

        if (ptr_opt.has_value()) {
          auto v = ptr_opt.value();
          m_ptr = FlowPtr<Pointee, Tracking>(v.get(), v.Trace());
        }
      }

      template <class U = Pointee>
      constexpr NullableFlowPtr(FlowPtr<U, Tracking> ptr) : NullableFlowPtr(ptr.get(), ptr.Trace()) {
        static_assert(std::is_convertible_v<U *, Pointee *>, "U* must be convertible to Pointee*");
      }

      constexpr NullableFlowPtr(std::nullopt_t, Tracking tracking = Tracking())
          : NullableFlowPtr(static_cast<Pointee *>(nullptr), std::move(tracking)) {}

      constexpr NullableFlowPtr(std::nullptr_t, Tracking tracking = Tracking())
          : NullableFlowPtr(static_cast<Pointee *>(nullptr), std::move(tracking)) {}

      constexpr NullableFlowPtr(const NullableFlowPtr<Pointee, Tracking> &o) : m_ptr(o.m_ptr) {}

      constexpr NullableFlowPtr(NullableFlowPtr<Pointee, Tracking> &&o) noexcept : m_ptr(std::move(o.m_ptr)) {}

      constexpr auto operator=(const NullableFlowPtr<Pointee, Tracking> &o) -> NullableFlowPtr & {
        m_ptr = o.m_ptr;
        return *this;
      }

      constexpr auto operator=(NullableFlowPtr<Pointee, Tracking> &&o) noexcept -> NullableFlowPtr & {
        m_ptr = std::move(o.m_ptr);
        return *this;
      }

      constexpr ~NullableFlowPtr() = default;

      ///=========================================================================
      /// Comparison

      constexpr auto operator==(const NullableFlowPtr &o) const -> bool { return m_ptr == o.m_ptr; }

      constexpr auto operator!=(const NullableFlowPtr &o) const -> bool { return m_ptr != o.m_ptr; }

      constexpr auto operator==(std::nullptr_t) const -> bool { return m_ptr == nullptr; }
      constexpr auto operator==(std::nullopt_t) const -> bool { return m_ptr == nullptr; }

      ///=========================================================================
      /// Casting

      template <class U>
      constexpr operator NullableFlowPtr<const U>() const {
        static_assert(std::is_convertible_v<const Pointee *, const U *>, "Cannot convert Pointee* to U*");
        return NullableFlowPtr<const U>(static_cast<const U *>(m_ptr.get()), m_ptr.Trace());
      }

      template <class U>
      constexpr operator NullableFlowPtr<U>() {
        static_assert(std::is_convertible_v<Pointee *, U *>, "Cannot convert Pointee* to U*");
        return NullableFlowPtr<U>(static_cast<U *>(m_ptr.get()), m_ptr.Trace());
      }

      template <class U>
      constexpr auto As() const {
        return NullableFlowPtr<const U>(reinterpret_cast<const U *>(m_ptr.get()), m_ptr.Trace());
      }

      template <class U>
      constexpr auto As() {
        return NullableFlowPtr<U>(reinterpret_cast<U *>(m_ptr.get()), m_ptr.Trace());
      }

      ///=========================================================================
      /// Accessors

      [[nodiscard]] constexpr bool has_value() const {  // NOLINT
        return m_ptr != nullptr;
      }
      constexpr operator bool() const { return has_value(); }

      constexpr FlowPtr<Pointee, Tracking> &value() {  // NOLINT
        if (!has_value()) [[unlikely]] {
          qcore_panicf("Attempted to dereference a nullptr. this=%p", this);
        }

        return m_ptr;
      }

      constexpr const FlowPtr<Pointee, Tracking> &value() const {  // NOLINT
        if (!has_value()) [[unlikely]] {
          qcore_panicf("Attempted to dereference a nullptr. this=%p", this);
        }

        return m_ptr;
      }

      template <class U>
      constexpr Pointee *value_or(U &&default_value) const {  // NOLINT
        return has_value() ? value().get() : std::forward<U>(default_value);
      }

      ///=========================================================================
      /// Data-Flow tracking

      [[nodiscard]] constexpr auto Trace() const -> const Tracking & { return m_ptr.Trace(); }
      constexpr void SetTracking(auto tracking) { m_ptr.SetTracking(std::move(tracking)); }
    };

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
