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

#include <descent/Recurse.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTType.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto GeneralParser::Context::RecurseTypeRangeStart() -> NullableFlowPtr<Expr> {
  if (NextIf<PuncColn>()) {
    return std::nullopt;
  }

  auto min_value = RecurseExpr({
      Token(Punc, PuncColn),
  });

  if (!NextIf<PuncColn>()) {
    Log << ParserSignal << Current() << "Expected ':' after range start";
  }

  return min_value;
}

auto GeneralParser::Context::RecurseTypeRangeEnd() -> NullableFlowPtr<Expr> {
  if (NextIf<PuncRBrk>()) {
    return std::nullopt;
  }

  auto max_val = RecurseExpr({
      Token(Punc, PuncRBrk),
  });

  if (!NextIf<PuncRBrk>()) {
    Log << ParserSignal << Current() << "Expected ']' after range";
  }

  return max_val;
}

auto GeneralParser::Context::RecurseTypeTemplateArguments() -> std::optional<std::vector<CallArg>> {
  if (!NextIf<OpLT>()) {
    return std::nullopt;
  }

  auto args = RecurseCallArguments(
      {
          Token(Oper, OpGT),
          Token(Oper, OpRShift),
          Token(Oper, OpROTR),
      },
      true);

  if (NextIf<OpGT>()) {
  } else if (NextIf<OpRShift>()) {
    m_rd.Insert(Token(Oper, OpGT));
  } else if (NextIf<OpROTR>()) {
    m_rd.Insert(Token(Oper, OpRShift));
  } else {
    Log << ParserSignal << Current() << "Expected '>' after template arguments";
  }

  return args;
}

auto GeneralParser::Context::RecurseTypeSuffix(FlowPtr<Type> base) -> FlowPtr<parse::Type> {
  static auto bit_width_terminaters = {
      Token(Punc, PuncRPar), Token(Punc, PuncRBrk), Token(Punc, PuncLCur), Token(Punc, PuncRCur), Token(Punc, PuncComa),
      Token(Punc, PuncColn), Token(Punc, PuncSemi), Token(Oper, OpSet),    Token(Oper, OpMinus),  Token(Oper, OpGT)};

  if (auto template_arguments = RecurseTypeTemplateArguments()) {
    auto templ = CreateTemplateType(template_arguments.value(), base);
    templ->SetOffset(base->SourceBegin());

    base = templ;
  }

  std::pair<NullableFlowPtr<Expr>, NullableFlowPtr<Expr>> range;
  NullableFlowPtr<Expr> width;

  if (NextIf<PuncColn>()) {
    if (NextIf<PuncLBrk>()) {
      range.first = RecurseTypeRangeStart();
      range.second = RecurseTypeRangeEnd();

      if (NextIf<PuncColn>()) {
        width = RecurseExpr(bit_width_terminaters);
      }
    } else {
      width = RecurseExpr(bit_width_terminaters);
    }
  }

  base->SetRangeBegin(range.first);
  base->SetRangeEnd(range.second);
  base->SetWidth(width);

  if (NextIf<OpTernary>()) {
    auto args = std::vector<CallArg>{{"0", base}};
    auto opt_type = CreateTemplateType(args, CreateNamed("__builtin_result"));

    opt_type->SetOffset(Current().GetStart());

    base = opt_type;
  }

  return base;
}

auto GeneralParser::Context::RecurseFunctionType() -> FlowPtr<parse::Type> {
  auto fn = RecurseFunction(true);

  if (!fn->Is<Function>() || !fn->As<Function>()->IsDeclaration()) {
    Log << ParserSignal << Current() << "Expected a function declaration but got something else";
    return CreateMockInstance<VoidTy>();
  }

  FlowPtr<Function> fn_def = fn.As<Function>();

  auto func_ty =
      CreateFunctionType(fn_def->GetReturn(), fn_def->GetParams(), fn_def->IsVariadic(), fn_def->GetAttributes());
  if (!func_ty.has_value()) {
    Log << ParserSignal << Current() << "Function type specification is incorrect";
    func_ty = CreateMockInstance<FuncTy>();
  }

  func_ty.value()->SetOffset(fn->SourceBegin());

  return func_ty.value();
}

auto GeneralParser::Context::RecurseOpaqueType() -> FlowPtr<parse::Type> {
  if (!NextIf<PuncLPar>()) {
    Log << ParserSignal << Current() << "Expected '(' after 'opaque'";
    return CreateMockInstance<VoidTy>();
  }

  if (auto name = RecurseName()) {
    if (NextIf<PuncRPar>()) {
      auto opaque = CreateOpaque(name);
      opaque->SetOffset(Current().GetStart());

      return opaque;
    }

    Log << ParserSignal << Current() << "Expected ')' after 'opaque(name'";
  } else {
    Log << ParserSignal << Current() << "Expected a name after 'opaque('";
  }

  return CreateMockInstance<VoidTy>();
}

auto GeneralParser::Context::RecurseTypeByKeyword(Keyword key) -> FlowPtr<parse::Type> {
  Next();

  switch (key) {
    case Fn: {
      return RecurseFunctionType();
    }

    case Opaque: {
      return RecurseOpaqueType();
    }

    case Keyword::Type: {
      return RecurseType();
    }

    default: {
      Log << ParserSignal << Current() << "Unexpected '" << key << "' is type context";
      return CreateMockInstance<VoidTy>();
    }
  }
}

auto GeneralParser::Context::RecurseTypeByOperator(Operator op) -> FlowPtr<parse::Type> {
  Next();

  switch (op) {
    case OpTimes: {
      auto start = Current().GetStart();
      auto pointee = RecurseType();
      auto ptr_ty = CreatePointer(pointee);

      ptr_ty->SetOffset(start);

      return ptr_ty;
    }

    case OpBitAnd: {
      auto start = Current().GetStart();
      auto refee = RecurseType();
      auto ref_ty = CreateReference(refee);

      ref_ty->SetOffset(start);

      return ref_ty;
    }

    case OpTernary: {
      auto infer = CreateUnknownType();

      infer->SetOffset(Current().GetStart());

      return infer;
    }

    case OpComptime: {
      if (!NextIf<PuncLPar>()) {
        Log << ParserSignal << Current() << "Expected '(' after 'comptime'";
        return CreateMockInstance<VoidTy>();
      }

      auto comptime_expr = CreateUnary(OpComptime, RecurseExpr({
                                                       Token(Punc, PuncRPar),
                                                   }));

      if (!NextIf<PuncRPar>()) {
        Log << ParserSignal << Current() << "Expected ')' after 'comptime('";
      }

      auto args = std::vector<CallArg>{{"0", comptime_expr}};
      return CreateTemplateType(std::move(args), CreateNamed("__builtin_meta"));
    }

    default: {
      Log << ParserSignal << Current() << "Unexpected operator '" << op << "' in type context";
      return CreateMockInstance<VoidTy>();
    }
  }
}

auto GeneralParser::Context::RecurseArrayOrVector() -> FlowPtr<parse::Type> {
  auto start = Current().GetStart();

  auto first = RecurseType();

  if (NextIf<PuncRBrk>()) {
    auto args = std::vector<CallArg>{{"0", first}};
    auto vector = CreateTemplateType(args, CreateNamed("__builtin_vec"));

    vector->SetOffset(start);

    return vector;
  }

  if (!NextIf<PuncSemi>()) {
    Log << ParserSignal << Current() << "Expected ';' separator in array type before size";
  }

  auto size = RecurseExpr({
      Token(Punc, PuncRBrk),
  });

  if (!NextIf<PuncRBrk>()) {
    Log << ParserSignal << Current() << "Expected ']' after array size";
  }

  auto array = CreateArray(first, size);
  array->SetOffset(start);

  return array;
}

auto GeneralParser::Context::RecurseSetType() -> FlowPtr<parse::Type> {
  auto start = Current().GetStart();

  auto set_type = RecurseType();

  if (!NextIf<PuncRCur>()) {
    Log << ParserSignal << Current() << "Expected '}' after set type";
  }

  auto args = std::vector<CallArg>{{"0", set_type}};
  auto set = CreateTemplateType(args, CreateNamed("__builtin_uset"));

  set->SetOffset(start);

  return set;
}

auto GeneralParser::Context::RecurseTupleType() -> FlowPtr<parse::Type> {
  std::vector<FlowPtr<Type>> items;

  auto start = Current().GetStart();

  while (true) {
    if (m_rd.IsEof()) {
      Log << ParserSignal << Current() << "Unexpected EOF in tuple type";
      return CreateMockInstance<VoidTy>();
    }

    if (NextIf<PuncRPar>()) {
      break;
    }

    auto type = RecurseType();
    items.push_back(type);

    NextIf<PuncComa>();
  }

  auto tuple = CreateTuple(items);
  tuple->SetOffset(start);

  return tuple;
}

auto GeneralParser::Context::RecurseTypeByPunctuation(Punctor punc) -> FlowPtr<parse::Type> {
  switch (punc) {
    case PuncLBrk: {
      Next();
      return RecurseArrayOrVector();
    }

    case PuncLCur: {
      Next();
      return RecurseSetType();
    }

    case PuncLPar: {
      Next();
      return RecurseTupleType();
    }

    case PuncScope: {
      return RecurseTypeByName(RecurseName());
    }

    default: {
      Log << ParserSignal << Next() << "Punctuation is not valid in this context";
      return CreateMockInstance<VoidTy>();
    }
  }
}

auto GeneralParser::Context::RecurseTypeByName(string name) -> FlowPtr<parse::Type> {
  NullableFlowPtr<Type> type;

  if (name == "u1") {
    type = CreateU1();
  } else if (name == "u8") {
    type = CreateU8();
  } else if (name == "u16") {
    type = CreateU16();
  } else if (name == "u32") {
    type = CreateU32();
  } else if (name == "u64") {
    type = CreateU64();
  } else if (name == "u128") {
    type = CreateU128();
  } else if (name == "i8") {
    type = CreateI8();
  } else if (name == "i16") {
    type = CreateI16();
  } else if (name == "i32") {
    type = CreateI32();
  } else if (name == "i64") {
    type = CreateI64();
  } else if (name == "i128") {
    type = CreateI128();
  } else if (name == "f16") {
    type = CreateF16();
  } else if (name == "f32") {
    type = CreateF32();
  } else if (name == "f64") {
    type = CreateF64();
  } else if (name == "f128") {
    type = CreateF128();
  } else if (name == "void") {
    type = CreateVoid();
  } else {
    type = CreateNamed(name);
  }

  if (!type.has_value()) {
    Log << ParserSignal << Current() << "Unknown type name: " << name;
    return CreateMockInstance<VoidTy>();
  }

  type.value()->SetOffset(Current().GetStart());

  return type.value();
}

auto GeneralParser::Context::RecurseType() -> FlowPtr<parse::Type> {
  auto comments = m_rd.CommentBuffer();
  m_rd.ClearCommentBuffer();

  std::optional<FlowPtr<Type>> r;

  switch (auto tok = Peek(); tok.GetKind()) {
    case KeyW: {
      auto type = RecurseTypeByKeyword(tok.GetKeyword());
      r = RecurseTypeSuffix(type);
      break;
    }

    case Oper: {
      auto type = RecurseTypeByOperator(tok.GetOperator());
      r = RecurseTypeSuffix(type);
      break;
    }

    case Punc: {
      auto type = RecurseTypeByPunctuation(tok.GetPunctor());
      r = RecurseTypeSuffix(type);
      break;
    }

    case Name: {
      auto type = RecurseTypeByName(RecurseName());
      r = RecurseTypeSuffix(type);
      break;
    }

    default: {
      Log << ParserSignal << Next() << "Expected a type";

      auto type = CreateMockInstance<VoidTy>();
      r = RecurseTypeSuffix(type);
      break;
    }
  }

  r = BindComments(r.value(), comments);

  return r.value();
}
