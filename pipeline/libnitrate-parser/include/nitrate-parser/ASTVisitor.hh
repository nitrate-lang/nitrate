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
#include <nitrate-parser/ASTCommon.hh>

namespace ncc::parse {
  class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;

    virtual void Visit(FlowPtr<Base> n) = 0;
    virtual void Visit(FlowPtr<ExprStmt> n) = 0;
    virtual void Visit(FlowPtr<LambdaExpr> n) = 0;
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
    virtual void Visit(FlowPtr<PostUnary> n) = 0;
    virtual void Visit(FlowPtr<Ternary> n) = 0;
    virtual void Visit(FlowPtr<Integer> n) = 0;
    virtual void Visit(FlowPtr<Float> n) = 0;
    virtual void Visit(FlowPtr<Boolean> n) = 0;
    virtual void Visit(FlowPtr<String> n) = 0;
    virtual void Visit(FlowPtr<Character> n) = 0;
    virtual void Visit(FlowPtr<Null> n) = 0;
    virtual void Visit(FlowPtr<Undefined> n) = 0;
    virtual void Visit(FlowPtr<Call> n) = 0;
    virtual void Visit(FlowPtr<TemplateCall> n) = 0;
    virtual void Visit(FlowPtr<List> n) = 0;
    virtual void Visit(FlowPtr<Assoc> n) = 0;
    virtual void Visit(FlowPtr<Index> n) = 0;
    virtual void Visit(FlowPtr<Slice> n) = 0;
    virtual void Visit(FlowPtr<FString> n) = 0;
    virtual void Visit(FlowPtr<Identifier> n) = 0;
    virtual void Visit(FlowPtr<Sequence> n) = 0;
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
        case QAST_BASE: {
          Visit(n.template As<Base>());
          break;
        }
        case QAST_BINEXPR: {
          Visit(n.template As<Binary>());
          break;
        }
        case QAST_UNEXPR: {
          Visit(n.template As<Unary>());
          break;
        }
        case QAST_TEREXPR: {
          Visit(n.template As<Ternary>());
          break;
        }
        case QAST_INT: {
          Visit(n.template As<Integer>());
          break;
        }
        case QAST_FLOAT: {
          Visit(n.template As<Float>());
          break;
        }
        case QAST_STRING: {
          Visit(n.template As<String>());
          break;
        }
        case QAST_CHAR: {
          Visit(n.template As<Character>());
          break;
        }
        case QAST_BOOL: {
          Visit(n.template As<Boolean>());
          break;
        }
        case QAST_NULL: {
          Visit(n.template As<Null>());
          break;
        }
        case QAST_UNDEF: {
          Visit(n.template As<Undefined>());
          break;
        }
        case QAST_CALL: {
          Visit(n.template As<Call>());
          break;
        }
        case QAST_LIST: {
          Visit(n.template As<List>());
          break;
        }
        case QAST_ASSOC: {
          Visit(n.template As<Assoc>());
          break;
        }
        case QAST_INDEX: {
          Visit(n.template As<Index>());
          break;
        }
        case QAST_SLICE: {
          Visit(n.template As<Slice>());
          break;
        }
        case QAST_FSTRING: {
          Visit(n.template As<FString>());
          break;
        }
        case QAST_IDENT: {
          Visit(n.template As<Identifier>());
          break;
        }
        case QAST_SEQ: {
          Visit(n.template As<Sequence>());
          break;
        }
        case QAST_POST_UNEXPR: {
          Visit(n.template As<PostUnary>());
          break;
        }
        case QAST_LAMBDA: {
          Visit(n.template As<LambdaExpr>());
          break;
        }
        case QAST_TEMPL_CALL: {
          Visit(n.template As<TemplateCall>());
          break;
        }
        case QAST_REF: {
          Visit(n.template As<RefTy>());
          break;
        }
        case QAST_U1: {
          Visit(n.template As<U1>());
          break;
        }
        case QAST_U8: {
          Visit(n.template As<U8>());
          break;
        }
        case QAST_U16: {
          Visit(n.template As<U16>());
          break;
        }
        case QAST_U32: {
          Visit(n.template As<U32>());
          break;
        }
        case QAST_U64: {
          Visit(n.template As<U64>());
          break;
        }
        case QAST_U128: {
          Visit(n.template As<U128>());
          break;
        }
        case QAST_I8: {
          Visit(n.template As<I8>());
          break;
        }
        case QAST_I16: {
          Visit(n.template As<I16>());
          break;
        }
        case QAST_I32: {
          Visit(n.template As<I32>());
          break;
        }
        case QAST_I64: {
          Visit(n.template As<I64>());
          break;
        }
        case QAST_I128: {
          Visit(n.template As<I128>());
          break;
        }
        case QAST_F16: {
          Visit(n.template As<F16>());
          break;
        }
        case QAST_F32: {
          Visit(n.template As<F32>());
          break;
        }
        case QAST_F64: {
          Visit(n.template As<F64>());
          break;
        }
        case QAST_F128: {
          Visit(n.template As<F128>());
          break;
        }
        case QAST_VOID: {
          Visit(n.template As<VoidTy>());
          break;
        }
        case QAST_PTR: {
          Visit(n.template As<PtrTy>());
          break;
        }
        case QAST_OPAQUE: {
          Visit(n.template As<OpaqueTy>());
          break;
        }
        case QAST_ARRAY: {
          Visit(n.template As<ArrayTy>());
          break;
        }
        case QAST_TUPLE: {
          Visit(n.template As<TupleTy>());
          break;
        }
        case QAST_FUNCTOR: {
          Visit(n.template As<FuncTy>());
          break;
        }
        case QAST_NAMED: {
          Visit(n.template As<NamedTy>());
          break;
        }
        case QAST_INFER: {
          Visit(n.template As<InferTy>());
          break;
        }
        case QAST_TEMPLATE: {
          Visit(n.template As<TemplateType>());
          break;
        }
        case QAST_TYPEDEF: {
          Visit(n.template As<Typedef>());
          break;
        }
        case QAST_STRUCT: {
          Visit(n.template As<Struct>());
          break;
        }
        case QAST_ENUM: {
          Visit(n.template As<Enum>());
          break;
        }
        case QAST_FUNCTION: {
          Visit(n.template As<Function>());
          break;
        }
        case QAST_SCOPE: {
          Visit(n.template As<Scope>());
          break;
        }
        case QAST_EXPORT: {
          Visit(n.template As<Export>());
          break;
        }
        case QAST_BLOCK: {
          Visit(n.template As<Block>());
          break;
        }
        case QAST_VAR: {
          Visit(n.template As<Variable>());
          break;
        }
        case QAST_INLINE_ASM: {
          Visit(n.template As<Assembly>());
          break;
        }
        case QAST_RETURN: {
          Visit(n.template As<Return>());
          break;
        }
        case QAST_RETIF: {
          Visit(n.template As<ReturnIf>());
          break;
        }
        case QAST_BREAK: {
          Visit(n.template As<Break>());
          break;
        }
        case QAST_CONTINUE: {
          Visit(n.template As<Continue>());
          break;
        }
        case QAST_IF: {
          Visit(n.template As<If>());
          break;
        }
        case QAST_WHILE: {
          Visit(n.template As<While>());
          break;
        }
        case QAST_FOR: {
          Visit(n.template As<For>());
          break;
        }
        case QAST_FOREACH: {
          Visit(n.template As<Foreach>());
          break;
        }
        case QAST_CASE: {
          Visit(n.template As<Case>());
          break;
        }
        case QAST_SWITCH: {
          Visit(n.template As<Switch>());
          break;
        }
        case QAST_ESTMT: {
          Visit(n.template As<ExprStmt>());
          break;
        }
      }
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_VISTOR_H__
