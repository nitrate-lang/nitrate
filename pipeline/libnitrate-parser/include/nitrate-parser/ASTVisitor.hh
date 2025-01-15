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
    virtual void Visit(FlowPtr<StmtExpr> n) = 0;
    virtual void Visit(FlowPtr<TypeExpr> n) = 0;
    virtual void Visit(FlowPtr<NamedTy> n) = 0;
    virtual void Visit(FlowPtr<InferTy> n) = 0;
    virtual void Visit(FlowPtr<TemplType> n) = 0;
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
    virtual void Visit(FlowPtr<UnaryExpr> n) = 0;
    virtual void Visit(FlowPtr<BinExpr> n) = 0;
    virtual void Visit(FlowPtr<PostUnaryExpr> n) = 0;
    virtual void Visit(FlowPtr<TernaryExpr> n) = 0;
    virtual void Visit(FlowPtr<ConstInt> n) = 0;
    virtual void Visit(FlowPtr<ConstFloat> n) = 0;
    virtual void Visit(FlowPtr<ConstBool> n) = 0;
    virtual void Visit(FlowPtr<ConstString> n) = 0;
    virtual void Visit(FlowPtr<ConstChar> n) = 0;
    virtual void Visit(FlowPtr<ConstNull> n) = 0;
    virtual void Visit(FlowPtr<ConstUndef> n) = 0;
    virtual void Visit(FlowPtr<Call> n) = 0;
    virtual void Visit(FlowPtr<TemplCall> n) = 0;
    virtual void Visit(FlowPtr<List> n) = 0;
    virtual void Visit(FlowPtr<Assoc> n) = 0;
    virtual void Visit(FlowPtr<Index> n) = 0;
    virtual void Visit(FlowPtr<Slice> n) = 0;
    virtual void Visit(FlowPtr<FString> n) = 0;
    virtual void Visit(FlowPtr<Ident> n) = 0;
    virtual void Visit(FlowPtr<SeqPoint> n) = 0;
    virtual void Visit(FlowPtr<Block> n) = 0;
    virtual void Visit(FlowPtr<VarDecl> n) = 0;
    virtual void Visit(FlowPtr<InlineAsm> n) = 0;
    virtual void Visit(FlowPtr<IfStmt> n) = 0;
    virtual void Visit(FlowPtr<WhileStmt> n) = 0;
    virtual void Visit(FlowPtr<ForStmt> n) = 0;
    virtual void Visit(FlowPtr<ForeachStmt> n) = 0;
    virtual void Visit(FlowPtr<BreakStmt> n) = 0;
    virtual void Visit(FlowPtr<ContinueStmt> n) = 0;
    virtual void Visit(FlowPtr<ReturnStmt> n) = 0;
    virtual void Visit(FlowPtr<ReturnIfStmt> n) = 0;
    virtual void Visit(FlowPtr<CaseStmt> n) = 0;
    virtual void Visit(FlowPtr<SwitchStmt> n) = 0;
    virtual void Visit(FlowPtr<TypedefStmt> n) = 0;
    virtual void Visit(FlowPtr<Function> n) = 0;
    virtual void Visit(FlowPtr<StructDef> n) = 0;
    virtual void Visit(FlowPtr<EnumDef> n) = 0;
    virtual void Visit(FlowPtr<ScopeStmt> n) = 0;
    virtual void Visit(FlowPtr<ExportStmt> n) = 0;

    template <typename T>
    void Dispatch(FlowPtr<T> n) {
      switch (n->GetKind()) {
        case QAST_BASE: {
          Visit(n.template as<Base>());
          break;
        }
        case QAST_BINEXPR: {
          Visit(n.template as<BinExpr>());
          break;
        }
        case QAST_UNEXPR: {
          Visit(n.template as<UnaryExpr>());
          break;
        }
        case QAST_TEREXPR: {
          Visit(n.template as<TernaryExpr>());
          break;
        }
        case QAST_INT: {
          Visit(n.template as<ConstInt>());
          break;
        }
        case QAST_FLOAT: {
          Visit(n.template as<ConstFloat>());
          break;
        }
        case QAST_STRING: {
          Visit(n.template as<ConstString>());
          break;
        }
        case QAST_CHAR: {
          Visit(n.template as<ConstChar>());
          break;
        }
        case QAST_BOOL: {
          Visit(n.template as<ConstBool>());
          break;
        }
        case QAST_NULL: {
          Visit(n.template as<ConstNull>());
          break;
        }
        case QAST_UNDEF: {
          Visit(n.template as<ConstUndef>());
          break;
        }
        case QAST_CALL: {
          Visit(n.template as<Call>());
          break;
        }
        case QAST_LIST: {
          Visit(n.template as<List>());
          break;
        }
        case QAST_ASSOC: {
          Visit(n.template as<Assoc>());
          break;
        }
        case QAST_INDEX: {
          Visit(n.template as<Index>());
          break;
        }
        case QAST_SLICE: {
          Visit(n.template as<Slice>());
          break;
        }
        case QAST_FSTRING: {
          Visit(n.template as<FString>());
          break;
        }
        case QAST_IDENT: {
          Visit(n.template as<Ident>());
          break;
        }
        case QAST_SEQ: {
          Visit(n.template as<SeqPoint>());
          break;
        }
        case QAST_POST_UNEXPR: {
          Visit(n.template as<PostUnaryExpr>());
          break;
        }
        case QAST_SEXPR: {
          Visit(n.template as<StmtExpr>());
          break;
        }
        case QAST_TEXPR: {
          Visit(n.template as<TypeExpr>());
          break;
        }
        case QAST_TEMPL_CALL: {
          Visit(n.template as<TemplCall>());
          break;
        }
        case QAST_REF: {
          Visit(n.template as<RefTy>());
          break;
        }
        case QAST_U1: {
          Visit(n.template as<U1>());
          break;
        }
        case QAST_U8: {
          Visit(n.template as<U8>());
          break;
        }
        case QAST_U16: {
          Visit(n.template as<U16>());
          break;
        }
        case QAST_U32: {
          Visit(n.template as<U32>());
          break;
        }
        case QAST_U64: {
          Visit(n.template as<U64>());
          break;
        }
        case QAST_U128: {
          Visit(n.template as<U128>());
          break;
        }
        case QAST_I8: {
          Visit(n.template as<I8>());
          break;
        }
        case QAST_I16: {
          Visit(n.template as<I16>());
          break;
        }
        case QAST_I32: {
          Visit(n.template as<I32>());
          break;
        }
        case QAST_I64: {
          Visit(n.template as<I64>());
          break;
        }
        case QAST_I128: {
          Visit(n.template as<I128>());
          break;
        }
        case QAST_F16: {
          Visit(n.template as<F16>());
          break;
        }
        case QAST_F32: {
          Visit(n.template as<F32>());
          break;
        }
        case QAST_F64: {
          Visit(n.template as<F64>());
          break;
        }
        case QAST_F128: {
          Visit(n.template as<F128>());
          break;
        }
        case QAST_VOID: {
          Visit(n.template as<VoidTy>());
          break;
        }
        case QAST_PTR: {
          Visit(n.template as<PtrTy>());
          break;
        }
        case QAST_OPAQUE: {
          Visit(n.template as<OpaqueTy>());
          break;
        }
        case QAST_ARRAY: {
          Visit(n.template as<ArrayTy>());
          break;
        }
        case QAST_TUPLE: {
          Visit(n.template as<TupleTy>());
          break;
        }
        case QAST_FUNCTOR: {
          Visit(n.template as<FuncTy>());
          break;
        }
        case QAST_NAMED: {
          Visit(n.template as<NamedTy>());
          break;
        }
        case QAST_INFER: {
          Visit(n.template as<InferTy>());
          break;
        }
        case QAST_TEMPLATE: {
          Visit(n.template as<TemplType>());
          break;
        }
        case QAST_TYPEDEF: {
          Visit(n.template as<TypedefStmt>());
          break;
        }
        case QAST_STRUCT: {
          Visit(n.template as<StructDef>());
          break;
        }
        case QAST_ENUM: {
          Visit(n.template as<EnumDef>());
          break;
        }
        case QAST_FUNCTION: {
          Visit(n.template as<Function>());
          break;
        }
        case QAST_SCOPE: {
          Visit(n.template as<ScopeStmt>());
          break;
        }
        case QAST_EXPORT: {
          Visit(n.template as<ExportStmt>());
          break;
        }
        case QAST_BLOCK: {
          Visit(n.template as<Block>());
          break;
        }
        case QAST_VAR: {
          Visit(n.template as<VarDecl>());
          break;
        }
        case QAST_INLINE_ASM: {
          Visit(n.template as<InlineAsm>());
          break;
        }
        case QAST_RETURN: {
          Visit(n.template as<ReturnStmt>());
          break;
        }
        case QAST_RETIF: {
          Visit(n.template as<ReturnIfStmt>());
          break;
        }
        case QAST_BREAK: {
          Visit(n.template as<BreakStmt>());
          break;
        }
        case QAST_CONTINUE: {
          Visit(n.template as<ContinueStmt>());
          break;
        }
        case QAST_IF: {
          Visit(n.template as<IfStmt>());
          break;
        }
        case QAST_WHILE: {
          Visit(n.template as<WhileStmt>());
          break;
        }
        case QAST_FOR: {
          Visit(n.template as<ForStmt>());
          break;
        }
        case QAST_FOREACH: {
          Visit(n.template as<ForeachStmt>());
          break;
        }
        case QAST_CASE: {
          Visit(n.template as<CaseStmt>());
          break;
        }
        case QAST_SWITCH: {
          Visit(n.template as<SwitchStmt>());
          break;
        }
        case QAST_ESTMT: {
          Visit(n.template as<ExprStmt>());
          break;
        }
      }
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_VISTOR_H__
