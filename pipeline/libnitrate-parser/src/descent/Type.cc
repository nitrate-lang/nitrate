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

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto Parser::PImpl::RecurseTypeRangeStart() -> NullableFlowPtr<Expr> {
  if (next_if(PuncColn)) {
    return std::nullopt;
  }

  auto min_value = RecurseExpr({
      Token(Punc, PuncColn),
  });

  if (!next_if(PuncColn)) {
    Log << SyntaxError << current() << "Expected ':' after range start";
  }

  return min_value;
}

auto Parser::PImpl::RecurseTypeRangeEnd() -> NullableFlowPtr<Expr> {
  if (next_if(PuncRBrk)) {
    return std::nullopt;
  }

  auto max_val = RecurseExpr({
      Token(Punc, PuncRBrk),
  });

  if (!next_if(PuncRBrk)) {
    Log << SyntaxError << current() << "Expected ']' after range";
  }

  return max_val;
}

auto Parser::PImpl::RecurseTypeTemplateArguments() -> std::optional<CallArgs> {
  if (!next_if(OpLT)) {
    return std::nullopt;
  }

  auto args = RecurseCallArguments(
      {
          Token(Oper, OpGT),
          Token(Oper, OpRShift),
          Token(Oper, OpROTR),
      },
      true);

  if (next_if(OpGT)) {
  } else if (next_if(OpRShift)) {
    m_rd.Insert(Token(Oper, OpGT));
  } else if (next_if(OpROTR)) {
    m_rd.Insert(Token(Oper, OpRShift));
  } else {
    Log << SyntaxError << current() << "Expected '>' after template arguments";
  }

  return args;
}

auto Parser::PImpl::RecurseTypeSuffix(FlowPtr<Type> base)
    -> FlowPtr<parse::Type> {
  static auto bit_width_terminaters = {
      Token(Punc, PuncRPar), Token(Punc, PuncRBrk), Token(Punc, PuncLCur),
      Token(Punc, PuncRCur), Token(Punc, PuncComa), Token(Punc, PuncColn),
      Token(Punc, PuncSemi), Token(Oper, OpSet),    Token(Oper, OpMinus),
      Token(Oper, OpGT)};

  auto template_arguments = RecurseTypeTemplateArguments();

  if (template_arguments.has_value()) {
    auto templ = make<TemplType>(base, template_arguments.value())();
    templ->SetOffset(base->Begin());

    base = templ;
  }

  std::pair<NullableFlowPtr<Expr>, NullableFlowPtr<Expr>> range;
  NullableFlowPtr<Expr> width;

  if (next_if(PuncColn)) {
    if (next_if(PuncLBrk)) {
      range.first = RecurseTypeRangeStart();
      range.second = RecurseTypeRangeEnd();

      if (next_if(PuncColn)) {
        width = RecurseExpr(bit_width_terminaters);
      }
    } else {
      width = RecurseExpr(bit_width_terminaters);
    }
  }

  base->SetRangeBegin(range.first);
  base->SetRangeEnd(range.second);
  base->SetWidth(width);

  if (next_if(OpTernary)) {
    auto args = CallArgs{{"0", make<TypeExpr>(base)()}};
    auto opt_type =
        make<TemplType>(make<NamedTy>("__builtin_result")(), args)();

    opt_type->SetOffset(current().GetStart());

    base = opt_type;
  }

  return base;
}

auto Parser::PImpl::RecurseFunctionType() -> FlowPtr<parse::Type> {
  auto fn = RecurseFunction(true);

  if (!fn->Is<Function>() || !fn->as<Function>()->IsDeclaration()) {
    Log << SyntaxError << current()
        << "Expected a function declaration but got something else";
    return MockType();
  }

  FlowPtr<Function> fn_def = fn.as<Function>();

  auto func_ty = make<FuncTy>(fn_def->GetReturn(), fn_def->GetParams(),
                              fn_def->IsVariadic(), fn_def->GetPurity(),
                              fn_def->GetAttributes())();

  func_ty->SetOffset(fn->Begin());

  return func_ty;
}

auto Parser::PImpl::RecurseOpaqueType() -> FlowPtr<parse::Type> {
  if (!next_if(PuncLPar)) {
    Log << SyntaxError << current() << "Expected '(' after 'opaque'";
    return MockType();
  }

  if (auto name = RecurseName(); !name->empty()) {
    if (next_if(PuncRPar)) {
      auto opaque = make<OpaqueTy>(name)();
      opaque->SetOffset(current().GetStart());

      return opaque;
    }

    Log << SyntaxError << current() << "Expected ')' after 'opaque(name'";
  } else {
    Log << SyntaxError << current() << "Expected a name after 'opaque('";
  }

  return MockType();
}

auto Parser::PImpl::RecurseTypeByKeyword(Keyword key) -> FlowPtr<parse::Type> {
  next();

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
      Log << SyntaxError << current() << "Unexpected '" << key
          << "' is type context";
      return MockType();
    }
  }
}

auto Parser::PImpl::RecurseTypeByOperator(Operator op) -> FlowPtr<parse::Type> {
  next();

  switch (op) {
    case OpTimes: {
      auto start = current().GetStart();
      auto pointee = RecurseType();
      auto ptr_ty = make<PtrTy>(pointee, false)();

      ptr_ty->SetOffset(start);

      return ptr_ty;
    }

    case OpBitAnd: {
      auto start = current().GetStart();
      auto refee = RecurseType();
      auto ref_ty = make<RefTy>(refee)();

      ref_ty->SetOffset(start);

      return ref_ty;
    }

    case OpTernary: {
      auto infer = make<InferTy>()();

      infer->SetOffset(current().GetStart());

      return infer;
    }

    case OpComptime: {
      if (!next_if(PuncLPar)) {
        Log << SyntaxError << current() << "Expected '(' after 'comptime'";
        return MockType();
      }

      auto comptime_expr =
          make<UnaryExpr>(OpComptime, RecurseExpr({
                                          Token(Punc, PuncRPar),
                                      }))();

      if (!next_if(PuncRPar)) {
        Log << SyntaxError << current() << "Expected ')' after 'comptime('";
      }

      auto args = CallArgs{{"0", comptime_expr}};
      return make<TemplType>(make<NamedTy>("__builtin_meta")(),
                             std::move(args))();
    }

    default: {
      Log << SyntaxError << current() << "Unexpected operator '" << op
          << "' in type context";
      return MockType();
    }
  }
}

auto Parser::PImpl::RecurseArrayOrVector() -> FlowPtr<parse::Type> {
  auto start = current().GetStart();

  auto first = RecurseType();

  if (next_if(PuncRBrk)) {
    auto args = CallArgs{{"0", make<TypeExpr>(first)()}};
    auto vector = make<TemplType>(make<NamedTy>("__builtin_vec")(), args)();

    vector->SetOffset(start);

    return vector;
  }

  if (!next_if(PuncSemi)) {
    Log << SyntaxError << current()
        << "Expected ';' separator in array type before size";
  }

  auto size = RecurseExpr({
      Token(Punc, PuncRBrk),
  });

  if (!next_if(PuncRBrk)) {
    Log << SyntaxError << current() << "Expected ']' after array size";
  }

  auto array = make<ArrayTy>(first, size)();
  array->SetOffset(start);

  return array;
}

auto Parser::PImpl::RecurseSetType() -> FlowPtr<parse::Type> {
  auto start = current().GetStart();

  auto set_type = RecurseType();

  if (!next_if(PuncRCur)) {
    Log << SyntaxError << current() << "Expected '}' after set type";
  }

  auto args = CallArgs{{"0", make<TypeExpr>(set_type)()}};
  auto set = make<TemplType>(make<NamedTy>("__builtin_uset")(), args)();

  set->SetOffset(start);

  return set;
}

auto Parser::PImpl::RecurseTupleType() -> FlowPtr<parse::Type> {
  TupleTyItems items;

  auto start = current().GetStart();

  while (true) {
    if (next_if(EofF)) {
      Log << SyntaxError << current() << "Unexpected EOF in tuple type";
      return MockType();
    }

    if (next_if(PuncRPar)) {
      break;
    }

    auto type = RecurseType();
    items.push_back(type);

    next_if(PuncComa);
  }

  auto tuple = make<TupleTy>(items)();
  tuple->SetOffset(start);

  return tuple;
}

auto Parser::PImpl::RecurseTypeByPunctuation(Punctor punc)
    -> FlowPtr<parse::Type> {
  switch (punc) {
    case PuncLBrk: {
      next();
      return RecurseArrayOrVector();
    }

    case PuncLCur: {
      next();
      return RecurseSetType();
    }

    case PuncLPar: {
      next();
      return RecurseTupleType();
    }

    case PuncScope: {
      return RecurseTypeByName(RecurseName());
    }

    default: {
      Log << SyntaxError << current()
          << "Punctuation is not valid in this context";
      return MockType();
    }
  }
}

auto Parser::PImpl::RecurseTypeByName(string name) -> FlowPtr<parse::Type> {
  NullableFlowPtr<Type> type;

  if (name == "u1") {
    type = make<U1>()();
  } else if (name == "u8") {
    type = make<U8>()();
  } else if (name == "u16") {
    type = make<U16>()();
  } else if (name == "u32") {
    type = make<U32>()();
  } else if (name == "u64") {
    type = make<U64>()();
  } else if (name == "u128") {
    type = make<U128>()();
  } else if (name == "i8") {
    type = make<I8>()();
  } else if (name == "i16") {
    type = make<I16>()();
  } else if (name == "i32") {
    type = make<I32>()();
  } else if (name == "i64") {
    type = make<I64>()();
  } else if (name == "i128") {
    type = make<I128>()();
  } else if (name == "f16") {
    type = make<F16>()();
  } else if (name == "f32") {
    type = make<F32>()();
  } else if (name == "f64") {
    type = make<F64>()();
  } else if (name == "f128") {
    type = make<F128>()();
  } else if (name == "void") {
    type = make<VoidTy>()();
  } else {
    type = make<NamedTy>(name)();
  }

  if (!type.has_value()) {
    Log << SyntaxError << current() << "Unknown type name: " << name;
    return MockType();
  }

  type.value()->SetOffset(current().GetStart());

  return type.value();
}

auto Parser::PImpl::RecurseType() -> FlowPtr<parse::Type> {
  auto comments = m_rd.CommentBuffer();
  m_rd.ClearCommentBuffer();

  std::optional<FlowPtr<Type>> r;

  switch (auto tok = peek(); tok.GetKind()) {
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
      Log << SyntaxError << next() << "Expected a type";

      auto type = MockType();
      r = RecurseTypeSuffix(type);
      break;
    }
  }

  r = BindComments(r.value(), comments);

  return r.value();
}
