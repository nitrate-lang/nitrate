////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_IR_DELTA_NODES_CAST_H__
#define __QUIXCC_IR_DELTA_NODES_CAST_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>

namespace libquixcc::ir::delta
{
    class SCast : public libquixcc::ir::Value<NodeType::SCast>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const SCast *create(const libquixcc::ir::Value<> *type, const libquixcc::ir::Value<> *value);

        const libquixcc::ir::Value<> *type;
        const libquixcc::ir::Value<> *value;
    };

    class UCast : public libquixcc::ir::Value<NodeType::UCast>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const UCast *create(const libquixcc::ir::Value<> *type, const libquixcc::ir::Value<> *value);

        const libquixcc::ir::Value<> *type;
        const libquixcc::ir::Value<> *value;
    };

    class PtrICast : public libquixcc::ir::Value<NodeType::PtrICast>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const PtrICast *create(const libquixcc::ir::Value<> *type, const libquixcc::ir::Value<> *value);

        const libquixcc::ir::Value<> *type;
        const libquixcc::ir::Value<> *value;
    };

    class IPtrCast : public libquixcc::ir::Value<NodeType::IPtrCast>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const IPtrCast *create(const libquixcc::ir::Value<> *type, const libquixcc::ir::Value<> *value);

        const libquixcc::ir::Value<> *type;
        const libquixcc::ir::Value<> *value;
    };

    class Bitcast : public libquixcc::ir::Value<NodeType::Bitcast>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Bitcast *create(const libquixcc::ir::Value<> *type, const libquixcc::ir::Value<> *value);

        const libquixcc::ir::Value<> *type;
        const libquixcc::ir::Value<> *value;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_CAST_H__