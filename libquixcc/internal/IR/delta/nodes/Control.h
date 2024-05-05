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

namespace libquixcc::ir::delta
{
    class IfElse : public libquixcc::ir::Value<NodeType::IfElse>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const IfElse *create(const libquixcc::ir::Value<> *cond, const libquixcc::ir::Value<> *then, const libquixcc::ir::Value<> *els);

        const libquixcc::ir::Value<> *cond;
        const libquixcc::ir::Value<> *then;
        const libquixcc::ir::Value<> *els;
    };

    class While : public libquixcc::ir::Value<NodeType::While>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const While *create(const libquixcc::ir::Value<> *cond, const libquixcc::ir::Value<> *body);

        const libquixcc::ir::Value<> *cond;
        const libquixcc::ir::Value<> *body;
    };

    class Jmp : public libquixcc::ir::Value<NodeType::Jmp>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Jmp *create(std::string target);

        std::string target;
    };

    class Label : public libquixcc::ir::Value<NodeType::Label>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Label *create(std::string name);

        std::string name;
    };

    class Ret : public libquixcc::ir::Value<NodeType::Ret>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Ret *create(const libquixcc::ir::Value<> *value);

        const libquixcc::ir::Value<> *value;
    };

    class Call : public libquixcc::ir::Value<NodeType::Call>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Call *create(const libquixcc::ir::Value<NodeType::Segment> *callee, std::vector<const libquixcc::ir::Value<> *> args);

        const libquixcc::ir::Value<NodeType::Segment> *callee;
        std::vector<const libquixcc::ir::Value<> *> args;
    };

    class PtrCall : public libquixcc::ir::Value<NodeType::PtrCall>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const PtrCall *create(const libquixcc::ir::Value<> *callee, std::vector<const libquixcc::ir::Value<> *> args);

        const libquixcc::ir::Value<> *callee;
        std::vector<const libquixcc::ir::Value<> *> args;
    };

    class Halt : public libquixcc::ir::Value<NodeType::Halt>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Halt *create();
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_CONTROL_H__