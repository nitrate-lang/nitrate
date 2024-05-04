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

#ifndef __QUIXCC_IR_DELTA_NODES_TYPE_H__
#define __QUIXCC_IR_DELTA_NODES_TYPE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>

namespace libquixcc::ir::delta
{
    class I1 : public libquixcc::ir::Value<NodeType::I1>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I1 *create();
    };

    class I8 : public libquixcc::ir::Value<NodeType::I8>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I8 *create();
    };

    class I16 : public libquixcc::ir::Value<NodeType::I16>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I16 *create();
    };

    class I32 : public libquixcc::ir::Value<NodeType::I32>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I32 *create();
    };

    class I64 : public libquixcc::ir::Value<NodeType::I64>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I64 *create();
    };

    class I128 : public libquixcc::ir::Value<NodeType::I128>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I128 *create();
    };

    class I256 : public libquixcc::ir::Value<NodeType::I256>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I256 *create();
    };

    class U8 : public libquixcc::ir::Value<NodeType::U8>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U8 *create();
    };

    class U16 : public libquixcc::ir::Value<NodeType::U16>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U16 *create();
    };

    class U32 : public libquixcc::ir::Value<NodeType::U32>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U32 *create();
    };

    class U64 : public libquixcc::ir::Value<NodeType::U64>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U64 *create();
    };

    class U128 : public libquixcc::ir::Value<NodeType::U128>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U128 *create();
    };

    class U256 : public libquixcc::ir::Value<NodeType::U256>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U256 *create();
    };

    class F32 : public libquixcc::ir::Value<NodeType::F32>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F32 *create();
    };

    class F64 : public libquixcc::ir::Value<NodeType::F64>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F64 *create();
    };

    class Void : public libquixcc::ir::Value<NodeType::Void>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Void *create();
    };

    class Ptr : public libquixcc::ir::Value<NodeType::Ptr>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Ptr *create(libquixcc::ir::Value<> *type);

        const libquixcc::ir::Value<> *type;
    };

    class Packet : public libquixcc::ir::Value<NodeType::Packet>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Packet *create(std::vector<std::pair<std::string, const libquixcc::ir::Value<> *>> fields, std::string name);

        std::vector<std::pair<std::string, const libquixcc::ir::Value<> *>> fields;
        std::string name;
    };

    class Array : public libquixcc::ir::Value<NodeType::Array>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        Result<bool> deserialize_impl(std::istream &is) override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Array *create(libquixcc::ir::Value<> *type, uint64_t size);

        const libquixcc::ir::Value<> *type;
        uint64_t size;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_TYPE_H__