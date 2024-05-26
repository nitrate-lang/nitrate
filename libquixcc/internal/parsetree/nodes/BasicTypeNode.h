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

#ifndef __QUIXCC_PARSE_NODES_INTEGER_H__
#define __QUIXCC_PARSE_NODES_INTEGER_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <llvm/LLVMWrapper.h>
#include <parsetree/nodes/BasicNodes.h>

namespace libquixcc
{
    class ImmMutTypeNode : public TypeNode
    {
        ImmMutTypeNode(TypeNode *type) : m_type(type) { ntype = NodeType::ImmMutTypeNode; }
        static std::map<TypeNode *, ImmMutTypeNode *> m_instances;

    public:
        static ImmMutTypeNode *create(TypeNode *type)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (!m_instances.contains(type))
                m_instances[type] = new ImmMutTypeNode(type);
            return m_instances[type];
        }

        TypeNode *m_type;
    };

    class U8TypeNode : public TypeNode
    {
        U8TypeNode() { ntype = NodeType::U8TypeNode; }
        static U8TypeNode *m_instance;

    public:
        static U8TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new U8TypeNode();
            return m_instance;
        }
    };

    class U16TypeNode : public TypeNode
    {
        U16TypeNode() { ntype = NodeType::U16TypeNode; }
        static U16TypeNode *m_instance;

    public:
        static U16TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new U16TypeNode();
            return m_instance;
        }
    };

    class U32TypeNode : public TypeNode
    {
        U32TypeNode() { ntype = NodeType::U32TypeNode; }
        static U32TypeNode *m_instance;

    public:
        static U32TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new U32TypeNode();
            return m_instance;
        }
    };

    class U64TypeNode : public TypeNode
    {
        U64TypeNode() { ntype = NodeType::U64TypeNode; }
        static U64TypeNode *m_instance;

    public:
        static U64TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new U64TypeNode();
            return m_instance;
        }
    };

    class U128TypeNode : public TypeNode
    {
        U128TypeNode() { ntype = NodeType::U128TypeNode; }
        static U128TypeNode *m_instance;

    public:
        static U128TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new U128TypeNode();
            return m_instance;
        }
    };

    class I8TypeNode : public TypeNode
    {
        I8TypeNode() { ntype = NodeType::I8TypeNode; }
        static I8TypeNode *m_instance;

    public:
        static I8TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new I8TypeNode();
            return m_instance;
        }
    };

    class I16TypeNode : public TypeNode
    {
        I16TypeNode() { ntype = NodeType::I16TypeNode; }
        static I16TypeNode *m_instance;

    public:
        static I16TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new I16TypeNode();
            return m_instance;
        }
    };

    class I32TypeNode : public TypeNode
    {
        I32TypeNode() { ntype = NodeType::I32TypeNode; }
        static I32TypeNode *m_instance;

    public:
        static I32TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new I32TypeNode();
            return m_instance;
        }
    };

    class I64TypeNode : public TypeNode
    {
        I64TypeNode() { ntype = NodeType::I64TypeNode; }
        static I64TypeNode *m_instance;

    public:
        static I64TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new I64TypeNode();
            return m_instance;
        }
    };

    class I128TypeNode : public TypeNode
    {
        I128TypeNode() { ntype = NodeType::I128TypeNode; }
        static I128TypeNode *m_instance;

    public:
        static I128TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new I128TypeNode();
            return m_instance;
        }
    };

    class F32TypeNode : public TypeNode
    {
        F32TypeNode() { ntype = NodeType::F32TypeNode; }
        static F32TypeNode *m_instance;

    public:
        static F32TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new F32TypeNode();
            return m_instance;
        }
    };

    class F64TypeNode : public TypeNode
    {
        F64TypeNode() { ntype = NodeType::F64TypeNode; }
        static F64TypeNode *m_instance;

    public:
        static F64TypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new F64TypeNode();
            return m_instance;
        }
    };

    class BoolTypeNode : public TypeNode
    {
        BoolTypeNode() { ntype = NodeType::BoolTypeNode; }
        static BoolTypeNode *m_instance;

    public:
        static BoolTypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new BoolTypeNode();
            return m_instance;
        }
    };

    class VoidTypeNode : public TypeNode
    {
        VoidTypeNode() { ntype = NodeType::VoidTypeNode; }
        static VoidTypeNode *m_instance;

    public:
        static VoidTypeNode *create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = new VoidTypeNode();
            return m_instance;
        }
    };

    class PointerTypeNode : public TypeNode
    {
        PointerTypeNode(TypeNode *type) : m_type(type) { ntype = NodeType::PointerTypeNode; }
        static std::map<TypeNode *, PointerTypeNode *> m_instances;

    public:
        static PointerTypeNode *create(TypeNode *type)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (!m_instances.contains(type))
                m_instances[type] = new PointerTypeNode(type);
            return m_instances[type];
        }

        TypeNode *m_type;
    };

    class OpaqueTypeNode : public TypeNode
    {
        OpaqueTypeNode(std::string name) : m_name(name) { ntype = NodeType::OpaqueTypeNode; }
        static std::map<std::string, OpaqueTypeNode *> m_instances;

    public:
        static OpaqueTypeNode *create(std::string name)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (!m_instances.contains(name))
                m_instances[name] = new OpaqueTypeNode(name);
            return m_instances[name];
        }

        std::string m_name;
    };
}

#endif // __QUIXCC_PARSE_NODES_INTEGER_H__