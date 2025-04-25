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

static auto RecurseTypeRangeStart(GeneralParser::Context& m) -> NullableFlowPtr<Expr> {
  if (m.NextIf<PuncColn>()) {
    return std::nullopt;
  }

  auto min_value = m.RecurseExpr({
      Token(Punc, PuncColn),
  });

  if (!m.NextIf<PuncColn>()) {
    Log << ParserSignal << m.Current() << "Expected ':' after range start";
  }

  return min_value;
}

static auto RecurseTypeRangeEnd(GeneralParser::Context& m) -> NullableFlowPtr<Expr> {
  if (m.NextIf<PuncRBrk>()) {
    return std::nullopt;
  }

  auto max_val = m.RecurseExpr({
      Token(Punc, PuncRBrk),
  });

  if (!m.NextIf<PuncRBrk>()) {
    Log << ParserSignal << m.Current() << "Expected ']' after range";
  }

  return max_val;
}

static auto RecurseTypeTemplateArguments(GeneralParser::Context& m) -> std::optional<std::vector<CallArg>> {
  if (!m.NextIf<OpLT>()) {
    return std::nullopt;
  }

  auto args = m.RecurseCallArguments(
      {
          Token(Oper, OpGT),
          Token(Oper, OpRShift),
          Token(Oper, OpROTR),
      },
      true);

  if (m.NextIf<OpGT>()) {
  } else if (m.NextIf<OpRShift>()) {
    m.GetScanner().Insert(Token(Oper, OpGT));
  } else if (m.NextIf<OpROTR>()) {
    m.GetScanner().Insert(Token(Oper, OpRShift));
  } else {
    Log << ParserSignal << m.Current() << "Expected '>' after template arguments";
  }

  return args;
}

static auto RecurseTypeSuffix(GeneralParser::Context& m, FlowPtr<parse::Type> base) -> FlowPtr<parse::Type> {
  static auto bit_width_terminaters = {
      Token(Punc, PuncRPar), Token(Punc, PuncRBrk), Token(Punc, PuncLCur), Token(Punc, PuncRCur), Token(Punc, PuncComa),
      Token(Punc, PuncColn), Token(Punc, PuncSemi), Token(Oper, OpSet),    Token(Oper, OpMinus),  Token(Oper, OpGT)};

  if (auto template_arguments = RecurseTypeTemplateArguments(m)) {
    auto templ = m.CreateTemplateType(template_arguments.value(), base);
    templ->SetOffset(base->SourceBegin());

    base = templ;
  }

  std::pair<NullableFlowPtr<Expr>, NullableFlowPtr<Expr>> range;
  NullableFlowPtr<Expr> width;

  if (m.NextIf<PuncColn>()) {
    if (m.NextIf<PuncLBrk>()) {
      range.first = RecurseTypeRangeStart(m);
      range.second = RecurseTypeRangeEnd(m);

      if (m.NextIf<PuncColn>()) {
        width = m.RecurseExpr(bit_width_terminaters);
      }
    } else {
      width = m.RecurseExpr(bit_width_terminaters);
    }
  }

  base->SetRangeBegin(range.first);
  base->SetRangeEnd(range.second);
  base->SetWidth(width);

  if (m.NextIf<OpTernary>()) {
    auto args = std::vector<CallArg>{{"0", base}};
    auto opt_type = m.CreateTemplateType(args, m.CreateNamed("__builtin_result"));

    opt_type->SetOffset(m.Current().GetStart());

    base = opt_type;
  }

  return base;
}

static auto RecurseFunctionType(GeneralParser::Context& m) -> FlowPtr<parse::Type> {
  auto fn = m.RecurseFunction(true);

  if (!fn->Is<Function>() || !fn->As<Function>()->IsDeclaration()) {
    Log << ParserSignal << m.Current() << "Expected a function declaration but got something else";
    return m.CreateMockInstance<InferTy>();
  }

  FlowPtr<Function> fn_def = fn.As<Function>();

  auto func_ty =
      m.CreateFunctionType(fn_def->GetReturn(), fn_def->GetParams(), fn_def->IsVariadic(), fn_def->GetAttributes());
  if (!func_ty) {
    Log << ParserSignal << m.Current() << "Function type specification is incorrect";
    func_ty = m.CreateMockInstance<FuncTy>();
  }

  func_ty.value()->SetOffset(fn->SourceBegin());

  return func_ty.value();
}

static auto RecurseOpaqueType(GeneralParser::Context& m) -> FlowPtr<parse::Type> {
  if (!m.NextIf<PuncLPar>()) {
    Log << ParserSignal << m.Current() << "Expected '(' after 'opaque'";
    return m.CreateMockInstance<InferTy>();
  }

  if (auto name = m.RecurseName()) {
    if (m.NextIf<PuncRPar>()) {
      auto opaque = m.CreateOpaque(name);
      opaque->SetOffset(m.Current().GetStart());

      return opaque;
    }

    Log << ParserSignal << m.Current() << "Expected ')' after 'opaque(name'";
  } else {
    Log << ParserSignal << m.Current() << "Expected a name after 'opaque('";
  }

  return m.CreateMockInstance<InferTy>();
}

static auto RecurseTypeByKeyword(GeneralParser::Context& m, Keyword key) -> FlowPtr<parse::Type> {
  m.Next();

  switch (key) {
    case Fn: {
      return RecurseFunctionType(m);
    }

    case Opaque: {
      return RecurseOpaqueType(m);
    }

    case Keyword::Type: {
      return m.RecurseType();
    }

    default: {
      Log << ParserSignal << m.Current() << "Unexpected '" << key << "' is type context";
      return m.CreateMockInstance<InferTy>();
    }
  }
}

static auto RecurseTypeByOperator(GeneralParser::Context& m, Operator op) -> FlowPtr<parse::Type> {
  m.Next();

  switch (op) {
    case OpTimes: {
      auto start = m.Current().GetStart();
      auto pointee = m.RecurseType();
      auto ptr_ty = m.CreatePointer(pointee);

      ptr_ty->SetOffset(start);

      return ptr_ty;
    }

    case OpBitAnd: {
      auto start = m.Current().GetStart();
      auto refee = m.RecurseType();
      auto ref_ty = m.CreateReference(refee);

      ref_ty->SetOffset(start);

      return ref_ty;
    }

    case OpTernary: {
      auto infer = m.CreateInferredType();
      infer->SetOffset(m.Current().GetStart());
      return infer;
    }

    case OpComptime: {
      if (!m.NextIf<PuncLPar>()) {
        Log << ParserSignal << m.Current() << "Expected '(' after 'comptime'";
        return m.CreateMockInstance<InferTy>();
      }

      auto comptime_expr = m.CreateUnary(OpComptime, m.RecurseExpr({
                                                         Token(Punc, PuncRPar),
                                                     }));

      if (!m.NextIf<PuncRPar>()) {
        Log << ParserSignal << m.Current() << "Expected ')' after 'comptime('";
      }

      auto args = std::vector<CallArg>{{"0", comptime_expr}};
      return m.CreateTemplateType(std::move(args), m.CreateNamed("__builtin_meta"));
    }

    default: {
      Log << ParserSignal << m.Current() << "Unexpected operator '" << op << "' in type context";
      return m.CreateMockInstance<InferTy>();
    }
  }
}

static auto RecurseArrayOrVector(GeneralParser::Context& m) -> FlowPtr<parse::Type> {
  auto start = m.Current().GetStart();
  auto first = m.RecurseType();

  if (m.NextIf<PuncRBrk>()) {
    auto args = std::vector<CallArg>{{"0", first}};
    auto vector = m.CreateTemplateType(args, m.CreateNamed("__builtin_vec"));

    vector->SetOffset(start);

    return vector;
  }

  if (!m.NextIf<PuncSemi>()) {
    Log << ParserSignal << m.Current() << "Expected ';' separator in array type before size";
  }

  auto size = m.RecurseExpr({
      Token(Punc, PuncRBrk),
  });

  if (!m.NextIf<PuncRBrk>()) {
    Log << ParserSignal << m.Current() << "Expected ']' after array size";
  }

  auto array = m.CreateArray(first, size);
  array->SetOffset(start);

  return array;
}

static auto RecurseSetType(GeneralParser::Context& m) -> FlowPtr<parse::Type> {
  auto start = m.Current().GetStart();
  auto set_type = m.RecurseType();

  if (!m.NextIf<PuncRCur>()) {
    Log << ParserSignal << m.Current() << "Expected '}' after set type";
  }

  auto args = std::vector<CallArg>{{"0", set_type}};
  auto set = m.CreateTemplateType(args, m.CreateNamed("__builtin_uset"));

  set->SetOffset(start);

  return set;
}

static auto RecurseTupleType(GeneralParser::Context& m) -> FlowPtr<parse::Type> {
  std::vector<FlowPtr<parse::Type>> items;
  auto start = m.Current().GetStart();

  while (true) {
    if (m.IsEof()) {
      Log << ParserSignal << m.Current() << "Unexpected EOF in tuple type";
      return m.CreateMockInstance<InferTy>();
    }

    if (m.NextIf<PuncRPar>()) {
      break;
    }

    auto type = m.RecurseType();
    items.push_back(type);

    m.NextIf<PuncComa>();
  }

  auto tuple = m.CreateTuple(items);
  tuple->SetOffset(start);

  return tuple;
}

static auto RecurseTypeByName(GeneralParser::Context& m, string name) -> FlowPtr<parse::Type> {
  auto type = m.CreateNamed(name);
  type->SetOffset(m.Current().GetStart());

  return type;
}

static auto RecurseTypeByPunctuation(GeneralParser::Context& m, Punctor punc) -> FlowPtr<parse::Type> {
  switch (punc) {
    case PuncLBrk: {
      m.Next();
      return RecurseArrayOrVector(m);
    }

    case PuncLCur: {
      m.Next();
      return RecurseSetType(m);
    }

    case PuncLPar: {
      m.Next();
      return RecurseTupleType(m);
    }

    case PuncScope: {
      return RecurseTypeByName(m, m.RecurseName());
    }

    default: {
      Log << ParserSignal << m.Next() << "Punctuation is not valid in this context";
      return m.CreateMockInstance<InferTy>();
    }
  }
}

auto GeneralParser::Context::RecurseType() -> FlowPtr<parse::Type> {
  auto& m = *this;

  std::optional<FlowPtr<Type>> r;

  switch (auto tok = Peek(); tok.GetKind()) {
    case KeyW: {
      r = RecurseTypeByKeyword(m, tok.GetKeyword());
      break;
    }

    case Oper: {
      r = RecurseTypeByOperator(m, tok.GetOperator());
      break;
    }

    case Punc: {
      r = RecurseTypeByPunctuation(m, tok.GetPunctor());
      break;
    }

    case Name: {
      r = RecurseTypeByName(m, RecurseName());
      break;
    }

    default: {
      Log << ParserSignal << Next() << "Expected a type";
      r = CreateMockInstance<InferTy>();
      break;
    }
  }

  r = RecurseTypeSuffix(m, r.value());

  return r.value();
}
