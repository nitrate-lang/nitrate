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

#ifndef __QUIXCC_IR_Q_NODES_MATH_H__
#define __QUIXCC_IR_Q_NODES_MATH_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/QIR.h>

namespace libquixcc::ir::q
{
    class Add : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Add(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Add *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Sub : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Sub(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Sub *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Mul : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Mul(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Mul *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Div : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Div(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Div *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Mod : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Mod(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Mod *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class BitAnd : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitAnd(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitAnd *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class BitOr : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitOr(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitOr *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class BitXor : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitXor(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitXor *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class BitNot : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitNot(const Value<Q> *operand) : operand(operand) {}

    public:
        static const BitNot *create(const Value<Q> *operand);

        const Value<Q> *operand;
    };

    class Shl : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Shl(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Shl *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Shr : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Shr(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Shr *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Rotl : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Rotl(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Rotl *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Rotr : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Rotr(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Rotr *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Eq : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Eq(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Eq *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Ne : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ne(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Ne *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Lt : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Lt(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Lt *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Gt : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Gt(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Gt *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Le : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Le(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Le *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Ge : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ge(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Ge *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class And : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        And(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const And *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Or : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Or(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Or *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };

    class Not : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Not(const Value<Q> *operand) : operand(operand) {}

    public:
        static const Not *create(const Value<Q> *operand);

        const Value<Q> *operand;
    };

    class Xor : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Xor(const Value<Q> *lhs, const Value<Q> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Xor *create(const Value<Q> *lhs, const Value<Q> *rhs);

        const Value<Q> *lhs;
        const Value<Q> *rhs;
    };
}

#endif // __QUIXCC_IR_Q_NODES_MATH_H__