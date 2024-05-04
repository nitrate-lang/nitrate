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
    class Add : public libquixcc::ir::Value<NodeType::Add>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Add *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Sub : public libquixcc::ir::Value<NodeType::Sub>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Sub *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Mul : public libquixcc::ir::Value<NodeType::Mul>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Mul *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Div : public libquixcc::ir::Value<NodeType::Div>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Div *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Mod : public libquixcc::ir::Value<NodeType::Mod>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Mod *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class BitAnd : public libquixcc::ir::Value<NodeType::BitAnd>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const BitAnd *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class BitOr : public libquixcc::ir::Value<NodeType::BitOr>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const BitOr *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class BitXor : public libquixcc::ir::Value<NodeType::BitXor>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const BitXor *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class BitNot : public libquixcc::ir::Value<NodeType::BitNot>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const BitNot *create(const libquixcc::ir::Value<> *operand);

        const libquixcc::ir::Value<> *operand;
    };

    class Shl : public libquixcc::ir::Value<NodeType::Shl>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Shl *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Shr : public libquixcc::ir::Value<NodeType::Shr>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Shr *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Rotl : public libquixcc::ir::Value<NodeType::Rotl>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Rotl *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Rotr : public libquixcc::ir::Value<NodeType::Rotr>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Rotr *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Eq : public libquixcc::ir::Value<NodeType::Eq>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Eq *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Ne : public libquixcc::ir::Value<NodeType::Ne>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Ne *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Lt : public libquixcc::ir::Value<NodeType::Lt>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Lt *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Gt : public libquixcc::ir::Value<NodeType::Gt>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Gt *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Le : public libquixcc::ir::Value<NodeType::Le>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Le *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Ge : public libquixcc::ir::Value<NodeType::Ge>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Ge *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class And : public libquixcc::ir::Value<NodeType::And>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const And *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Or : public libquixcc::ir::Value<NodeType::Or>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Or *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };

    class Not : public libquixcc::ir::Value<NodeType::Not>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Not *create(const libquixcc::ir::Value<> *operand);

        const libquixcc::ir::Value<> *operand;
    };

    class Xor : public libquixcc::ir::Value<NodeType::Xor>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Xor *create(const libquixcc::ir::Value<> *lhs, const libquixcc::ir::Value<> *rhs);

        const libquixcc::ir::Value<> *lhs;
        const libquixcc::ir::Value<> *rhs;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_MATH_H__