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

#define QUIXCC_INTERNAL

#include <mutate/Routine.h>
#include <error/Message.h>
#include <mutex>
#include <set>
#include <quixcc.h>
#include <iostream>
#include <algorithm>

using namespace libquixcc;

void libquixcc::mutate::ExtrapolateEnumFields(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(ParseNodePreorderVisitor(
        [job](std::string _namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            if ((*node)->ntype != NodeType::EnumDefNode)
                return;

            auto def = std::static_pointer_cast<libquixcc::EnumDefNode>(*node);

            std::shared_ptr<ConstExprNode> last;

            for (size_t i = 0; i < def->m_fields.size(); i++)
            {
                if (def->m_fields[i]->m_value)
                {
                    last = def->m_fields[i]->m_value;
                    continue;
                }
                else if (i == 0)
                {
                    def->m_fields[i]->m_value = last = IntegerLiteralNode::create("0");
                    continue;
                }

                last = std::make_shared<ConstBinaryExprNode>(
                    Operator::Plus, last, IntegerLiteralNode::create("1"));

                def->m_fields[i]->m_value = last;
            }
        },
        job->m_inner.prefix));
}