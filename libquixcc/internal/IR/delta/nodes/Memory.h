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

#ifndef __QUIXCC_IR_DELTA_NODES_MEMORY_H__
#define __QUIXCC_IR_DELTA_NODES_MEMORY_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>

namespace libquixcc::ir::delta
{
    class Assign : public libquixcc::ir::Value<NodeType::Assign>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Assign *create(const libquixcc::ir::Value<> *var, const libquixcc::ir::Value<> *value, bool _volatile = false, uint64_t rank = 0);

        uint64_t rank; /* How many levels of dereferencing are to be done */
        const libquixcc::ir::Value<> *var;
        const libquixcc::ir::Value<> *value;
        bool _volatile;
    };

    class Load : public libquixcc::ir::Value<NodeType::Load>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Load *create(const libquixcc::ir::Value<> *var, uint64_t rank = 0);

        uint64_t rank; /* How many levels of dereferencing are to be done */
        const libquixcc::ir::Value<> *var;
    };

    class Index : public libquixcc::ir::Value<NodeType::Index>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Index *create(const libquixcc::ir::Value<> *var, const libquixcc::ir::Value<> *index);

        const libquixcc::ir::Value<> *var;
        const libquixcc::ir::Value<> *index;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_MEMORY_H__