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

#ifndef __NITRATE_AST_CORE_HASH_H__
#define __NITRATE_AST_CORE_HASH_H__

#include <boost/uuid/detail/sha1.hpp>
#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <cstdint>
#include <nitrate-parser/ASTWriter.hh>

namespace ncc::parse {
  class NCC_EXPORT AstHash64 : public AstWriter {
    boost::uuids::detail::sha1 m_sum;

    void Update(uint64_t data) { m_sum.process_bytes(&data, sizeof(data)); }

    void StrImpl(std::string_view str);
    void UintImpl(uint64_t val);
    void DoubleImpl(double val);
    void BoolImpl(bool val);
    void NullImpl();
    void BeginObjImpl(size_t size);
    void EndObjImpl();
    void BeginArrImpl(size_t size);
    void EndArrImpl();

  public:
    AstHash64()
        : AstWriter(
              [this](auto&& x) { StrImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { UintImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { DoubleImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { BoolImpl(std::forward<decltype(x)>(x)); },
              [this] { NullImpl(); },
              [this](auto&& x) { BeginObjImpl(std::forward<decltype(x)>(x)); },
              [this] { EndObjImpl(); },
              [this](auto&& x) { BeginArrImpl(std::forward<decltype(x)>(x)); },
              [this] { EndArrImpl(); }) {}
    ~AstHash64() override = default;

    auto Get() -> uint64_t {
      boost::uuids::detail::sha1::digest_type digest;
      m_sum.get_digest(digest);

      return static_cast<uint64_t>(digest[0]) << 32 | digest[1];
    }
  };
}  // namespace ncc::parse

#endif
