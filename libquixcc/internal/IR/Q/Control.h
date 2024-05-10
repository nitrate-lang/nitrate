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

#ifndef __QUIXCC_IR_Q_NODES_CONTROL_H__
#define __QUIXCC_IR_Q_NODES_CONTROL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/QIR.h>

namespace libquixcc::ir::q
{
    class IfElse : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        IfElse(const Value<Q> *cond, const Value<Q> *then, const Value<Q> *els) : cond(cond), then(then), els(els) {}

    public:
        static const IfElse *create(const Value<Q> *cond, const Value<Q> *then, const Value<Q> *els);

        const Value<Q> *cond;
        const Value<Q> *then;
        const Value<Q> *els;
    };

    class While : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        While(const Value<Q> *cond, const Value<Q> *body) : cond(cond), body(body) {}

    public:
        static const While *create(const Value<Q> *cond, const Value<Q> *body);

        const Value<Q> *cond;
        const Value<Q> *body;
    };

    class For : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        For(const Value<Q> *init, const Value<Q> *cond, const Value<Q> *step, const Value<Q> *body) : init(init), cond(cond), step(step), body(body) {}

    public:
        static const For *create(const Value<Q> *init, const Value<Q> *cond, const Value<Q> *step, const Value<Q> *body);

        const Value<Q> *init;
        const Value<Q> *cond;
        const Value<Q> *step;
        const Value<Q> *body;
    };

    class Loop : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Loop(const Value<Q> *body) : body(body) {}

    public:
        static const Loop *create(const Value<Q> *body);

        const Value<Q> *body;
    };

    class Break : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Break *create();
    };

    class Continue : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Continue *create();
    };

    class Ret : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ret(const Value<Q> *value) : value(value) {}

    public:
        static const Ret *create(const Value<Q> *value);

        const Value<Q> *value;
    };

    class Throw : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Throw(const Value<Q> *value) : value(value) {}

    public:
        static const Throw *create(const Value<Q> *value);

        const Value<Q> *value;
    };

    class TryCatchFinally : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        TryCatchFinally(const Value<Q> *tryblock, std::vector<std::pair<const Value<Q> *, const Value<Q> *>> catchblocks, const Value<Q> *finallyblock) : tryblock(tryblock), catchblocks(catchblocks), finallyblock(finallyblock) {}

    public:
        static const TryCatchFinally *create(const Value<Q> *tryblock, std::vector<std::pair<const Value<Q> *, const Value<Q> *>> catchblocks, const Value<Q> *finallyblock);

        const Value<Q> *tryblock;
        std::vector<std::pair<const Value<Q> *, const Value<Q> *>> catchblocks;
        const Value<Q> *finallyblock;
    };

    class Case : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Case(const Value<Q> *value, const Value<Q> *body) : value(value), body(body) {}

    public:
        static const Case *create(const Value<Q> *value, const Value<Q> *body);

        const Value<Q> *value;
        const Value<Q> *body;
    };

    class Switch : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Switch(const Value<Q> *value, const std::set<const Case *> &cases, const Value<Q> *defaultcase) : value(value), cases(cases), defaultcase(defaultcase) {}

    public:
        static const Switch *create(const Value<Q> *value, const std::set<const Case *> &cases, const Value<Q> *defaultcase);

        const Value<Q> *value;
        const std::set<const Case *> cases;
        const Value<Q> *defaultcase;
    };
}

#endif // __QUIXCC_IR_Q_NODES_CONTROL_H__