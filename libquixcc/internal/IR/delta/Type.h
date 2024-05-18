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
    class Type : public Value
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override = 0;
        boost::uuids::uuid hash_impl() const override = 0;
        bool verify_impl() const override = 0;

    public:
        virtual size_t bitcount() const = 0;
        size_t size() const;
        bool is_ptr() const;
        bool is_integer() const;
        bool is_float() const;
        bool is_void() const;
        bool is_signed() const;
        bool is_unsigned() const;
    };

    class I1 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I1 *create();
        size_t bitcount() const override { return 1; }
    };

    class I8 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I8 *create();
        size_t bitcount() const override { return 8; }
    };

    class I16 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I16 *create();
        size_t bitcount() const override { return 16; }
    };

    class I32 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I32 *create();
        size_t bitcount() const override { return 32; }
    };

    class I64 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I64 *create();
        size_t bitcount() const override { return 64; }
    };

    class I128 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I128 *create();
        size_t bitcount() const override { return 128; }
    };

    class U8 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U8 *create();
        size_t bitcount() const override { return 8; }
    };

    class U16 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U16 *create();
        size_t bitcount() const override { return 16; }
    };

    class U32 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U32 *create();
        size_t bitcount() const override { return 32; }
    };

    class U64 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U64 *create();
        size_t bitcount() const override { return 64; }
    };

    class U128 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U128 *create();
        size_t bitcount() const override { return 128; }
    };

    class F32 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F32 *create();
        size_t bitcount() const override { return 32; }
    };

    class F64 : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F64 *create();
        size_t bitcount() const override { return 64; }
    };

    class Void : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Void *create();
        size_t bitcount() const override { return 0; }
    };

    class Ptr : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ptr(const Type *type) : type(type) {}

    public:
        static const Ptr *create(const Type *type);
        size_t bitcount() const override { throw std::runtime_error("Pointer size is not known"); }

        const Type *type;
    };

    class Packet : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Packet(std::vector<std::pair<std::string, const Type *>> fields, std::string name) : fields(fields), name(name) {}

    public:
        static const Packet *create(std::vector<std::pair<std::string, const Type *>> fields, std::string name);
        size_t bitcount() const override
        {
            size_t count = 0;
            for (auto &field : fields)
                count += field.second->bitcount();
            return count;
        }

        std::vector<std::pair<std::string, const Type *>> fields;
        std::string name;
    };

    class Array : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Array(const Type *type, uint64_t size) : type(type), size(size) {}

    public:
        static const Array *create(const Type *type, uint64_t size);
        size_t bitcount() const override { return type->bitcount() * size; }

        const Type *type;
        uint64_t size;
    };

    class FType : public Type
    {
    protected:
        bool print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        FType(std::vector<const Type *> params, const Type *ret) : params(params), ret(ret) {}

    public:
        static const FType *create(std::vector<const Type *> params, const Type *ret);
        size_t bitcount() const override { return Ptr::create(Void::create())->bitcount(); }

        std::vector<const Type *> params;
        const Type *ret;
    };
}

#endif // __QUIXCC_IR_DELTA_NODES_TYPE_H__