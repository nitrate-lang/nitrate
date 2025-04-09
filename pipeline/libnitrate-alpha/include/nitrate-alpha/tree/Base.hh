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

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace ncc::alpha::tree {
  class eINT;
  class eFLOAT;
  class eTUPLE;
  class eBIN;
  class eUNARY;
  class eACCESS;
  class eINDEX;
  class eBLOCK;
  class eCALL;
  class eIF;
  class eSWITCH;
  class eRET;
  class eBREAK;
  class eCONTINUE;
  class eWHILE;
  class eASM;
  class eVAR;
  class eFUNCTION;
  class tINT;
  class tUINT;
  class tFLOAT;
  class tVOID;
  class tINFER;
  class tREF;
  class tPTR;
  class tARRAY;
  class tTUPLE;
  class tFUNCTION;

  enum IRKind : uint8_t {
    AIR_DISCARDED,

    /*****************************************************************************
     * Expression
     ****************************************************************************/
    AIR_eINT,
    AIR_eFLOAT,
    AIR_eTUPLE,
    AIR_eBIN,
    AIR_eUNARY,
    AIR_eACCESS,
    AIR_eINDEX,
    AIR_eBLOCK,
    AIR_eCALL,
    AIR_eIF,
    AIR_eSWITCH,
    AIR_eRET,
    AIR_eBREAK,
    AIR_eCONTINUE,
    AIR_eWHILE,
    AIR_eASM,
    AIR_eVAR,
    AIR_eFUNCTION,

    /*****************************************************************************
     * Types
     ****************************************************************************/
    AIR_tINT,
    AIR_tUINT,
    AIR_tFLOAT,
    AIR_tVOID,
    AIR_tINFER,
    AIR_tREF,
    AIR_tPTR,
    AIR_tARRAY,
    AIR_tTUPLE,
    AIR_tFUNCTION,

    AIR__FIRST = AIR_DISCARDED,
    AIR__LAST = AIR_tFUNCTION,
  };

  static inline constexpr size_t kAIRNodeCount = AIR__LAST - AIR__FIRST + 1;

  class Base {
    IRKind m_kind : 6;
    bool m_is_poison : 1 = false;
    bool m_is_discarded : 1 = false;

    template <typename T>
    [[nodiscard, gnu::pure]] static constexpr auto SafeCastAs(Base *ptr) -> T * {
#ifndef NDEBUG
      if (!ptr) [[unlikely]] {
        return nullptr;
      }

      if (!ptr->Is<T>()) [[unlikely]] {
        qcore_panicf("Invalid cast from %s to %s", ptr->GetKindName(), GetKindName(GetTypeCode<T>()));
      }
#endif

      return reinterpret_cast<T *>(ptr);
    }

  public:
    constexpr Base(IRKind kind) : m_kind(kind){};
    constexpr Base(const Base &) = delete;
    constexpr Base(Base &&) = default;
    constexpr Base &operator=(const Base &) = delete;
    constexpr Base &operator=(Base &&) = default;

    /*****************************************************************************
     * Read-only accessors
     ****************************************************************************/

    [[nodiscard, gnu::pure]] constexpr auto IsDiscarded() const -> bool { return m_is_discarded; }
    [[nodiscard, gnu::pure]] constexpr auto IsPoison() const -> bool { return m_is_poison; }
    [[nodiscard, gnu::pure]] constexpr auto IsOkay() const -> bool { return !IsDiscarded() && !IsPoison(); }

    [[nodiscard, gnu::pure]] constexpr auto GetKind() const -> IRKind { return m_kind; }
    [[nodiscard, gnu::pure]] constexpr auto GetKindName() const -> std::string_view { return GetKindName(GetKind()); }
    [[nodiscard, gnu::pure]] constexpr auto Is(IRKind type) const -> bool { return type == GetKind(); }

    template <class T>
    [[nodiscard, gnu::pure]] constexpr auto Is() const -> bool {
      return GetTypeCode<T>() == GetKind();
    }

    [[nodiscard, gnu::const]] static constexpr auto GetKindName(IRKind type) -> std::string_view;

    template <typename T>
    [[nodiscard, gnu::const]] static constexpr auto GetTypeCode() -> IRKind {
      if constexpr (std::is_same_v<T, eINT>) {
        return AIR_eINT;
      } else if constexpr (std::is_same_v<T, eFLOAT>) {
        return AIR_eFLOAT;
      } else if constexpr (std::is_same_v<T, eTUPLE>) {
        return AIR_eTUPLE;
      } else if constexpr (std::is_same_v<T, eBIN>) {
        return AIR_eBIN;
      } else if constexpr (std::is_same_v<T, eUNARY>) {
        return AIR_eUNARY;
      } else if constexpr (std::is_same_v<T, eACCESS>) {
        return AIR_eACCESS;
      } else if constexpr (std::is_same_v<T, eINDEX>) {
        return AIR_eINDEX;
      } else if constexpr (std::is_same_v<T, eBLOCK>) {
        return AIR_eBLOCK;
      } else if constexpr (std::is_same_v<T, eCALL>) {
        return AIR_eCALL;
      } else if constexpr (std::is_same_v<T, eIF>) {
        return AIR_eIF;
      } else if constexpr (std::is_same_v<T, eSWITCH>) {
        return AIR_eSWITCH;
      } else if constexpr (std::is_same_v<T, eRET>) {
        return AIR_eRET;
      } else if constexpr (std::is_same_v<T, eBREAK>) {
        return AIR_eBREAK;
      } else if constexpr (std::is_same_v<T, eCONTINUE>) {
        return AIR_eCONTINUE;
      } else if constexpr (std::is_same_v<T, eWHILE>) {
        return AIR_eWHILE;
      } else if constexpr (std::is_same_v<T, eASM>) {
        return AIR_eASM;
      } else if constexpr (std::is_same_v<T, eVAR>) {
        return AIR_eVAR;
      } else if constexpr (std::is_same_v<T, eFUNCTION>) {
        return AIR_eFUNCTION;
      } else if constexpr (std::is_same_v<T, tINT>) {
        return AIR_tINT;
      } else if constexpr (std::is_same_v<T, tUINT>) {
        return AIR_tUINT;
      } else if constexpr (std::is_same_v<T, tFLOAT>) {
        return AIR_tFLOAT;
      } else if constexpr (std::is_same_v<T, tVOID>) {
        return AIR_tVOID;
      } else if constexpr (std::is_same_v<T, tINFER>) {
        return AIR_tINFER;
      } else if constexpr (std::is_same_v<T, tREF>) {
        return AIR_tREF;
      } else if constexpr (std::is_same_v<T, tPTR>) {
        return AIR_tPTR;
      } else if constexpr (std::is_same_v<T, tARRAY>) {
        return AIR_tARRAY;
      } else if constexpr (std::is_same_v<T, tTUPLE>) {
        return AIR_tTUPLE;
      } else if constexpr (std::is_same_v<T, tFUNCTION>) {
        return AIR_tFUNCTION;
      } else {
        static_assert(!std::is_same_v<T, T>, "Unrecognized type to Base::GetTypeCode");
      }
    }

    /*****************************************************************************
     * Casting
     ****************************************************************************/

    template <typename T>
    [[nodiscard, gnu::pure]] constexpr auto As() -> T * {
      return SafeCastAs<T>(this);
    }

    template <typename T>
    [[nodiscard, gnu::pure]] constexpr auto As() const -> const T * {
      return SafeCastAs<T>(const_cast<decltype(this)>(this));
    }

    /*****************************************************************************
     * Visitation
     ****************************************************************************/

    template <typename Visitor>
    constexpr void Accept(Visitor &&v) {
      v.Dispatch(this);
    }

    /*****************************************************************************
     * Debugging
     ****************************************************************************/

    enum class PrintMode : uint8_t {
      kDefault = 0,
    };

    auto Dump(std::ostream &os, PrintMode mode = PrintMode::kDefault) const -> std::ostream &;
    [[nodiscard]] auto Dump(PrintMode mode = PrintMode::kDefault) const -> std::string;

    [[nodiscard]] bool InvariantCheck(bool no_panic = false) const;

    /*****************************************************************************
     * Mutators
     ****************************************************************************/

    constexpr void Discard() { m_is_discarded = true; }
    constexpr void Poison() { m_is_poison = true; }
  } __attribute__((packed));

  static_assert(sizeof(Base) == 1, "Failed to pack Base class");

  namespace detail {
    constexpr static auto kGetKindNames = []() {
      std::array<std::string_view, kAIRNodeCount> r;
      r.fill("");

      r[AIR_DISCARDED] = "DISCARDED";
      r[AIR_eINT] = "eINT";
      r[AIR_eFLOAT] = "eFLOAT";
      r[AIR_eTUPLE] = "eTUPLE";
      r[AIR_eBIN] = "eBIN";
      r[AIR_eUNARY] = "eUNARY";
      r[AIR_eACCESS] = "eACCESS";
      r[AIR_eINDEX] = "eINDEX";
      r[AIR_eBLOCK] = "eBLOCK";
      r[AIR_eCALL] = "eCALL";
      r[AIR_eIF] = "eIF";
      r[AIR_eSWITCH] = "eSWITCH";
      r[AIR_eRET] = "eRET";
      r[AIR_eBREAK] = "eBREAK";
      r[AIR_eCONTINUE] = "eCONTINUE";
      r[AIR_eWHILE] = "eWHILE";
      r[AIR_eASM] = "eASM";
      r[AIR_eVAR] = "eVAR";
      r[AIR_eFUNCTION] = "eFUNCTION";
      r[AIR_tINT] = "tINT";
      r[AIR_tUINT] = "tUINT";
      r[AIR_tFLOAT] = "tFLOAT";
      r[AIR_tVOID] = "tVOID";
      r[AIR_tINFER] = "tINFER";
      r[AIR_tREF] = "tREF";
      r[AIR_tPTR] = "tPTR";
      r[AIR_tARRAY] = "tARRAY";
      r[AIR_tTUPLE] = "tTUPLE";
      r[AIR_tFUNCTION] = "tFUNCTION";

      return r;
    }();
  }  // namespace detail

  [[nodiscard, gnu::const]] constexpr auto Base::GetKindName(IRKind type) -> std::string_view {
    return detail::kGetKindNames[static_cast<size_t>(type)];
  }

}  // namespace ncc::alpha::tree
