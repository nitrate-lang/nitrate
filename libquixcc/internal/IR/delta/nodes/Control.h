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

#ifndef __QUIXCC_IR_DELTA_NODES_CONTROL_H__
#define __QUIXCC_IR_DELTA_NODES_CONTROL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>
#include <IR/delta/nodes/Segment.h>

namespace libquixcc::ir::delta
{
    class IfElse : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        IfElse(const Value<Delta> *cond, const Value<Delta> *then, const Value<Delta> *els) : cond(cond), then(then), els(els) {}

    public:
        static const IfElse *create(const Value<Delta> *cond, const Value<Delta> *then, const Value<Delta> *els);

        const Value<Delta> *cond;
        const Value<Delta> *then;
        const Value<Delta> *els;
    };

    class While : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        While(const Value<Delta> *cond, const Value<Delta> *body) : cond(cond), body(body) {}

    public:
        static const While *create(const Value<Delta> *cond, const Value<Delta> *body);

        const Value<Delta> *cond;
        const Value<Delta> *body;
    };

    class Jmp : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Jmp(std::string target) : target(target) {}

    public:
        static const Jmp *create(std::string target);

        std::string target;
    };

    class Label : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Label(std::string name) : name(name) {}

    public:
        static const Label *create(std::string name);

        std::string name;
    };

    class Ret : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ret(const Value<Delta> *value) : value(value) {}

    public:
        static const Ret *create(const Value<Delta> *value);

        const Value<Delta> *value;
    };

    class Call : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Call(std::string callee, std::vector<const Value<Delta> *> args) : callee(callee), args(args) {}

    public:
        static const Call *create(std::string callee, std::vector<const Value<Delta> *> args);

        std::string callee;
        std::vector<const Value<Delta> *> args;
    };

    class PtrCall : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        PtrCall(const Value<Delta> *callee, std::vector<const Value<Delta> *> args) : callee(callee), args(args) {}

    public:
        static const PtrCall *create(const Value<Delta> *callee, std::vector<const Value<Delta> *> args);

        const Value<Delta> *callee;
        std::vector<const Value<Delta> *> args;
    };

    class Halt : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Halt() {}

    public:
        static const Halt *create();
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_CONTROL_H__