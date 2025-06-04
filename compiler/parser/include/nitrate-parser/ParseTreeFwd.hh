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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

namespace nitrate::compiler::parser {
  enum class ASTKind : uint8_t {
    Discarded,

    /* General Expression Nodes */
    gBinExpr,
    gUnaryExpr,
    gNumber,
    gFString,
    gString,
    gChar,
    gList,
    gIdent,
    gIndex,
    gSlice,
    gCall,
    gTemplateCall,
    gIf,
    gElse,
    gFor,
    gWhile,
    gDo,
    gSwitch,
    gBreak,
    gContinue,
    gReturn,
    gForeach,
    gTry,
    gCatch,
    gThrow,
    gAwait,
    gAsm,

    /* Type Nodes */
    tInfer,
    tOpaque,
    tNamed,
    tRef,
    tPtr,
    tArray,
    tTuple,
    tTemplate,
    tLambda,

    /* Symbol Nodes */
    sLet,
    sVar,
    sFn,
    sEnum,
    sStruct,
    sUnion,
    sContract,
    sTrait,
    sTypeDef,
    sScope,
    sImport,
    sUnitTest,
  };

  constexpr ASTKind ASTKIND_MIN = ASTKind::Discarded;
  constexpr ASTKind ASTKIND_MAX = ASTKind::sUnitTest;

  class SymbolTable;

  class Visitor;
  class ConstVisitor;

  class Expr;
  class Type;

  class BinExpr;
  class UnaryExpr;
  class Number;
  class FString;
  class String;
  class Char;
  class List;
  class Ident;
  class Index;
  class Slice;
  class Call;
  class TemplateCall;
  class If;
  class Else;
  class For;
  class While;
  class Do;
  class Switch;
  class Break;
  class Continue;
  class Return;
  class Foreach;
  class Try;
  class Catch;
  class Throw;
  class Await;
  class Asm;

  class InferTy;
  class OpaqueTy;
  class NamedTy;
  class RefTy;
  class PtrTy;
  class ArrayTy;
  class TupleTy;
  class TemplateTy;
  class LambdaTy;

  class Let;
  class Var;
  class Fn;
  class Enum;
  class Struct;
  class Union;
  class Contract;
  class Trait;
  class TypeDef;
  class Scope;
  class Import;
  class UnitTest;
}  // namespace nitrate::compiler::parser
