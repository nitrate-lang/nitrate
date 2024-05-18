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

#include <IR/Q/QIR.h>
#include <IR/Q/Asm.h>
#include <IR/Q/Call.h>
#include <IR/Q/Cast.h>
#include <IR/Q/Control.h>
#include <IR/Q/Function.h>
#include <IR/Q/Ident.h>
#include <IR/Q/Math.h>
#include <IR/Q/OO.h>
#include <IR/Q/Type.h>
#include <IR/Q/Variable.h>
#include <IR/Q/Memory.h>

#include <IR/delta/DeltaIR.h>
#include <IR/delta/Segment.h>
#include <IR/delta/Variable.h>
#include <IR/delta/Type.h>
#include <IR/delta/Control.h>
#include <IR/delta/Cast.h>

#include <core/Logger.h>
#include <mangle/Symbol.h>

#include <stack>
#include <any>
#include <optional>

using namespace libquixcc;
using namespace libquixcc::ir::q;

struct DState
{
    DState()
    {
    }
};

typedef const Value *DValue;

class DResult
{
    std::vector<DValue> m_values;

public:
    DResult() = default;
    DResult(const DValue &value)
    {
        m_values.push_back(value);
    }
    DResult(const std::vector<DValue> &values)
        : m_values(values) {}
    DResult(const std::initializer_list<DValue> &values)
        : m_values(values) {}

    operator bool() const
    {
        if (m_values.empty())
            return false;

        return m_values[0] != nullptr;
    }

    auto operator*() const -> const std::vector<DValue> & { return m_values; }
    auto operator->() const -> const std::vector<DValue> & { return m_values; }
    bool empty() const { return m_values.empty(); }
    DValue operator[](size_t i) const { return m_values.at(i); }

    auto begin() const { return m_values.begin(); }
    auto end() const { return m_values.end(); }
};

static auto conv(const I1 *n, DState &state) -> DResult;
static auto conv(const RootNode *n, DState &state) -> DResult;
static auto conv(const I8 *n, DState &state) -> DResult;
static auto conv(const I16 *n, DState &state) -> DResult;
static auto conv(const I32 *n, DState &state) -> DResult;
static auto conv(const I64 *n, DState &state) -> DResult;
static auto conv(const I128 *n, DState &state) -> DResult;
static auto conv(const U8 *n, DState &state) -> DResult;
static auto conv(const U16 *n, DState &state) -> DResult;
static auto conv(const U32 *n, DState &state) -> DResult;
static auto conv(const U64 *n, DState &state) -> DResult;
static auto conv(const U128 *n, DState &state) -> DResult;
static auto conv(const F32 *n, DState &state) -> DResult;
static auto conv(const F64 *n, DState &state) -> DResult;
static auto conv(const Void *n, DState &state) -> DResult;
static auto conv(const Ptr *n, DState &state) -> DResult;
static auto conv(const Array *n, DState &state) -> DResult;
static auto conv(const Vector *n, DState &state) -> DResult;
static auto conv(const FType *n, DState &state) -> DResult;
static auto conv(const Region *n, DState &state) -> DResult;
static auto conv(const Group *n, DState &state) -> DResult;
static auto conv(const Union *n, DState &state) -> DResult;
static auto conv(const Opaque *n, DState &state) -> DResult;
static auto conv(const Block *n, DState &state) -> DResult;
static auto conv(const Segment *n, DState &state) -> DResult;
static auto conv(const Asm *n, DState &state) -> DResult;
static auto conv(const RegionDef *n, DState &state) -> DResult;
static auto conv(const GroupDef *n, DState &state) -> DResult;
static auto conv(const UnionDef *n, DState &state) -> DResult;
static auto conv(const SCast *n, DState &state) -> DResult;
static auto conv(const UCast *n, DState &state) -> DResult;
static auto conv(const PtrICast *n, DState &state) -> DResult;
static auto conv(const IPtrCast *n, DState &state) -> DResult;
static auto conv(const Bitcast *n, DState &state) -> DResult;
static auto conv(const Call *n, DState &state) -> DResult;
static auto conv(const CallIndirect *n, DState &state) -> DResult;
static auto conv(const IfElse *n, DState &state) -> DResult;
static auto conv(const While *n, DState &state) -> DResult;
static auto conv(const For *n, DState &state) -> DResult;
static auto conv(const Loop *n, DState &state) -> DResult;
static auto conv(const Break *n, DState &state) -> DResult;
static auto conv(const Continue *n, DState &state) -> DResult;
static auto conv(const Ret *n, DState &state) -> DResult;
static auto conv(const Throw *n, DState &state) -> DResult;
static auto conv(const TryCatchFinally *n, DState &state) -> DResult;
static auto conv(const Case *n, DState &state) -> DResult;
static auto conv(const Switch *n, DState &state) -> DResult;
static auto conv(const Ident *n, DState &state) -> DResult;
static auto conv(const Add *n, DState &state) -> DResult;
static auto conv(const Sub *n, DState &state) -> DResult;
static auto conv(const Mul *n, DState &state) -> DResult;
static auto conv(const Div *n, DState &state) -> DResult;
static auto conv(const Mod *n, DState &state) -> DResult;
static auto conv(const BitAnd *n, DState &state) -> DResult;
static auto conv(const BitOr *n, DState &state) -> DResult;
static auto conv(const BitXor *n, DState &state) -> DResult;
static auto conv(const BitNot *n, DState &state) -> DResult;
static auto conv(const Shl *n, DState &state) -> DResult;
static auto conv(const Shr *n, DState &state) -> DResult;
static auto conv(const Rotl *n, DState &state) -> DResult;
static auto conv(const Rotr *n, DState &state) -> DResult;
static auto conv(const Eq *n, DState &state) -> DResult;
static auto conv(const Ne *n, DState &state) -> DResult;
static auto conv(const Lt *n, DState &state) -> DResult;
static auto conv(const Gt *n, DState &state) -> DResult;
static auto conv(const Le *n, DState &state) -> DResult;
static auto conv(const Ge *n, DState &state) -> DResult;
static auto conv(const And *n, DState &state) -> DResult;
static auto conv(const Or *n, DState &state) -> DResult;
static auto conv(const Not *n, DState &state) -> DResult;
static auto conv(const Xor *n, DState &state) -> DResult;
static auto conv(const Local *n, DState &state) -> DResult;
static auto conv(const Global *n, DState &state) -> DResult;
static auto conv(const Number *n, DState &state) -> DResult;
static auto conv(const String *n, DState &state) -> DResult;
static auto conv(const Char *n, DState &state) -> DResult;
static auto conv(const Assign *n, DState &state) -> DResult;

static auto conv(const RootNode *n, DState &state) -> DResult
{
    /// TODO: Implement RootNode
    throw std::runtime_error("DeltaIR translation: RootNode not implemented");
}

static auto conv(const I1 *n, DState &state) -> DResult
{
    /// TODO: Implement I1
    throw std::runtime_error("DeltaIR translation: I1 not implemented");
}

static auto conv(const I8 *n, DState &state) -> DResult
{
    /// TODO: Implement I8
    throw std::runtime_error("DeltaIR translation: I8 not implemented");
}

static auto conv(const I16 *n, DState &state) -> DResult
{
    /// TODO: Implement I16
    throw std::runtime_error("DeltaIR translation: I16 not implemented");
}

static auto conv(const I32 *n, DState &state) -> DResult
{
    /// TODO: Implement I32
    throw std::runtime_error("DeltaIR translation: I32 not implemented");
}

static auto conv(const I64 *n, DState &state) -> DResult
{
    /// TODO: Implement I64
    throw std::runtime_error("DeltaIR translation: I64 not implemented");
}

static auto conv(const I128 *n, DState &state) -> DResult
{
    /// TODO: Implement I128
    throw std::runtime_error("DeltaIR translation: I128 not implemented");
}

static auto conv(const U8 *n, DState &state) -> DResult
{
    /// TODO: Implement U8
    throw std::runtime_error("DeltaIR translation: U8 not implemented");
}

static auto conv(const U16 *n, DState &state) -> DResult
{
    /// TODO: Implement U16
    throw std::runtime_error("DeltaIR translation: U16 not implemented");
}

static auto conv(const U32 *n, DState &state) -> DResult
{
    /// TODO: Implement U32
    throw std::runtime_error("DeltaIR translation: U32 not implemented");
}

static auto conv(const U64 *n, DState &state) -> DResult
{
    /// TODO: Implement U64
    throw std::runtime_error("DeltaIR translation: U64 not implemented");
}

static auto conv(const U128 *n, DState &state) -> DResult
{
    /// TODO: Implement U128
    throw std::runtime_error("DeltaIR translation: U128 not implemented");
}

static auto conv(const F32 *n, DState &state) -> DResult
{
    /// TODO: Implement F32
    throw std::runtime_error("DeltaIR translation: F32 not implemented");
}

static auto conv(const F64 *n, DState &state) -> DResult
{
    /// TODO: Implement F64
    throw std::runtime_error("DeltaIR translation: F64 not implemented");
}

static auto conv(const Void *n, DState &state) -> DResult
{
    /// TODO: Implement Void
    throw std::runtime_error("DeltaIR translation: Void not implemented");
}

static auto conv(const Ptr *n, DState &state) -> DResult
{
    /// TODO: Implement Ptr
    throw std::runtime_error("DeltaIR translation: Ptr not implemented");
}

static auto conv(const Array *n, DState &state) -> DResult
{
    /// TODO: Implement Array
    throw std::runtime_error("DeltaIR translation: Array not implemented");
}

static auto conv(const Vector *n, DState &state) -> DResult
{
    /// TODO: Implement Vector
    throw std::runtime_error("DeltaIR translation: Vector not implemented");
}

static auto conv(const FType *n, DState &state) -> DResult
{
    /// TODO: Implement FType
    throw std::runtime_error("DeltaIR translation: FType not implemented");
}

static auto conv(const Region *n, DState &state) -> DResult
{
    /// TODO: Implement Region
    throw std::runtime_error("DeltaIR translation: Region not implemented");
}

static auto conv(const Group *n, DState &state) -> DResult
{
    /// TODO: Implement Group
    throw std::runtime_error("DeltaIR translation: Group not implemented");
}

static auto conv(const Union *n, DState &state) -> DResult
{
    /// TODO: Implement Union
    throw std::runtime_error("DeltaIR translation: Union not implemented");
}

static auto conv(const Opaque *n, DState &state) -> DResult
{
    /// TODO: Implement Opaque
    throw std::runtime_error("DeltaIR translation: Opaque not implemented");
}

static auto conv(const Block *n, DState &state) -> DResult
{
    /// TODO: Implement Block
    throw std::runtime_error("DeltaIR translation: Block not implemented");
}

static auto conv(const Segment *n, DState &state) -> DResult
{
    /// TODO: Implement Segment
    throw std::runtime_error("DeltaIR translation: Segment not implemented");
}

static auto conv(const Asm *n, DState &state) -> DResult
{
    /// TODO: Implement Asm
    throw std::runtime_error("DeltaIR translation: Asm not implemented");
}

static auto conv(const RegionDef *n, DState &state) -> DResult
{
    /// TODO: Implement RegionDef
    throw std::runtime_error("DeltaIR translation: RegionDef not implemented");
}

static auto conv(const GroupDef *n, DState &state) -> DResult
{
    /// TODO: Implement GroupDef
    throw std::runtime_error("DeltaIR translation: GroupDef not implemented");
}

static auto conv(const UnionDef *n, DState &state) -> DResult
{
    /// TODO: Implement UnionDef
    throw std::runtime_error("DeltaIR translation: UnionDef not implemented");
}

static auto conv(const SCast *n, DState &state) -> DResult
{
    /// TODO: Implement SCast
    throw std::runtime_error("DeltaIR translation: SCast not implemented");
}

static auto conv(const UCast *n, DState &state) -> DResult
{
    /// TODO: Implement UCast
    throw std::runtime_error("DeltaIR translation: UCast not implemented");
}

static auto conv(const PtrICast *n, DState &state) -> DResult
{
    /// TODO: Implement PtrICast
    throw std::runtime_error("DeltaIR translation: PtrICast not implemented");
}

static auto conv(const IPtrCast *n, DState &state) -> DResult
{
    /// TODO: Implement IPtrCast
    throw std::runtime_error("DeltaIR translation: IPtrCast not implemented");
}

static auto conv(const Bitcast *n, DState &state) -> DResult
{
    /// TODO: Implement Bitcast
    throw std::runtime_error("DeltaIR translation: Bitcast not implemented");
}

static auto conv(const Call *n, DState &state) -> DResult
{
    /// TODO: Implement Call
    throw std::runtime_error("DeltaIR translation: Call not implemented");
}

static auto conv(const CallIndirect *n, DState &state) -> DResult
{
    /// TODO: Implement CallIndirect
    throw std::runtime_error("DeltaIR translation: CallIndirect not implemented");
}

static auto conv(const IfElse *n, DState &state) -> DResult
{
    /// TODO: Implement IfElse
    throw std::runtime_error("DeltaIR translation: IfElse not implemented");
}

static auto conv(const While *n, DState &state) -> DResult
{
    /// TODO: Implement While
    throw std::runtime_error("DeltaIR translation: While not implemented");
}

static auto conv(const For *n, DState &state) -> DResult
{
    /// TODO: Implement For
    throw std::runtime_error("DeltaIR translation: For not implemented");
}

static auto conv(const Loop *n, DState &state) -> DResult
{
    /// TODO: Implement Loop
    throw std::runtime_error("DeltaIR translation: Loop not implemented");
}

static auto conv(const Break *n, DState &state) -> DResult
{
    /// TODO: Implement Break
    throw std::runtime_error("DeltaIR translation: Break not implemented");
}

static auto conv(const Continue *n, DState &state) -> DResult
{
    /// TODO: Implement Continue
    throw std::runtime_error("DeltaIR translation: Continue not implemented");
}

static auto conv(const Ret *n, DState &state) -> DResult
{
    /// TODO: Implement Ret
    throw std::runtime_error("DeltaIR translation: Ret not implemented");
}

static auto conv(const Throw *n, DState &state) -> DResult
{
    /// TODO: Implement Throw
    throw std::runtime_error("DeltaIR translation: Throw not implemented");
}

static auto conv(const TryCatchFinally *n, DState &state) -> DResult
{
    /// TODO: Implement TryCatchFinally
    throw std::runtime_error("DeltaIR translation: TryCatchFinally not implemented");
}

static auto conv(const Case *n, DState &state) -> DResult
{
    /// TODO: Implement Case
    throw std::runtime_error("DeltaIR translation: Case not implemented");
}

static auto conv(const Switch *n, DState &state) -> DResult
{
    /// TODO: Implement Switch
    throw std::runtime_error("DeltaIR translation: Switch not implemented");
}

static auto conv(const Ident *n, DState &state) -> DResult
{
    /// TODO: Implement Ident
    throw std::runtime_error("DeltaIR translation: Ident not implemented");
}

static auto conv(const Add *n, DState &state) -> DResult
{
    /// TODO: Implement Add
    throw std::runtime_error("DeltaIR translation: Add not implemented");
}

static auto conv(const Sub *n, DState &state) -> DResult
{
    /// TODO: Implement Sub
    throw std::runtime_error("DeltaIR translation: Sub not implemented");
}

static auto conv(const Mul *n, DState &state) -> DResult
{
    /// TODO: Implement Mul
    throw std::runtime_error("DeltaIR translation: Mul not implemented");
}

static auto conv(const Div *n, DState &state) -> DResult
{
    /// TODO: Implement Div
    throw std::runtime_error("DeltaIR translation: Div not implemented");
}

static auto conv(const Mod *n, DState &state) -> DResult
{
    /// TODO: Implement Mod
    throw std::runtime_error("DeltaIR translation: Mod not implemented");
}

static auto conv(const BitAnd *n, DState &state) -> DResult
{
    /// TODO: Implement BitAnd
    throw std::runtime_error("DeltaIR translation: BitAnd not implemented");
}

static auto conv(const BitOr *n, DState &state) -> DResult
{
    /// TODO: Implement BitOr
    throw std::runtime_error("DeltaIR translation: BitOr not implemented");
}

static auto conv(const BitXor *n, DState &state) -> DResult
{
    /// TODO: Implement BitXor
    throw std::runtime_error("DeltaIR translation: BitXor not implemented");
}

static auto conv(const BitNot *n, DState &state) -> DResult
{
    /// TODO: Implement BitNot
    throw std::runtime_error("DeltaIR translation: BitNot not implemented");
}

static auto conv(const Shl *n, DState &state) -> DResult
{
    /// TODO: Implement Shl
    throw std::runtime_error("DeltaIR translation: Shl not implemented");
}

static auto conv(const Shr *n, DState &state) -> DResult
{
    /// TODO: Implement Shr
    throw std::runtime_error("DeltaIR translation: Shr not implemented");
}

static auto conv(const Rotl *n, DState &state) -> DResult
{
    /// TODO: Implement Rotl
    throw std::runtime_error("DeltaIR translation: Rotl not implemented");
}

static auto conv(const Rotr *n, DState &state) -> DResult
{
    /// TODO: Implement Rotr
    throw std::runtime_error("DeltaIR translation: Rotr not implemented");
}

static auto conv(const Eq *n, DState &state) -> DResult
{
    /// TODO: Implement Eq
    throw std::runtime_error("DeltaIR translation: Eq not implemented");
}

static auto conv(const Ne *n, DState &state) -> DResult
{
    /// TODO: Implement Ne
    throw std::runtime_error("DeltaIR translation: Ne not implemented");
}

static auto conv(const Lt *n, DState &state) -> DResult
{
    /// TODO: Implement Lt
    throw std::runtime_error("DeltaIR translation: Lt not implemented");
}

static auto conv(const Gt *n, DState &state) -> DResult
{
    /// TODO: Implement Gt
    throw std::runtime_error("DeltaIR translation: Gt not implemented");
}

static auto conv(const Le *n, DState &state) -> DResult
{
    /// TODO: Implement Le
    throw std::runtime_error("DeltaIR translation: Le not implemented");
}

static auto conv(const Ge *n, DState &state) -> DResult
{
    /// TODO: Implement Ge
    throw std::runtime_error("DeltaIR translation: Ge not implemented");
}

static auto conv(const And *n, DState &state) -> DResult
{
    /// TODO: Implement And
    throw std::runtime_error("DeltaIR translation: And not implemented");
}

static auto conv(const Or *n, DState &state) -> DResult
{
    /// TODO: Implement Or
    throw std::runtime_error("DeltaIR translation: Or not implemented");
}

static auto conv(const Not *n, DState &state) -> DResult
{
    /// TODO: Implement Not
    throw std::runtime_error("DeltaIR translation: Not not implemented");
}

static auto conv(const Xor *n, DState &state) -> DResult
{
    /// TODO: Implement Xor
    throw std::runtime_error("DeltaIR translation: Xor not implemented");
}

static auto conv(const Local *n, DState &state) -> DResult
{
    /// TODO: Implement Local
    throw std::runtime_error("DeltaIR translation: Local not implemented");
}

static auto conv(const Global *n, DState &state) -> DResult
{
    /// TODO: Implement Global
    throw std::runtime_error("DeltaIR translation: Global not implemented");
}

static auto conv(const Number *n, DState &state) -> DResult
{
    /// TODO: Implement Number
    throw std::runtime_error("DeltaIR translation: Number not implemented");
}

static auto conv(const String *n, DState &state) -> DResult
{
    /// TODO: Implement String
    throw std::runtime_error("DeltaIR translation: String not implemented");
}

static auto conv(const Char *n, DState &state) -> DResult
{
    /// TODO: Implement Char
    throw std::runtime_error("DeltaIR translation: Char not implemented");
}

static auto conv(const Assign *n, DState &state) -> DResult
{
    /// TODO: Implement Assign
    throw std::runtime_error("DeltaIR translation: Assign not implemented");
}

static auto conv(const Value *n, DState &state) -> DResult
{
    DResult r;

    switch ((libquixcc::ir::q::NodeType)n->ntype)
    {
    case libquixcc::ir::q::NodeType::Root:
        r = conv(n->as<RootNode>(), state);
        break;

    case libquixcc::ir::q::NodeType::I1:
        r = conv(n->as<I1>(), state);
        break;

    case libquixcc::ir::q::NodeType::I8:
        r = conv(n->as<I8>(), state);
        break;

    case libquixcc::ir::q::NodeType::I16:
        r = conv(n->as<I16>(), state);
        break;

    case libquixcc::ir::q::NodeType::I32:
        r = conv(n->as<I32>(), state);
        break;

    case libquixcc::ir::q::NodeType::I64:
        r = conv(n->as<I64>(), state);
        break;

    case libquixcc::ir::q::NodeType::I128:
        r = conv(n->as<I128>(), state);
        break;

    case libquixcc::ir::q::NodeType::U8:
        r = conv(n->as<U8>(), state);
        break;

    case libquixcc::ir::q::NodeType::U16:
        r = conv(n->as<U16>(), state);
        break;

    case libquixcc::ir::q::NodeType::U32:
        r = conv(n->as<U32>(), state);
        break;

    case libquixcc::ir::q::NodeType::U64:
        r = conv(n->as<U64>(), state);
        break;

    case libquixcc::ir::q::NodeType::U128:
        r = conv(n->as<U128>(), state);
        break;

    case libquixcc::ir::q::NodeType::F32:
        r = conv(n->as<F32>(), state);
        break;

    case libquixcc::ir::q::NodeType::F64:
        r = conv(n->as<F64>(), state);
        break;

    case libquixcc::ir::q::NodeType::Void:
        r = conv(n->as<Void>(), state);
        break;

    case libquixcc::ir::q::NodeType::Ptr:
        r = conv(n->as<Ptr>(), state);
        break;

    case libquixcc::ir::q::NodeType::Array:
        r = conv(n->as<Array>(), state);
        break;

    case libquixcc::ir::q::NodeType::Vector:
        r = conv(n->as<Vector>(), state);
        break;

    case libquixcc::ir::q::NodeType::FType:
        r = conv(n->as<FType>(), state);
        break;

    case libquixcc::ir::q::NodeType::Region:
        r = conv(n->as<Region>(), state);
        break;

    case libquixcc::ir::q::NodeType::Group:
        r = conv(n->as<Group>(), state);
        break;

    case libquixcc::ir::q::NodeType::Union:
        r = conv(n->as<Union>(), state);
        break;

    case libquixcc::ir::q::NodeType::Opaque:
        r = conv(n->as<Opaque>(), state);
        break;

    case libquixcc::ir::q::NodeType::Block:
        r = conv(n->as<Block>(), state);
        break;

    case libquixcc::ir::q::NodeType::Segment:
        r = conv(n->as<Segment>(), state);
        break;

    case libquixcc::ir::q::NodeType::Asm:
        r = conv(n->as<Asm>(), state);
        break;

    case libquixcc::ir::q::NodeType::RegionDef:
        r = conv(n->as<RegionDef>(), state);
        break;

    case libquixcc::ir::q::NodeType::GroupDef:
        r = conv(n->as<GroupDef>(), state);
        break;

    case libquixcc::ir::q::NodeType::UnionDef:
        r = conv(n->as<UnionDef>(), state);
        break;

    case libquixcc::ir::q::NodeType::SCast:
        r = conv(n->as<SCast>(), state);
        break;

    case libquixcc::ir::q::NodeType::UCast:
        r = conv(n->as<UCast>(), state);
        break;

    case libquixcc::ir::q::NodeType::PtrICast:
        r = conv(n->as<PtrICast>(), state);
        break;

    case libquixcc::ir::q::NodeType::IPtrCast:
        r = conv(n->as<IPtrCast>(), state);
        break;

    case libquixcc::ir::q::NodeType::Bitcast:
        r = conv(n->as<Bitcast>(), state);
        break;

    case libquixcc::ir::q::NodeType::Call:
        r = conv(n->as<Call>(), state);
        break;

    case libquixcc::ir::q::NodeType::CallIndirect:
        r = conv(n->as<CallIndirect>(), state);
        break;

    case libquixcc::ir::q::NodeType::IfElse:
        r = conv(n->as<IfElse>(), state);
        break;

    case libquixcc::ir::q::NodeType::While:
        r = conv(n->as<While>(), state);
        break;

    case libquixcc::ir::q::NodeType::For:
        r = conv(n->as<For>(), state);
        break;

    case libquixcc::ir::q::NodeType::Loop:
        r = conv(n->as<Loop>(), state);
        break;

    case libquixcc::ir::q::NodeType::Break:
        r = conv(n->as<Break>(), state);
        break;

    case libquixcc::ir::q::NodeType::Continue:
        r = conv(n->as<Continue>(), state);
        break;

    case libquixcc::ir::q::NodeType::Ret:
        r = conv(n->as<Ret>(), state);
        break;

    case libquixcc::ir::q::NodeType::Throw:
        r = conv(n->as<Throw>(), state);
        break;

    case libquixcc::ir::q::NodeType::TryCatchFinally:
        r = conv(n->as<TryCatchFinally>(), state);
        break;

    case libquixcc::ir::q::NodeType::Case:
        r = conv(n->as<Case>(), state);
        break;

    case libquixcc::ir::q::NodeType::Switch:
        r = conv(n->as<Switch>(), state);
        break;

    case libquixcc::ir::q::NodeType::Ident:
        r = conv(n->as<Ident>(), state);
        break;

    case libquixcc::ir::q::NodeType::Add:
        r = conv(n->as<Add>(), state);
        break;

    case libquixcc::ir::q::NodeType::Sub:
        r = conv(n->as<Sub>(), state);
        break;

    case libquixcc::ir::q::NodeType::Mul:
        r = conv(n->as<Mul>(), state);
        break;

    case libquixcc::ir::q::NodeType::Div:
        r = conv(n->as<Div>(), state);
        break;

    case libquixcc::ir::q::NodeType::Mod:
        r = conv(n->as<Mod>(), state);
        break;

    case libquixcc::ir::q::NodeType::BitAnd:
        r = conv(n->as<BitAnd>(), state);
        break;

    case libquixcc::ir::q::NodeType::BitOr:
        r = conv(n->as<BitOr>(), state);
        break;

    case libquixcc::ir::q::NodeType::BitXor:
        r = conv(n->as<BitXor>(), state);
        break;

    case libquixcc::ir::q::NodeType::BitNot:
        r = conv(n->as<BitNot>(), state);
        break;

    case libquixcc::ir::q::NodeType::Shl:
        r = conv(n->as<Shl>(), state);
        break;

    case libquixcc::ir::q::NodeType::Shr:
        r = conv(n->as<Shr>(), state);
        break;

    case libquixcc::ir::q::NodeType::Rotl:
        r = conv(n->as<Rotl>(), state);
        break;

    case libquixcc::ir::q::NodeType::Rotr:
        r = conv(n->as<Rotr>(), state);
        break;

    case libquixcc::ir::q::NodeType::Eq:
        r = conv(n->as<Eq>(), state);
        break;

    case libquixcc::ir::q::NodeType::Ne:
        r = conv(n->as<Ne>(), state);
        break;

    case libquixcc::ir::q::NodeType::Lt:
        r = conv(n->as<Lt>(), state);
        break;

    case libquixcc::ir::q::NodeType::Gt:
        r = conv(n->as<Gt>(), state);
        break;

    case libquixcc::ir::q::NodeType::Le:
        r = conv(n->as<Le>(), state);
        break;

    case libquixcc::ir::q::NodeType::Ge:
        r = conv(n->as<Ge>(), state);
        break;

    case libquixcc::ir::q::NodeType::And:
        r = conv(n->as<And>(), state);
        break;

    case libquixcc::ir::q::NodeType::Or:
        r = conv(n->as<Or>(), state);
        break;

    case libquixcc::ir::q::NodeType::Not:
        r = conv(n->as<Not>(), state);
        break;

    case libquixcc::ir::q::NodeType::Xor:
        r = conv(n->as<Xor>(), state);
        break;

    case libquixcc::ir::q::NodeType::Local:
        r = conv(n->as<Local>(), state);
        break;

    case libquixcc::ir::q::NodeType::Global:
        r = conv(n->as<Global>(), state);
        break;

    case libquixcc::ir::q::NodeType::Number:
        r = conv(n->as<Number>(), state);
        break;

    case libquixcc::ir::q::NodeType::String:
        r = conv(n->as<String>(), state);
        break;

    case libquixcc::ir::q::NodeType::Char:
        r = conv(n->as<Char>(), state);
        break;

    case libquixcc::ir::q::NodeType::Assign:
        r = conv(n->as<Assign>(), state);
        break;

    default:
        throw std::runtime_error("DIR translation: Unknown node type");
    }
    return r;

    (void)conv(n->as<Value>(), state);
}

bool libquixcc::ir::delta::IRDelta::from_qir(const std::unique_ptr<libquixcc::ir::q::QModule> &ir)
{
    LOG(DEBUG) << "Translating QUIX intermediate representation to DeltaIR" << std::endl;

    DState state;
    auto r = conv(ir->root(), state);

    m_root = RootNode::create(r[0]->as<Block>()->stmts);

    if (!verify())
    {
        LOG(FATAL) << "Failed to qualify QUIX intermediate representation" << std::endl;
        return false;
    }

    LOG(DEBUG) << this->to_string() << std::endl;

    LOG(DEBUG) << "Successfully translated QUIX intermediate representation to DeltaIR" << std::endl;

    return true;
}