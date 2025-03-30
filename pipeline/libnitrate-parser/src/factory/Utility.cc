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
#define SIMPLE_TYPE() CreateMockInstance<Type>(AST_tVOID, origin)

  switch (kind) {
    case AST_DISCARDED: {
      // Create some node to avoid a panic
      r = CreateNull();
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

    case AST_eCALL: {
      r = CreateCall(SIMPLE_EXPR(), {});
      break;
    }

    case AST_eLIST: {
      r = CreateList();
      break;
    }

    case AST_ePAIR: {
      r = CreateAssociation(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_eINDEX: {
      r = CreateIndex(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_eSLICE: {
      r = CreateSlice(SIMPLE_EXPR(), SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_eFSTRING: {
      r = CreateFormatString();
      break;
    }

    case AST_eIDENT: {
      r = CreateIdentifier("_");
      break;
    }

    case AST_eTEMPLATE_CALL: {
      r = CreateTemplateCall(SIMPLE_EXPR());
      break;
    }

    case AST_eIMPORT: {
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

    case AST_tVOID: {
      r = CreateVoid();
      break;
    }

    case AST_tINFER: {
      r = CreateUnknownType();
      break;
    }

    case AST_tOPAQUE: {
      r = CreateOpaque("_");
      break;
    }

    case AST_tNAMED: {
      r = CreateNamed("_");
      break;
    }

    case AST_tREF: {
      r = CreateReference(SIMPLE_TYPE());
      break;
    }

    case AST_tPTR: {
      r = CreatePointer(SIMPLE_TYPE());
      break;
    }

    case AST_tARRAY: {
      r = CreateArray(SIMPLE_TYPE(), SIMPLE_EXPR());
      break;
    }

    case AST_tTUPLE: {
      r = CreateTuple();
      break;
    }

    case AST_tTEMPLATE: {
      r = CreateTemplateType(SIMPLE_TYPE());
      break;
    }

    case AST_tFUNCTION: {
      r = CreateFunctionType(SIMPLE_TYPE());
      break;
    }

    case AST_sIF: {
      r = CreateIf(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_sSWITCH: {
      r = CreateSwitch(SIMPLE_EXPR());
      break;
    }

    case AST_sCASE: {
      r = CreateCase(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_sRET: {
      r = CreateReturn();
      break;
    }

    case AST_sBRK: {
      r = CreateBreak();
      break;
    }

    case AST_sCONT: {
      r = CreateContinue();
      break;
    }

    case AST_sWHILE: {
      r = CreateWhile(SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_sFOR: {
      r = CreateFor(nullptr, nullptr, nullptr, SIMPLE_EXPR());
      break;
    }

    case AST_sFOREACH: {
      r = CreateForeach("_", "_", SIMPLE_EXPR(), SIMPLE_EXPR());
      break;
    }

    case AST_sASM: {
      r = CreateAssembly("");
      break;
    }

    case AST_sTYPEDEF: {
      r = CreateTypedef("_", SIMPLE_TYPE());
      break;
    }

    case AST_sSTRUCT: {
      r = CreateStruct();
      break;
    }

    case AST_sENUM: {
      r = CreateEnum("");
      break;
    }

    case AST_sSCOPE: {
      r = CreateScope("_", SIMPLE_EXPR());
      break;
    }

    case AST_sBLOCK: {
      r = CreateBlock();
      break;
    }

    case AST_sEXPORT: {
      r = CreateExport(CreateMockInstance<Block>());
      break;
    }

    case AST_sVAR: {
      r = CreateVariable(VariableType::Var, "_");
      break;
    }

    case AST_sFUNCTION: {
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
