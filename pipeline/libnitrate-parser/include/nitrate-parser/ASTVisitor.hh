////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __NITRATE_AST_VISTOR_H__
#define __NITRATE_AST_VISTOR_H__

#include <cassert>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTFwd.hh>

namespace ncc::parse {
  class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;

    virtual void Visit(FlowPtr<NamedTy> n) = 0;
    virtual void Visit(FlowPtr<InferTy> n) = 0;
    virtual void Visit(FlowPtr<TemplateType> n) = 0;
    virtual void Visit(FlowPtr<U1> n) = 0;
    virtual void Visit(FlowPtr<U8> n) = 0;
    virtual void Visit(FlowPtr<U16> n) = 0;
    virtual void Visit(FlowPtr<U32> n) = 0;
    virtual void Visit(FlowPtr<U64> n) = 0;
    virtual void Visit(FlowPtr<U128> n) = 0;
    virtual void Visit(FlowPtr<I8> n) = 0;
    virtual void Visit(FlowPtr<I16> n) = 0;
    virtual void Visit(FlowPtr<I32> n) = 0;
    virtual void Visit(FlowPtr<I64> n) = 0;
    virtual void Visit(FlowPtr<I128> n) = 0;
    virtual void Visit(FlowPtr<F16> n) = 0;
    virtual void Visit(FlowPtr<F32> n) = 0;
    virtual void Visit(FlowPtr<F64> n) = 0;
    virtual void Visit(FlowPtr<F128> n) = 0;
    virtual void Visit(FlowPtr<VoidTy> n) = 0;
    virtual void Visit(FlowPtr<PtrTy> n) = 0;
    virtual void Visit(FlowPtr<OpaqueTy> n) = 0;
    virtual void Visit(FlowPtr<TupleTy> n) = 0;
    virtual void Visit(FlowPtr<ArrayTy> n) = 0;
    virtual void Visit(FlowPtr<RefTy> n) = 0;
    virtual void Visit(FlowPtr<FuncTy> n) = 0;
    virtual void Visit(FlowPtr<Unary> n) = 0;
    virtual void Visit(FlowPtr<Binary> n) = 0;
    virtual void Visit(FlowPtr<Integer> n) = 0;
    virtual void Visit(FlowPtr<Float> n) = 0;
    virtual void Visit(FlowPtr<Boolean> n) = 0;
    virtual void Visit(FlowPtr<String> n) = 0;
    virtual void Visit(FlowPtr<Character> n) = 0;
    virtual void Visit(FlowPtr<Null> n) = 0;
    virtual void Visit(FlowPtr<Undefined> n) = 0;
    virtual void Visit(FlowPtr<Call> n) = 0;
    virtual void Visit(FlowPtr<TemplateCall> n) = 0;
    virtual void Visit(FlowPtr<Import> n) = 0;
    virtual void Visit(FlowPtr<List> n) = 0;
    virtual void Visit(FlowPtr<Assoc> n) = 0;
    virtual void Visit(FlowPtr<Index> n) = 0;
    virtual void Visit(FlowPtr<Slice> n) = 0;
    virtual void Visit(FlowPtr<FString> n) = 0;
    virtual void Visit(FlowPtr<Identifier> n) = 0;
    virtual void Visit(FlowPtr<Block> n) = 0;
    virtual void Visit(FlowPtr<Variable> n) = 0;
    virtual void Visit(FlowPtr<Assembly> n) = 0;
    virtual void Visit(FlowPtr<If> n) = 0;
    virtual void Visit(FlowPtr<While> n) = 0;
    virtual void Visit(FlowPtr<For> n) = 0;
    virtual void Visit(FlowPtr<Foreach> n) = 0;
    virtual void Visit(FlowPtr<Break> n) = 0;
    virtual void Visit(FlowPtr<Continue> n) = 0;
    virtual void Visit(FlowPtr<Return> n) = 0;
    virtual void Visit(FlowPtr<ReturnIf> n) = 0;
    virtual void Visit(FlowPtr<Case> n) = 0;
    virtual void Visit(FlowPtr<Switch> n) = 0;
    virtual void Visit(FlowPtr<Typedef> n) = 0;
    virtual void Visit(FlowPtr<Function> n) = 0;
    virtual void Visit(FlowPtr<Struct> n) = 0;
    virtual void Visit(FlowPtr<Enum> n) = 0;
    virtual void Visit(FlowPtr<Scope> n) = 0;
    virtual void Visit(FlowPtr<Export> n) = 0;

    template <typename T>
    void Dispatch(FlowPtr<T> n) {
      switch (n->GetKind()) {
        case AST_DISCARDED: {
          // This node is to be ignored.
          break;
        }

        case AST_eBIN: {
          Visit(n.template As<Binary>());
          break;
        }
        case AST_eUNARY: {
          Visit(n.template As<Unary>());
          break;
        }
        case AST_eINT: {
          Visit(n.template As<Integer>());
          break;
        }
        case AST_eFLOAT: {
          Visit(n.template As<Float>());
          break;
        }
        case AST_eSTRING: {
          Visit(n.template As<String>());
          break;
        }
        case AST_eCHAR: {
          Visit(n.template As<Character>());
          break;
        }
        case AST_eBOOL: {
          Visit(n.template As<Boolean>());
          break;
        }
        case AST_eNULL: {
          Visit(n.template As<Null>());
          break;
        }
        case AST_eUNDEF: {
          Visit(n.template As<Undefined>());
          break;
        }
        case AST_eCALL: {
          Visit(n.template As<Call>());
          break;
        }
        case AST_eLIST: {
          Visit(n.template As<List>());
          break;
        }
        case AST_ePAIR: {
          Visit(n.template As<Assoc>());
          break;
        }
        case AST_eINDEX: {
          Visit(n.template As<Index>());
          break;
        }
        case AST_eSLICE: {
          Visit(n.template As<Slice>());
          break;
        }
        case AST_eFSTRING: {
          Visit(n.template As<FString>());
          break;
        }
        case AST_eIDENT: {
          Visit(n.template As<Identifier>());
          break;
        }
        case AST_eTEMPLATE_CALL: {
          Visit(n.template As<TemplateCall>());
          break;
        }
        case AST_eIMPORT: {
          Visit(n.template As<Import>());
          break;
        }
        case AST_tREF: {
          Visit(n.template As<RefTy>());
          break;
        }
        case AST_tU1: {
          Visit(n.template As<U1>());
          break;
        }
        case AST_tU8: {
          Visit(n.template As<U8>());
          break;
        }
        case AST_tU16: {
          Visit(n.template As<U16>());
          break;
        }
        case AST_tU32: {
          Visit(n.template As<U32>());
          break;
        }
        case AST_tU64: {
          Visit(n.template As<U64>());
          break;
        }
        case AST_tU128: {
          Visit(n.template As<U128>());
          break;
        }
        case AST_tI8: {
          Visit(n.template As<I8>());
          break;
        }
        case AST_tI16: {
          Visit(n.template As<I16>());
          break;
        }
        case AST_tI32: {
          Visit(n.template As<I32>());
          break;
        }
        case AST_tI64: {
          Visit(n.template As<I64>());
          break;
        }
        case AST_tI128: {
          Visit(n.template As<I128>());
          break;
        }
        case AST_tF16: {
          Visit(n.template As<F16>());
          break;
        }
        case AST_tF32: {
          Visit(n.template As<F32>());
          break;
        }
        case AST_tF64: {
          Visit(n.template As<F64>());
          break;
        }
        case AST_tF128: {
          Visit(n.template As<F128>());
          break;
        }
        case AST_tVOID: {
          Visit(n.template As<VoidTy>());
          break;
        }
        case AST_tPTR: {
          Visit(n.template As<PtrTy>());
          break;
        }
        case AST_tOPAQUE: {
          Visit(n.template As<OpaqueTy>());
          break;
        }
        case AST_tARRAY: {
          Visit(n.template As<ArrayTy>());
          break;
        }
        case AST_tTUPLE: {
          Visit(n.template As<TupleTy>());
          break;
        }
        case AST_tFUNCTION: {
          Visit(n.template As<FuncTy>());
          break;
        }
        case AST_tNAMED: {
          Visit(n.template As<NamedTy>());
          break;
        }
        case AST_tINFER: {
          Visit(n.template As<InferTy>());
          break;
        }
        case AST_tTEMPLATE: {
          Visit(n.template As<TemplateType>());
          break;
        }
        case AST_sTYPEDEF: {
          Visit(n.template As<Typedef>());
          break;
        }
        case AST_sSTRUCT: {
          Visit(n.template As<Struct>());
          break;
        }
        case AST_sENUM: {
          Visit(n.template As<Enum>());
          break;
        }
        case AST_sFUNCTION: {
          Visit(n.template As<Function>());
          break;
        }
        case AST_sSCOPE: {
          Visit(n.template As<Scope>());
          break;
        }
        case AST_sEXPORT: {
          Visit(n.template As<Export>());
          break;
        }
        case AST_sBLOCK: {
          Visit(n.template As<Block>());
          break;
        }
        case AST_sVAR: {
          Visit(n.template As<Variable>());
          break;
        }
        case AST_sASM: {
          Visit(n.template As<Assembly>());
          break;
        }
        case AST_sRET: {
          Visit(n.template As<Return>());
          break;
        }
        case AST_sRETIF: {
          Visit(n.template As<ReturnIf>());
          break;
        }
        case AST_sBRK: {
          Visit(n.template As<Break>());
          break;
        }
        case AST_sCONT: {
          Visit(n.template As<Continue>());
          break;
        }
        case AST_sIF: {
          Visit(n.template As<If>());
          break;
        }
        case AST_sWHILE: {
          Visit(n.template As<While>());
          break;
        }
        case AST_sFOR: {
          Visit(n.template As<For>());
          break;
        }
        case AST_sFOREACH: {
          Visit(n.template As<Foreach>());
          break;
        }
        case AST_sCASE: {
          Visit(n.template As<Case>());
          break;
        }
        case AST_sSWITCH: {
          Visit(n.template As<Switch>());
          break;
        }
      }
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_VISTOR_H__
