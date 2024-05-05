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
    class I1 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I1 *create();
    };

    class I8 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I8 *create();
    };

    class I16 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I16 *create();
    };

    class I32 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I32 *create();
    };

    class I64 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I64 *create();
    };

    class I128 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I128 *create();
    };

    class U8 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U8 *create();
    };

    class U16 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U16 *create();
    };

    class U32 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U32 *create();
    };

    class U64 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U64 *create();
    };

    class U128 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U128 *create();
    };

    class F32 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F32 *create();
    };

    class F64 : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F64 *create();
    };

    class Void : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Void *create();
    };

    class Ptr : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ptr(const Value<Delta> *type) : type(type) {}

    public:
        static const Ptr *create(Value<Delta> *type);

        const Value<Delta> *type;
    };

    class Packet : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Packet(std::vector<std::pair<std::string, const Value<Delta> *>> fields, std::string name) : fields(fields), name(name) {}

    public:
        static const Packet *create(std::vector<std::pair<std::string, const Value<Delta> *>> fields, std::string name);

        std::vector<std::pair<std::string, const Value<Delta> *>> fields;
        std::string name;
    };

    class Array : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Array(Value<Delta> *type, uint64_t size) : type(type), size(size) {}

    public:
        static const Array *create(Value<Delta> *type, uint64_t size);

        const Value<Delta> *type;
        uint64_t size;
    };

    class FType : public Value<Delta>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, bool debug) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        FType(std::vector<const Value<Delta> *> params, const Value<Delta> *ret) : params(params), ret(ret) {}

    public:
        static const FType *create(std::vector<const Value<Delta> *> params, const Value<Delta> *ret);

        std::vector<const Value<Delta> *> params;
        const Value<Delta> *ret;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_TYPE_H__