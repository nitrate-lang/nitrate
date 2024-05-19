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

#ifndef __QUIXCC_IR_DELTAIR_H__
#define __QUIXCC_IR_DELTAIR_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/QIR.h>
#include <IR/IRModule.h>
#include <IR/Type.h>

namespace libquixcc
{
    namespace ir
    {
        namespace delta
        {
            enum class NodeType
            {
                Root,

                /* Types */
                I1,
                I8,
                I16,
                I32,
                I64,
                I128,
                I256,
                U8,
                U16,
                U32,
                U64,
                U128,
                U256,
                F32,
                F64,
                Void,
                Ptr,
                Packet,
                Array,
                FType,

                /* Variables & Literals*/
                Local,
                Global,
                Number,
                String,
                List,

                /* Memory */
                Assign,
                Member,
                Load,
                Index,
                Ident,

                /* Casting */
                SCast,
                UCast,
                PtrICast,
                IPtrCast,
                Bitcast,

                /* Control Flow */
                IfElse,
                While,
                Jmp,
                Label,
                Ret,
                Call,
                PtrCall,
                Halt,

                /* Blocks */
                Segment,
                Block,

                /* Arithmetic */
                Add,
                Sub,
                Mul,
                Div,
                Mod,
                BitAnd,
                BitOr,
                BitXor,
                BitNot,
                Shl,
                Shr,
                Rotl,
                Rotr,

                /* Comparison */
                Eq,
                Ne,
                Lt,
                Gt,
                Le,
                Ge,

                /* Logical */
                And,
                Or,
                Not,
                Xor,
            };

            class Value : public libquixcc::ir::Node<Delta>
            {
            protected:
                bool print_impl(std::ostream &os, PState &state) const override = 0;
                boost::uuids::uuid hash_impl() const override = 0;
                bool verify_impl() const override = 0;
            };

            class RootNode : public Value
            {
            protected:
                bool print_impl(std::ostream &os, PState &state) const override;
                boost::uuids::uuid hash_impl() const override;
                bool verify_impl() const override;

                RootNode(std::vector<const Value *> children) : children(children)
                {
                    ntype = (int)NodeType::Root;
                }

            public:
                static const RootNode *create(std::vector<const Value *> children = {});

                std::vector<const Value *> children;
            };

            class IRDelta : public libquixcc::ir::IRModule<IR::Delta, const RootNode *>
            {
            protected:
                bool print_impl(std::ostream &os, PState &state) const override;
                std::string_view ir_dialect_name_impl() const override;
                unsigned ir_dialect_version_impl() const override;
                std::string_view ir_dialect_family_impl() const override;
                std::string_view ir_dialect_description_impl() const override;
                bool verify_impl() const override;

            public:
                IRDelta(const std::string_view &name) : IRModule<IR::Delta, const RootNode *>(name) {}
                ~IRDelta() = default;

                bool from_qir(const std::unique_ptr<libquixcc::ir::q::QModule> &qir);
            };
        }
    }
}

#endif // __QUIXCC_IR_DELTAIR_H__