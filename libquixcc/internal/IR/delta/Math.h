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
#include <IR/delta/Type.h>
#include <IR/delta/Expr.h>

namespace libquixcc::ir::delta
{
    class Add : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Add(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Add *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Sub : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Sub(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Sub *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Mul : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Mul(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Mul *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Div : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Div(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Div *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Mod : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Mod(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Mod *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class BitAnd : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitAnd(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitAnd *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class BitOr : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitOr(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitOr *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class BitXor : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitXor(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const BitXor *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class BitNot : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        BitNot(const Value *operand) : operand(operand) {}

    public:
        static const BitNot *create(const Value *operand);

        const Value *operand;
    };

    class Shl : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Shl(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Shl *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Shr : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Shr(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Shr *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Rotl : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Rotl(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Rotl *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Rotr : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Rotr(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Rotr *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Eq : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Eq(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Eq *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Ne : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ne(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Ne *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Lt : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Lt(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Lt *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Gt : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Gt(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Gt *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Le : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Le(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Le *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Ge : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ge(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Ge *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class And : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        And(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const And *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Or : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Or(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Or *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };

    class Not : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Not(const Value *operand) : operand(operand) {}

    public:
        static const Not *create(const Value *operand);

        const Value *operand;
    };

    class Xor : public Expr
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Xor(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {}

    public:
        static const Xor *create(const Expr *lhs, const Expr *rhs);

        const Expr *lhs;
        const Expr *rhs;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_MATH_H__