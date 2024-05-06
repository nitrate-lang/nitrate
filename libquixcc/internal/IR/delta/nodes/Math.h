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

#ifndef __QUIXCC_IR_DELTA_NODES_MATH_H__
#define __QUIXCC_IR_DELTA_NODES_MATH_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>

namespace libquixcc::ir::delta
{
    class Add : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Add(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Add *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Sub : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Sub(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Sub *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Mul : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Mul(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Mul *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Div : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Div(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Div *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Mod : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Mod(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Mod *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class BitAnd : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitAnd(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitAnd *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class BitOr : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitOr(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitOr *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class BitXor : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitXor(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitXor *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class BitNot : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitNot(const Value<Delta> *operand) : operand(operand) {}

    public:
        static const BitNot *create(const Value<Delta> *operand);

        const Value<Delta> *operand;
    };

    class Shl : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Shl(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Shl *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Shr : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Shr(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Shr *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Rotl : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Rotl(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Rotl *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Rotr : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Rotr(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Rotr *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Eq : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Eq(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Eq *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Ne : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ne(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Ne *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Lt : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Lt(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Lt *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Gt : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Gt(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Gt *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Le : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Le(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Le *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Ge : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ge(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Ge *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class And : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        And(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const And *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Or : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Or(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Or *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };

    class Not : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Not(const Value<Delta> *operand) : operand(operand) {}

    public:
        static const Not *create(const Value<Delta> *operand);

        const Value<Delta> *operand;
    };

    class Xor : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Xor(const Value<Delta> *lhs, const Value<Delta> *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Xor *create(const Value<Delta> *lhs, const Value<Delta> *rhs);

        const Value<Delta> *lhs;
        const Value<Delta> *rhs;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_MATH_H__