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

NCC_EXPORT std::optional<uint64_t> ncc::ir::detail::TypeGetSizeBitsImpl(const Type* self) {
  std::optional<uint64_t> r;

  switch (self->GetKind()) {
    case IR_tU1: {
      r = 8;
      break;
    }

    case IR_tU8: {
      r = 8;
      break;
    }

    case IR_tU16: {
      r = 16;
      break;
    }

    case IR_tU32: {
      r = 32;
      break;
    }

    case IR_tU64: {
      r = 64;
      break;
    }

    case IR_tU128: {
      r = 128;
      break;
    }

    case IR_tI8: {
      r = 8;
      break;
    }

    case IR_tI16: {
      r = 16;
      break;
    }

    case IR_tI32: {
      r = 32;
      break;
    }

    case IR_tI64: {
      r = 64;
      break;
    }

    case IR_tI128: {
      r = 128;
      break;
    }

    case IR_tF16_TY: {
      r = 16;
      break;
    }

    case IR_tF32_TY: {
      r = 32;
      break;
    }

    case IR_tF64_TY: {
      r = 64;
      break;
    }

    case IR_tF128_TY: {
      r = 128;
      break;
    }

    case IR_tVOID: {
      r = 0;
      break;
    }

    case IR_tPTR: {
      r = self->As<PtrTy>()->GetNativeSize() * 8;
      break;
    }

    case IR_tCONST: {
      r = self->As<ConstTy>()->GetItem()->GetSizeBits();
      break;
    }

    case IR_tSTRUCT: {
      // The size of a packed struct is the sum of the sizes of its members
      size_t size = 0;
      bool okay = true;

      for (auto f : self->As<StructTy>()->GetFields()) {
        if (auto member_size = f->GetSizeBits()) {
          size += member_size.value();
        } else {
          okay = false;
          break;
        }
      }

      okay && (r = size);
      break;
    }

    case IR_tUNION: {
      // The size of a packed union is the size of its largest member
      size_t max_size = 0;
      bool okay = true;

      for (auto f : self->As<UnionTy>()->GetFields()) {
        if (auto member_size = f->GetSizeBits()) {
          max_size = std::max(max_size, member_size.value());
        } else {
          okay = false;
          break;
        }
      }

      okay && (r = max_size);
      break;
    }

    case IR_tARRAY: {
      auto a = self->As<ArrayTy>();
      if (auto element_size = a->GetElement()->GetSizeBits()) {
        r = element_size.value() * a->GetCount();
      }
      break;
    }

    case IR_tFUNC: {
      r = self->As<FnTy>()->GetNativeSize() * 8;
      break;
    }

    default: {
      break;
    }
  }

  return r;
}
