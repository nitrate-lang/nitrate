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
      FlowPtr<Pointee, Tracking> m_ptr =
          FlowPtr<Pointee, Tracking>::CreateNullPtr();

      static_assert(sizeof(m_ptr) >= sizeof(uintptr_t));

    public:
      using value_type = Pointee;

      ///=========================================================================
      /// Constructors

      template <class U = Pointee>
      constexpr NullableFlowPtr(U *nullable_ptr = nullptr,
                                Tracking tracking = Tracking()) {
        static_assert(std::is_convertible_v<U *, Pointee *>,
                      "U* must be convertible to Pointee*");

        if (nullable_ptr) {
          m_ptr = FlowPtr<Pointee, Tracking>(nullable_ptr, std::move(tracking));
        }
      }

      template <class U>
      constexpr NullableFlowPtr(std::optional<FlowPtr<U, Tracking>> ptr_opt) {
        static_assert(std::is_convertible_v<U *, Pointee *>,
                      "U* must be convertible to Pointee*");

        if (ptr_opt.has_value()) {
          auto v = ptr_opt.value();
          m_ptr = FlowPtr<Pointee, Tracking>(v.get(), v.trace());
        }
      }

      template <class U = Pointee>
      constexpr NullableFlowPtr(FlowPtr<U, Tracking> ptr)
          : NullableFlowPtr(ptr.get(), ptr.trace()) {
        static_assert(std::is_convertible_v<U *, Pointee *>,
                      "U* must be convertible to Pointee*");
      }

      constexpr NullableFlowPtr(std::nullopt_t, Tracking tracking = Tracking())
          : NullableFlowPtr(static_cast<Pointee *>(nullptr),
                            std::move(tracking)) {}

      constexpr NullableFlowPtr(std::nullptr_t, Tracking tracking = Tracking())
          : NullableFlowPtr(static_cast<Pointee *>(nullptr),
                            std::move(tracking)) {}

      constexpr NullableFlowPtr(const NullableFlowPtr<Pointee, Tracking> &O) {
        m_ptr = O.m_ptr;
      }

      constexpr NullableFlowPtr(NullableFlowPtr<Pointee, Tracking> &&O) {
        m_ptr = std::move(O.m_ptr);
      }

      constexpr NullableFlowPtr &operator=(
          const NullableFlowPtr<Pointee, Tracking> &O) {
        m_ptr = O.m_ptr;
        return *this;
      }

      constexpr NullableFlowPtr &operator=(
          NullableFlowPtr<Pointee, Tracking> &&O) {
        m_ptr = std::move(O.m_ptr);
        return *this;
      }

      constexpr ~NullableFlowPtr() = default;

      ///=========================================================================
      /// Comparison

      constexpr bool operator==(const NullableFlowPtr &O) const {
        return m_ptr == O.m_ptr;
      }

      constexpr bool operator!=(const NullableFlowPtr &O) const {
        return m_ptr != O.m_ptr;
      }

      constexpr bool operator==(std::nullptr_t) const {
        return m_ptr == nullptr;
      }
      constexpr bool operator==(std::nullopt_t) const {
        return m_ptr == nullptr;
      }

      ///=========================================================================
      /// Accessors

      constexpr bool has_value() const { return m_ptr != nullptr; }
      constexpr operator bool() const { return has_value(); }

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
        return has_value() ? value().get() : std::forward<U>(default_value);
      }
    };

    constexpr auto NullableFlowPtrStructSize = sizeof(NullableFlowPtr<int>);
    static_assert(sizeof(FlowPtr<int>) == sizeof(NullableFlowPtr<int>));
  }  // namespace flowptr_detail

  template <class Pointee, class Tracking = DefaultTracking>
  using NullableFlowPtr = flowptr_detail::NullableFlowPtr<Pointee, Tracking>;

  template <class Pointee, class Tracking = DefaultTracking>
  constexpr NullableFlowPtr<Pointee, Tracking> MakeNullableFlowPtr(
      Pointee *ptr, Tracking tracking = Tracking()) {
    return NullableFlowPtr<Pointee, Tracking>(ptr, std::move(tracking));
  }
}  // namespace ncc

namespace std {
  template <class Pointee, class Tracking>
  struct hash<ncc::NullableFlowPtr<Pointee, Tracking>> {
    size_t operator()(
        const ncc::NullableFlowPtr<Pointee, Tracking> &ptr) const {
      return std::hash<Pointee *>()(ptr.value_or(nullptr));
    }
  };
}  // namespace std

#endif
