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

#include <cstdint>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Nodes.hh>

NCC_EXPORT std::optional<uint64_t> ncc::ir::detail::Type_getAlignBitsImpl(
    const Type* self) {
  std::optional<uint64_t> R;

  switch (self->getKind()) {
    case IR_tU1: {
      R = 8;
      break;
    }

    case IR_tU8: {
      R = 8;
      break;
    }

    case IR_tU16: {
      R = 16;
      break;
    }

    case IR_tU32: {
      R = 32;
      break;
    }

    case IR_tU64: {
      R = 64;
      break;
    }

    case IR_tU128: {
      R = 128;
      break;
    }

    case IR_tI8: {
      R = 8;
      break;
    }

    case IR_tI16: {
      R = 16;
      break;
    }

    case IR_tI32: {
      R = 32;
      break;
    }

    case IR_tI64: {
      R = 64;
      break;
    }

    case IR_tI128: {
      R = 128;
      break;
    }

    case IR_tF16_TY: {
      R = 16;
      break;
    }

    case IR_tF32_TY: {
      R = 32;
      break;
    }

    case IR_tF64_TY: {
      R = 64;
      break;
    }

    case IR_tF128_TY: {
      R = 128;
      break;
    }

    case IR_tVOID: {
      R = 0;
      break;
    }

    case IR_tPTR: {
      R = self->as<PtrTy>()->getNativeSize() * 8;
      break;
    }

    case IR_tCONST: {
      R = self->as<ConstTy>()->getItem()->getAlignBits();
      break;
    }

    case IR_tSTRUCT: {
      // The alignment of a struct is the maximum alignment of its members
      size_t max_alignment = 0;
      bool okay = true;

      for (auto f : self->as<StructTy>()->getFields()) {
        if (auto member_alignment = f->getAlignBits()) {
          max_alignment = std::max(max_alignment, member_alignment.value());
        } else {
          okay = false;
          break;
        }
      }

      okay && (R = max_alignment);
      break;
    }

    case IR_tUNION: {
      // The alignment of a union is the maximum alignment of its members
      size_t max_alignment = 0;
      bool okay = true;

      for (auto f : self->as<UnionTy>()->getFields()) {
        if (auto member_alignment = f->getAlignBits()) {
          max_alignment = std::max(max_alignment, member_alignment.value());
        } else {
          okay = false;
          break;
        }
      }

      okay && (R = max_alignment);
      break;
    }

    case IR_tARRAY: {
      R = self->as<ArrayTy>()->getElement()->getAlignBits();
      break;
    }

    case IR_tFUNC: {
      R = self->as<FnTy>()->getNativeSize() * 8;
      break;
    }

    default: {
      break;
    }
  }

  return R;
}
