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

#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>

using namespace ncc::parse;

auto ASTFactory::CreateMockInstance(ASTNodeKind kind, SourceLocation origin) -> FlowPtr<Expr> {
  NullableFlowPtr<Expr> r;

#define SIMPLE_EXPR() CreateMockInstance(AST_eNULL, origin)
#define SIMPLE_TYPE() CreateMockInstance<Type>(AST_VOID, origin)

  switch (kind) {
    case AST_DISCARDED: {
      // Create some node to avoid a panic
      r = CreateUndefined();
      r.value()->Discard();
      break;
    }

    case AST_eBIN: {
      r = CreateBinary(SIMPLE_EXPR(), lex::OpPlus, SIMPLE_EXPR());
      break;
    }

    case AST_eUNARY: {
      r = CreateUnary(lex::OpPlus, SIMPLE_EXPR());
      break;
    }

    case AST_eINT: {
      r = CreateInteger(0);
      break;
    }

    case AST_eFLOAT: {
      r = CreateFloat(0.0);
      break;
    }

    case AST_eSTRING: {
      r = CreateString("");
      break;
    }

    case AST_eCHAR: {
      r = CreateCharacter(0);
      break;
    }

    case AST_eBOOL: {
      r = CreateBoolean(false);
      break;
    }

    case AST_eNULL: {
      r = CreateNull();
      break;
    }

    case AST_eUNDEF: {
      r = CreateUndefined();
      break;
    }

    case AST_eCALL: {
      r = CreateCall(SIMPLE_EXPR(), {});
      break;
    }

    case AST_LIST: {
      r = CreateList();
      break;
    }

    case AST_ASSOC: {
      r = CreateAssociation(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_INDEX: {
      r = CreateIndex(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_SLICE: {
      r = CreateSlice(SIMPLE_EXPR(), SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_FSTRING: {
      r = CreateFormatString();
      break;
    }

    case AST_IDENT: {
      r = CreateIdentifier("_");
      break;
    }

    case AST_TEMPL_CALL: {
      r = CreateTemplateCall(SIMPLE_EXPR());
      break;
    }

    case AST_IMPORT: {
      r = CreateImport("", ImportMode::Code, SIMPLE_EXPR());
      break;
    }

    case AST_tU1: {
      r = CreateU1();
      break;
    }

    case AST_tU8: {
      r = CreateU8();
      break;
    }

    case AST_tU16: {
      r = CreateU16();
      break;
    }

    case AST_tU32: {
      r = CreateU32();
      break;
    }

    case AST_tU64: {
      r = CreateU64();
      break;
    }

    case AST_tU128: {
      r = CreateU128();
      break;
    }

    case AST_tI8: {
      r = CreateI8();
      break;
    }

    case AST_tI16: {
      r = CreateI16();
      break;
    }

    case AST_tI32: {
      r = CreateI32();
      break;
    }

    case AST_tI64: {
      r = CreateI64();
      break;
    }

    case AST_tI128: {
      r = CreateI128();
      break;
    }

    case AST_tF16: {
      r = CreateF16();
      break;
    }

    case AST_tF32: {
      r = CreateF32();
      break;
    }

    case AST_tF64: {
      r = CreateF64();
      break;
    }

    case AST_tF128: {
      r = CreateF128();
      break;
    }

    case AST_VOID: {
      r = CreateVoid();
      break;
    }

    case AST_INFER: {
      r = CreateUnknownType();
      break;
    }

    case AST_OPAQUE: {
      r = CreateOpaque("_");
      break;
    }

    case AST_NAMED: {
      r = CreateNamed("_");
      break;
    }

    case AST_REF: {
      r = CreateReference(SIMPLE_TYPE());
      break;
    }

    case AST_PTR: {
      r = CreatePointer(SIMPLE_TYPE());
      break;
    }

    case AST_ARRAY: {
      r = CreateArray(SIMPLE_TYPE(), SIMPLE_EXPR());
      break;
    }

    case AST_TUPLE: {
      r = CreateTuple();
      break;
    }

    case AST_TEMPLATE: {
      r = CreateTemplateType(SIMPLE_TYPE());
      break;
    }

    case AST_FUNCTOR: {
      r = CreateFunctionType(SIMPLE_TYPE());
      break;
    }

    case AST_IF: {
      r = CreateIf(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_RETIF: {
      r = CreateReturnIf(SIMPLE_EXPR());
      break;
    }

    case AST_SWITCH: {
      r = CreateSwitch(SIMPLE_EXPR());
      break;
    }

    case AST_CASE: {
      r = CreateCase(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_RETURN: {
      r = CreateReturn();
      break;
    }

    case AST_BREAK: {
      r = CreateBreak();
      break;
    }

    case AST_CONTINUE: {
      r = CreateContinue();
      break;
    }

    case AST_WHILE: {
      r = CreateWhile(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_FOR: {
      r = CreateFor(nullptr, nullptr, nullptr, SIMPLE_EXPR());
      break;
    }

    case AST_FOREACH: {
      r = CreateForeach("_", "_", SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_INLINE_ASM: {
      r = CreateAssembly("");
      break;
    }

    case AST_TYPEDEF: {
      r = CreateTypedef("_", SIMPLE_TYPE());
      break;
    }

    case AST_STRUCT: {
      r = CreateStruct();
      break;
    }

    case AST_ENUM: {
      r = CreateEnum("");
      break;
    }

    case AST_SCOPE: {
      r = CreateScope("_", SIMPLE_EXPR());
      break;
    }

    case AST_BLOCK: {
      r = CreateBlock();
      break;
    }

    case AST_EXPORT: {
      r = CreateExport(CreateMockInstance<Block>());
      break;
    }

    case AST_VAR: {
      r = CreateVariable(VariableType::Var, "_");
      break;
    }

    case AST_FUNCTION: {
      r = CreateFunction("");
      break;
    }
  }

#undef SIMPLE_TYPE
#undef SIMPLE_EXPR

  r.value()->SetMock(true);
  r.SetTracking(origin);

  return r.value();
}
