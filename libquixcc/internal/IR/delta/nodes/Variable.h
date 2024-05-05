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

#ifndef __QUIXCC_IR_DELTA_NODES_VARIABLE_H__
#define __QUIXCC_IR_DELTA_NODES_VARIABLE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>

namespace libquixcc::ir::delta
{
    class Local : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Local(std::string name, const Value<Delta> *type) : name(name), type(type) {}

    public:
        static const Local *create(std::string name, const Value<Delta> *type);

        std::string name;
        const Value<Delta> *type;
    };

    class Global : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Global(std::string name, const Value<Delta> *type, const Value<Delta> *value, bool _volatile, bool _atomic) : name(name), type(type), value(value), _volatile(_volatile), _atomic(_atomic) {}

    public:
        static const Global *create(std::string name, const Value<Delta> *type, const Value<Delta> *value, bool _volatile = false, bool _atomic = false);

        std::string name;
        const Value<Delta> *type;
        const Value<Delta> *value;
        bool _volatile;
        bool _atomic;
    };

    class Number : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Number(std::string value) : value(value) {}

    public:
        static const Number *create(std::string value);

        std::string value;
    };

    class String : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        String(std::string value) : value(value) {}

    public:
        static const String *create(std::string value);

        std::string value;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_VARIABLE_H__