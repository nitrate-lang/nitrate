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
    class SCast : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        SCast(const Value<Delta> *type, const Value<Delta> *value) : type(type), value(value) {}

    public:
        static const SCast *create(const Value<Delta> *type, const Value<Delta> *value);

        const Value<Delta> *type;
        const Value<Delta> *value;
    };

    class UCast : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        UCast(const Value<Delta> *type, const Value<Delta> *value) : type(type), value(value) {}

    public:
        static const UCast *create(const Value<Delta> *type, const Value<Delta> *value);

        const Value<Delta> *type;
        const Value<Delta> *value;
    };

    class PtrICast : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        PtrICast(const Value<Delta> *type, const Value<Delta> *value) : type(type), value(value) {}

    public:
        static const PtrICast *create(const Value<Delta> *type, const Value<Delta> *value);

        const Value<Delta> *type;
        const Value<Delta> *value;
    };

    class IPtrCast : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        IPtrCast(const Value<Delta> *type, const Value<Delta> *value) : type(type), value(value) {}

    public:
        static const IPtrCast *create(const Value<Delta> *type, const Value<Delta> *value);

        const Value<Delta> *type;
        const Value<Delta> *value;
    };

    class Bitcast : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Bitcast(const Value<Delta> *type, const Value<Delta> *value) : type(type), value(value) {}

    public:
        static const Bitcast *create(const Value<Delta> *type, const Value<Delta> *value);

        const Value<Delta> *type;
        const Value<Delta> *value;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_CAST_H__