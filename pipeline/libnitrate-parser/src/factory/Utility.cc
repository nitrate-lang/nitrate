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

#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>

using namespace ncc::parse;

auto ASTFactory::CreateMockInstance(ASTNodeKind kind, SourceLocation origin) -> FlowPtr<Expr> {
  NullableFlowPtr<Expr> r;

#define SIMPLE_EXPR() CreateMockInstance(QAST_NULL, origin)
#define SIMPLE_TYPE() CreateMockInstance(QAST_VOID, origin)->As<Type>()

  switch (kind) {
    case QAST_BINEXPR: {
      r = CreateBinary(SIMPLE_EXPR(), lex::OpPlus, SIMPLE_EXPR());
      break;
    }

    case QAST_UNEXPR: {
      r = CreateUnary(lex::OpPlus, SIMPLE_EXPR());
      break;
    }

    case QAST_POST_UNEXPR: {
      r = CreatePostUnary(SIMPLE_EXPR(), lex::OpPlus);
      break;
    }

    case QAST_TEREXPR: {
      r = CreateTernary(SIMPLE_EXPR(), SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_INT: {
      r = CreateInteger(0);
      break;
    }

    case QAST_FLOAT: {
      r = CreateFloat(0.0);
      break;
    }

    case QAST_STRING: {
      r = CreateString("");
      break;
    }

    case QAST_CHAR: {
      r = CreateCharacter(0);
      break;
    }

    case QAST_BOOL: {
      r = CreateBoolean(false);
      break;
    }

    case QAST_NULL: {
      r = CreateNull();
      break;
    }

    case QAST_UNDEF: {
      r = CreateUndefined();
      break;
    }

    case QAST_CALL: {
      r = CreateCall(SIMPLE_EXPR(), {});
      break;
    }

    case QAST_LIST: {
      r = CreateList();
      break;
    }

    case QAST_ASSOC: {
      r = CreateAssociation(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_INDEX: {
      r = CreateIndex(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_SLICE: {
      r = CreateSlice(SIMPLE_EXPR(), SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_FSTRING: {
      r = CreateFormatString();
      break;
    }

    case QAST_IDENT: {
      r = CreateIdentifier("_");
      break;
    }

    case QAST_SEQ: {
      r = CreateSequence();
      break;
    }

    case QAST_TEMPL_CALL: {
      r = CreateTemplateCall(SIMPLE_EXPR());
      break;
    }

    case QAST_U1: {
      r = CreateU1();
      break;
    }

    case QAST_U8: {
      r = CreateU8();
      break;
    }

    case QAST_U16: {
      r = CreateU16();
      break;
    }

    case QAST_U32: {
      r = CreateU32();
      break;
    }

    case QAST_U64: {
      r = CreateU64();
      break;
    }

    case QAST_U128: {
      r = CreateU128();
      break;
    }

    case QAST_I8: {
      r = CreateI8();
      break;
    }

    case QAST_I16: {
      r = CreateI16();
      break;
    }

    case QAST_I32: {
      r = CreateI32();
      break;
    }

    case QAST_I64: {
      r = CreateI64();
      break;
    }

    case QAST_I128: {
      r = CreateI128();
      break;
    }

    case QAST_F16: {
      r = CreateF16();
      break;
    }

    case QAST_F32: {
      r = CreateF32();
      break;
    }

    case QAST_F64: {
      r = CreateF64();
      break;
    }

    case QAST_F128: {
      r = CreateF128();
      break;
    }

    case QAST_VOID: {
      r = CreateVoid();
      break;
    }

    case QAST_INFER: {
      r = CreateUnknownType();
      break;
    }

    case QAST_OPAQUE: {
      r = CreateOpaque("_");
      break;
    }

    case QAST_NAMED: {
      r = CreateNamed("_");
      break;
    }

    case QAST_REF: {
      r = CreateReference(SIMPLE_TYPE());
      break;
    }

    case QAST_PTR: {
      r = CreatePointer(SIMPLE_TYPE());
      break;
    }

    case QAST_ARRAY: {
      r = CreateArray(SIMPLE_TYPE(), SIMPLE_EXPR());
      break;
    }

    case QAST_TUPLE: {
      r = CreateTuple();
      break;
    }

    case QAST_TEMPLATE: {
      r = CreateTemplateType(SIMPLE_TYPE());
      break;
    }

    case QAST_FUNCTOR: {
      r = CreateFunc(SIMPLE_TYPE());
      break;
    }

    case QAST_IF: {
      r = CreateIf(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_RETIF: {
      r = CreateReturnIf(SIMPLE_EXPR());
      break;
    }

    case QAST_SWITCH: {
      r = CreateSwitch(SIMPLE_EXPR());
      break;
    }

    case QAST_CASE: {
      r = CreateCase(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_RETURN: {
      r = CreateReturn();
      break;
    }

    case QAST_BREAK: {
      r = CreateBreak();
      break;
    }

    case QAST_CONTINUE: {
      r = CreateContinue();
      break;
    }

    case QAST_WHILE: {
      r = CreateWhile(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_FOR: {
      r = CreateFor(nullptr, nullptr, nullptr, SIMPLE_EXPR());
      break;
    }

    case QAST_FOREACH: {
      r = CreateForeach("_", "_", SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case QAST_INLINE_ASM: {
      r = CreateAssembly("");
      break;
    }

    case QAST_TYPEDEF: {
      r = CreateTypedef("_", SIMPLE_TYPE());
      break;
    }

    case QAST_STRUCT: {
      r = CreateStruct();
      break;
    }

    case QAST_ENUM: {
      r = CreateEnum();
      break;
    }

    case QAST_SCOPE: {
      r = CreateScope();
      break;
    }

    case QAST_BLOCK: {
      r = CreateBlock();
      break;
    }

    case QAST_EXPORT: {
      r = CreateExport();
      break;
    }

    case QAST_VAR: {
      r = CreateVariable(VariableType::Var, "_");
      break;
    }

    case QAST_FUNCTION: {
      r = CreateFunction();
      break;
    }
  }

#undef SIMPLE_TYPE
#undef SIMPLE_EXPR

  r.value()->SetMock(true);
  r.SetTracking(origin);

  return r.value();
}
