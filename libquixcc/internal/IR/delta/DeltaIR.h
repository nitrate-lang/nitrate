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

#include <IR/gamma/GammaIR.h>
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
                Node,
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

                /* Variables */
                Local,
                Global,
                Number,
                String,

                /* Memory */
                Assign,
                Load,
                Index,

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

            class IRDelta : public libquixcc::ir::IRModule<IR::Delta, NodeType::Root>
            {
            protected:
                Result<bool> print_impl(std::ostream &os, bool debug) const override;
                Result<bool> deserialize_impl(std::istream &is) override;
                std::string_view ir_dialect_name_impl() const override;
                unsigned ir_dialect_version_impl() const override;
                std::string_view ir_dialect_family_impl() const override;
                std::string_view ir_dialect_description_impl() const override;
                bool verify_impl() const override;

            public:
                IRDelta(const std::string_view &name) : IRModule<IR::Delta, NodeType::Root>(name) {}
                ~IRDelta() = default;

                bool from_gamma(const std::unique_ptr<libquixcc::ir::gamma::IRGamma> &beta);
            };
        }
    }
}

#endif // __QUIXCC_IR_DELTAIR_H__