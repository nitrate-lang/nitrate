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
#include <IR/delta/Segment.h>
#include <IR/delta/Expr.h>

namespace libquixcc::ir::delta
{
    class IfElse : public Value
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        IfElse(const Expr *cond, const Value *then, const Value *els) : cond(cond), then(then), els(els)
        {
            ntype = (int)NodeType::IfElse;
        }

    public:
        static const IfElse *create(const Expr *cond, const Value *then, const Value *els);

        const Expr *cond;
        const Value *then;
        const Value *els;
    };

    class While : public Value
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        While(const Expr *cond, const Value *body) : cond(cond), body(body)
        {
            ntype = (int)NodeType::While;
        }

    public:
        static const While *create(const Expr *cond, const Value *body);

        const Expr *cond;
        const Value *body;
    };

    class Jmp : public Value
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Jmp(std::string target) : target(target)
        {
            ntype = (int)NodeType::Jmp;
        }

    public:
        static const Jmp *create(std::string target);

        std::string target;
    };

    class Label : public Value
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Label(std::string name) : name(name)
        {
            ntype = (int)NodeType::Label;
        }

    public:
        static const Label *create(std::string name);

        std::string name;
    };

    class Ret : public Value
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ret(const Expr *value) : value(value)
        {
            ntype = (int)NodeType::Ret;
        }

    public:
        static const Ret *create(const Expr *value);

        const Expr *value;
    };

    class Call : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Call(std::string callee, std::vector<const Expr *> args) : callee(callee), args(args)
        {
            ntype = (int)NodeType::Call;
        }

    public:
        static const Call *create(std::string callee, std::vector<const Expr *> args);

        std::string callee;
        std::vector<const Expr *> args;
    };

    class PtrCall : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        PtrCall(const Value *callee, std::vector<const Expr *> args) : callee(callee), args(args)
        {
            ntype = (int)NodeType::PtrCall;
        }

    public:
        static const PtrCall *create(const Value *callee, std::vector<const Expr *> args);

        const Value *callee;
        std::vector<const Expr *> args;
    };

    class Halt : public Value
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Halt()
        {
            ntype = (int)NodeType::Halt;
        }

    public:
        static const Halt *create();
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_CONTROL_H__