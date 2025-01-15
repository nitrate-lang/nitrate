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
    void BeginObjImpl(size_t pair_count);
    void EndObjImpl();
    void BeginArrImpl(size_t size);
    void EndArrImpl();

  public:
    AstHash64()
        : AstWriter(
              std::bind(&AstHash64::StrImpl, this, std::placeholders::_1),
              std::bind(&AstHash64::UintImpl, this, std::placeholders::_1),
              std::bind(&AstHash64::DoubleImpl, this, std::placeholders::_1),
              std::bind(&AstHash64::BoolImpl, this, std::placeholders::_1),
              std::bind(&AstHash64::NullImpl, this),
              std::bind(&AstHash64::BeginObjImpl, this,
                        std::placeholders::_1),
              std::bind(&AstHash64::EndObjImpl, this),
              std::bind(&AstHash64::BeginArrImpl, this,
                        std::placeholders::_1),
              std::bind(&AstHash64::EndArrImpl, this)) {}
    virtual ~AstHash64() = default;

    uint64_t Get() {
      boost::uuids::detail::sha1::digest_type digest;
      m_sum.get_digest(digest);

      return static_cast<uint64_t>(digest[0]) << 32 | digest[1];
    }
  };
}  // namespace ncc::parse

#endif
