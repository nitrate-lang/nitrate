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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_PARSE_NODES_TYPEDEF_H__
#define __QUIXCC_PARSE_NODES_TYPEDEF_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <llvm/LLVMWrapper.h>
#include <parse/nodes/BasicNodes.h>

namespace libquixcc
{
    class TypedefNode : public DeclNode
    {
    public:
        TypedefNode(TypeNode *orig, const std::string &name) : m_orig(orig), m_name(name) { ntype = NodeType::TypedefNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { throw CodegenException("TypedefNode is not codegen-able"); }

        TypeNode *m_orig;
        std::string m_name;
    };
}

#endif // __QUIXCC_PARSE_NODES_TYPEDEF_H__