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

#ifndef __QUIXCC_IR_Q_NODES_TYPE_H__
#define __QUIXCC_IR_Q_NODES_TYPE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/QIR.h>

namespace libquixcc::ir::q
{
    class I1 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I1 *create();
    };

    class I8 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I8 *create();
    };

    class I16 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I16 *create();
    };

    class I32 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I32 *create();
    };

    class I64 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I64 *create();
    };

    class I128 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const I128 *create();
    };

    class U8 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U8 *create();
    };

    class U16 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U16 *create();
    };

    class U32 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U32 *create();
    };

    class U64 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U64 *create();
    };

    class U128 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const U128 *create();
    };

    class F32 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F32 *create();
    };

    class F64 : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const F64 *create();
    };

    class Void : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

    public:
        static const Void *create();
    };

    class Ptr : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Ptr(const Value<Q> *type) : type(type) {}

    public:
        static const Ptr *create(Value<Q> *type);

        const Value<Q> *type;
    };

    class Array : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Array(Value<Q> *type, uint64_t size) : type(type), size(size) {}

    public:
        static const Array *create(Value<Q> *type, uint64_t size);

        const Value<Q> *type;
        uint64_t size;
    };

    class FType : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        FType(std::vector<const Value<Q> *> params, const Value<Q> *ret, bool variadic, bool pure, bool thread_safe, bool foreign, bool nothrow) : params(params), ret(ret), m_variadic(variadic), m_pure(pure), m_thread_safe(thread_safe), m_foreign(foreign), m_nothrow(nothrow) {}

    public:
        static const FType *create(std::vector<const Value<Q> *> params, const Value<Q> *ret, bool variadic = false, bool pure = false, bool thread_safe = false, bool foreign = false, bool nothrow = false);

        std::vector<const Value<Q> *> params;
        const Value<Q> *ret;
        bool m_variadic;
        bool m_pure;
        bool m_thread_safe;
        bool m_foreign;
        bool m_nothrow;
    };

    class Region : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Region(std::vector<std::pair<std::string, const Value<Q> *>> fields) : fields(fields) {}

    public:
        static const Region *create(std::vector<std::pair<std::string, const Value<Q> *>> fields);

        std::vector<std::pair<std::string, const Value<Q> *>> fields;
    };

    class Group : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Group(std::unordered_map<std::string, const Value<Q> *> fields) : fields(fields) {}

    public:
        static const Group *create(std::unordered_map<std::string, const Value<Q> *> fields);

        std::unordered_map<std::string, const Value<Q> *> fields;
    };

    class Union : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Union(std::unordered_map<std::string, const Value<Q> *> fields) : fields(fields) {}

    public:
        static const Union *create(std::unordered_map<std::string, const Value<Q> *> fields);

        std::unordered_map<std::string, const Value<Q> *> fields;
    };

    class Opaque : public Value<Q>
    {
    protected:
        Result<bool> print_impl(std::ostream &os, PState &state) const override;
        boost::uuids::uuid hash_impl() const override;
        bool verify_impl() const override;

        Opaque(std::string name) : name(name) {}

    public:
        static const Opaque *create(std::string name);

        std::string name;
    };
}

#endif // __QUIXCC_IR_Q_NODES_TYPE_H__