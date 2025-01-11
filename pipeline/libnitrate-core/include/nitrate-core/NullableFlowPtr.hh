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

#include "nitrate-core/Logger.hh"

namespace ncc {
  namespace flowptr_detail {
    template <class Pointee, class Tracking = DefaultTracking>
    class NullableFlowPtr {
      uintptr_t m_ptr;

    public:
      using value_type = Pointee;

      ///=========================================================================
      /// Constructors

      constexpr NullableFlowPtr() {
        /// TODO: Implement this

        qcore_implement();
      }

      constexpr explicit NullableFlowPtr(FlowPtr<Pointee, Tracking>) {
        /// TODO: Implement this

        qcore_implement();
      }

      template <class U>
      constexpr NullableFlowPtr(U *, Tracking = Tracking()) {
        /// TODO: Implement this

        qcore_implement();
      }

      constexpr NullableFlowPtr(std::nullopt_t, Tracking = Tracking()) {
        /// TODO: Implement this

        qcore_implement();
      }

      constexpr NullableFlowPtr(std::nullptr_t, Tracking = Tracking()) {
        /// TODO: Implement this

        qcore_implement();
      }

      constexpr NullableFlowPtr(std::optional<FlowPtr<Pointee, Tracking>>) {
        /// TODO: Implement this

        qcore_implement();
      }

      template <class U = Pointee>
      constexpr NullableFlowPtr(FlowPtr<U, Tracking>) {
        /// TODO: Implement this

        qcore_implement();
      }

      template <class U>
      constexpr NullableFlowPtr(const NullableFlowPtr<U, Tracking> &) {
        /// TODO: Implement this

        qcore_implement();
      }

      template <class U>
      constexpr NullableFlowPtr(NullableFlowPtr<U, Tracking> &&) {
        /// TODO: Implement this

        qcore_implement();
      }

      template <class U>
      constexpr NullableFlowPtr &operator=(
          const NullableFlowPtr<U, Tracking> &) {
        /// TODO: Implement this

        qcore_implement();
        return *this;
      }

      template <class U>
      constexpr NullableFlowPtr &operator=(NullableFlowPtr<U, Tracking> &&) {
        /// TODO: Implement this

        qcore_implement();
        return *this;
      }

      ///=========================================================================
      /// Comparison

      bool operator==(const NullableFlowPtr &) const {
        /// TODO: Implement this

        qcore_implement();
      }

      bool operator!=(const NullableFlowPtr &) const {
        /// TODO: Implement this

        qcore_implement();
      }

      bool operator==(std::nullptr_t) const {
        /// TODO: Implement this

        qcore_implement();
      }

      ///=========================================================================
      /// Accessors

      operator bool() const {
        /// TODO: Implement this

        qcore_implement();
      }
      bool has_value() const {
        /// TODO: Implement this

        qcore_implement();
      }

      FlowPtr<Pointee, Tracking> &value() {
        if (!has_value()) [[unlikely]] {
          qcore_panicf("Attempted to dereference a nullptr. this=%p", this);
        }

        /// TODO: Implement this

        qcore_implement();
      }

      const FlowPtr<Pointee, Tracking> &value() const {
        if (!has_value()) [[unlikely]] {
          qcore_panicf("Attempted to dereference a nullptr. this=%p", this);
        }

        /// TODO: Implement this

        qcore_implement();
      }

      template <class U>
      Pointee *value_or(U &&) const {
        // return m_ptr ? m_ptr.get() : std::forward<U>(default_value);

        /// TODO: Implement this

        qcore_implement();
      }
    };

    constexpr auto NullableFlowPtrStructSize = sizeof(NullableFlowPtr<int>);
    // static_assert(sizeof(FlowPtr<int>) == sizeof(NullableFlowPtr<int>));
    /// TODO: Reenable assert
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
