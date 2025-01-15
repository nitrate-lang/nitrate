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

    virtual void visit(FlowPtr<Base> n) = 0;
    virtual void visit(FlowPtr<ExprStmt> n) = 0;
    virtual void visit(FlowPtr<StmtExpr> n) = 0;
    virtual void visit(FlowPtr<TypeExpr> n) = 0;
    virtual void visit(FlowPtr<NamedTy> n) = 0;
    virtual void visit(FlowPtr<InferTy> n) = 0;
    virtual void visit(FlowPtr<TemplType> n) = 0;
    virtual void visit(FlowPtr<U1> n) = 0;
    virtual void visit(FlowPtr<U8> n) = 0;
    virtual void visit(FlowPtr<U16> n) = 0;
    virtual void visit(FlowPtr<U32> n) = 0;
    virtual void visit(FlowPtr<U64> n) = 0;
    virtual void visit(FlowPtr<U128> n) = 0;
    virtual void visit(FlowPtr<I8> n) = 0;
    virtual void visit(FlowPtr<I16> n) = 0;
    virtual void visit(FlowPtr<I32> n) = 0;
    virtual void visit(FlowPtr<I64> n) = 0;
    virtual void visit(FlowPtr<I128> n) = 0;
    virtual void visit(FlowPtr<F16> n) = 0;
    virtual void visit(FlowPtr<F32> n) = 0;
    virtual void visit(FlowPtr<F64> n) = 0;
    virtual void visit(FlowPtr<F128> n) = 0;
    virtual void visit(FlowPtr<VoidTy> n) = 0;
    virtual void visit(FlowPtr<PtrTy> n) = 0;
    virtual void visit(FlowPtr<OpaqueTy> n) = 0;
    virtual void visit(FlowPtr<TupleTy> n) = 0;
    virtual void visit(FlowPtr<ArrayTy> n) = 0;
    virtual void visit(FlowPtr<RefTy> n) = 0;
    virtual void visit(FlowPtr<FuncTy> n) = 0;
    virtual void visit(FlowPtr<UnaryExpr> n) = 0;
    virtual void visit(FlowPtr<BinExpr> n) = 0;
    virtual void visit(FlowPtr<PostUnaryExpr> n) = 0;
    virtual void visit(FlowPtr<TernaryExpr> n) = 0;
    virtual void visit(FlowPtr<ConstInt> n) = 0;
    virtual void visit(FlowPtr<ConstFloat> n) = 0;
    virtual void visit(FlowPtr<ConstBool> n) = 0;
    virtual void visit(FlowPtr<ConstString> n) = 0;
    virtual void visit(FlowPtr<ConstChar> n) = 0;
    virtual void visit(FlowPtr<ConstNull> n) = 0;
    virtual void visit(FlowPtr<ConstUndef> n) = 0;
    virtual void visit(FlowPtr<Call> n) = 0;
    virtual void visit(FlowPtr<TemplCall> n) = 0;
    virtual void visit(FlowPtr<List> n) = 0;
    virtual void visit(FlowPtr<Assoc> n) = 0;
    virtual void visit(FlowPtr<Index> n) = 0;
    virtual void visit(FlowPtr<Slice> n) = 0;
    virtual void visit(FlowPtr<FString> n) = 0;
    virtual void visit(FlowPtr<Ident> n) = 0;
    virtual void visit(FlowPtr<SeqPoint> n) = 0;
    virtual void visit(FlowPtr<Block> n) = 0;
    virtual void visit(FlowPtr<VarDecl> n) = 0;
    virtual void visit(FlowPtr<InlineAsm> n) = 0;
    virtual void visit(FlowPtr<IfStmt> n) = 0;
    virtual void visit(FlowPtr<WhileStmt> n) = 0;
    virtual void visit(FlowPtr<ForStmt> n) = 0;
    virtual void visit(FlowPtr<ForeachStmt> n) = 0;
    virtual void visit(FlowPtr<BreakStmt> n) = 0;
    virtual void visit(FlowPtr<ContinueStmt> n) = 0;
    virtual void visit(FlowPtr<ReturnStmt> n) = 0;
    virtual void visit(FlowPtr<ReturnIfStmt> n) = 0;
    virtual void visit(FlowPtr<CaseStmt> n) = 0;
    virtual void visit(FlowPtr<SwitchStmt> n) = 0;
    virtual void visit(FlowPtr<TypedefStmt> n) = 0;
    virtual void visit(FlowPtr<Function> n) = 0;
    virtual void visit(FlowPtr<StructDef> n) = 0;
    virtual void visit(FlowPtr<EnumDef> n) = 0;
    virtual void visit(FlowPtr<ScopeStmt> n) = 0;
    virtual void visit(FlowPtr<ExportStmt> n) = 0;

    template <typename T>
    void dispatch(FlowPtr<T> n) {
      switch (n->GetKind()) {
        case QAST_BASE: {
          visit(n.template as<Base>());
          break;
        }
        case QAST_BINEXPR: {
          visit(n.template as<BinExpr>());
          break;
        }
        case QAST_UNEXPR: {
          visit(n.template as<UnaryExpr>());
          break;
        }
        case QAST_TEREXPR: {
          visit(n.template as<TernaryExpr>());
          break;
        }
        case QAST_INT: {
          visit(n.template as<ConstInt>());
          break;
        }
        case QAST_FLOAT: {
          visit(n.template as<ConstFloat>());
          break;
        }
        case QAST_STRING: {
          visit(n.template as<ConstString>());
          break;
        }
        case QAST_CHAR: {
          visit(n.template as<ConstChar>());
          break;
        }
        case QAST_BOOL: {
          visit(n.template as<ConstBool>());
          break;
        }
        case QAST_NULL: {
          visit(n.template as<ConstNull>());
          break;
        }
        case QAST_UNDEF: {
          visit(n.template as<ConstUndef>());
          break;
        }
        case QAST_CALL: {
          visit(n.template as<Call>());
          break;
        }
        case QAST_LIST: {
          visit(n.template as<List>());
          break;
        }
        case QAST_ASSOC: {
          visit(n.template as<Assoc>());
          break;
        }
        case QAST_INDEX: {
          visit(n.template as<Index>());
          break;
        }
        case QAST_SLICE: {
          visit(n.template as<Slice>());
          break;
        }
        case QAST_FSTRING: {
          visit(n.template as<FString>());
          break;
        }
        case QAST_IDENT: {
          visit(n.template as<Ident>());
          break;
        }
        case QAST_SEQ: {
          visit(n.template as<SeqPoint>());
          break;
        }
        case QAST_POST_UNEXPR: {
          visit(n.template as<PostUnaryExpr>());
          break;
        }
        case QAST_SEXPR: {
          visit(n.template as<StmtExpr>());
          break;
        }
        case QAST_TEXPR: {
          visit(n.template as<TypeExpr>());
          break;
        }
        case QAST_TEMPL_CALL: {
          visit(n.template as<TemplCall>());
          break;
        }
        case QAST_REF: {
          visit(n.template as<RefTy>());
          break;
        }
        case QAST_U1: {
          visit(n.template as<U1>());
          break;
        }
        case QAST_U8: {
          visit(n.template as<U8>());
          break;
        }
        case QAST_U16: {
          visit(n.template as<U16>());
          break;
        }
        case QAST_U32: {
          visit(n.template as<U32>());
          break;
        }
        case QAST_U64: {
          visit(n.template as<U64>());
          break;
        }
        case QAST_U128: {
          visit(n.template as<U128>());
          break;
        }
        case QAST_I8: {
          visit(n.template as<I8>());
          break;
        }
        case QAST_I16: {
          visit(n.template as<I16>());
          break;
        }
        case QAST_I32: {
          visit(n.template as<I32>());
          break;
        }
        case QAST_I64: {
          visit(n.template as<I64>());
          break;
        }
        case QAST_I128: {
          visit(n.template as<I128>());
          break;
        }
        case QAST_F16: {
          visit(n.template as<F16>());
          break;
        }
        case QAST_F32: {
          visit(n.template as<F32>());
          break;
        }
        case QAST_F64: {
          visit(n.template as<F64>());
          break;
        }
        case QAST_F128: {
          visit(n.template as<F128>());
          break;
        }
        case QAST_VOID: {
          visit(n.template as<VoidTy>());
          break;
        }
        case QAST_PTR: {
          visit(n.template as<PtrTy>());
          break;
        }
        case QAST_OPAQUE: {
          visit(n.template as<OpaqueTy>());
          break;
        }
        case QAST_ARRAY: {
          visit(n.template as<ArrayTy>());
          break;
        }
        case QAST_TUPLE: {
          visit(n.template as<TupleTy>());
          break;
        }
        case QAST_FUNCTOR: {
          visit(n.template as<FuncTy>());
          break;
        }
        case QAST_NAMED: {
          visit(n.template as<NamedTy>());
          break;
        }
        case QAST_INFER: {
          visit(n.template as<InferTy>());
          break;
        }
        case QAST_TEMPLATE: {
          visit(n.template as<TemplType>());
          break;
        }
        case QAST_TYPEDEF: {
          visit(n.template as<TypedefStmt>());
          break;
        }
        case QAST_STRUCT: {
          visit(n.template as<StructDef>());
          break;
        }
        case QAST_ENUM: {
          visit(n.template as<EnumDef>());
          break;
        }
        case QAST_FUNCTION: {
          visit(n.template as<Function>());
          break;
        }
        case QAST_SCOPE: {
          visit(n.template as<ScopeStmt>());
          break;
        }
        case QAST_EXPORT: {
          visit(n.template as<ExportStmt>());
          break;
        }
        case QAST_BLOCK: {
          visit(n.template as<Block>());
          break;
        }
        case QAST_VAR: {
          visit(n.template as<VarDecl>());
          break;
        }
        case QAST_INLINE_ASM: {
          visit(n.template as<InlineAsm>());
          break;
        }
        case QAST_RETURN: {
          visit(n.template as<ReturnStmt>());
          break;
        }
        case QAST_RETIF: {
          visit(n.template as<ReturnIfStmt>());
          break;
        }
        case QAST_BREAK: {
          visit(n.template as<BreakStmt>());
          break;
        }
        case QAST_CONTINUE: {
          visit(n.template as<ContinueStmt>());
          break;
        }
        case QAST_IF: {
          visit(n.template as<IfStmt>());
          break;
        }
        case QAST_WHILE: {
          visit(n.template as<WhileStmt>());
          break;
        }
        case QAST_FOR: {
          visit(n.template as<ForStmt>());
          break;
        }
        case QAST_FOREACH: {
          visit(n.template as<ForeachStmt>());
          break;
        }
        case QAST_CASE: {
          visit(n.template as<CaseStmt>());
          break;
        }
        case QAST_SWITCH: {
          visit(n.template as<SwitchStmt>());
          break;
        }
        case QAST_ESTMT: {
          visit(n.template as<ExprStmt>());
          break;
        }
      }
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_VISTOR_H__
