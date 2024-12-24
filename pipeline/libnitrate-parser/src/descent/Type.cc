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

NullableFlowPtr<Expr> Parser::recurse_type_range_start() {
  if (next_if(PuncColn)) {
    return std::nullopt;
  }

  auto min_value = recurse_expr({Token(Punc, PuncColn)});

  if (!next_if(PuncColn)) {
    diagnostic << current() << "Expected ':' after range start";
  }

  return min_value;
}

NullableFlowPtr<Expr> Parser::recurse_type_range_end() {
  if (next_if(PuncRBrk)) {
    return std::nullopt;
  }

  auto max_val = recurse_expr({Token(Punc, PuncRBrk)});

  if (!next_if(PuncRBrk)) {
    diagnostic << current() << "Expected ']' after range";
  }

  return max_val;
}

std::optional<CallArgs> Parser::recurse_type_template_arguments() {
  if (!next_if(OpLT)) {
    return std::nullopt;
  }

  auto args = recurse_call_arguments(Token(Oper, OpGT));

  if (!next_if(OpGT)) {
    diagnostic << current() << "Expected '>' after template arguments";
  }

  return args;
}

FlowPtr<parse::Type> Parser::recurse_type_suffix(FlowPtr<Type> base) {
  static auto bit_width_terminaters = {
      Token(Punc, PuncRPar), Token(Punc, PuncRBrk), Token(Punc, PuncLCur),
      Token(Punc, PuncRCur), Token(Punc, PuncComa), Token(Punc, PuncColn),
      Token(Punc, PuncSemi), Token(Oper, OpSet),    Token(Oper, OpMinus),
      Token(Oper, OpGT)};

  auto template_arguments = recurse_type_template_arguments();

  if (template_arguments.has_value()) {
    auto templ = make<TemplType>(base, template_arguments.value())();
    templ->set_offset(base->begin());

    base = templ;
  }

  std::pair<NullableFlowPtr<Expr>, NullableFlowPtr<Expr>> range;
  NullableFlowPtr<Expr> width;

  if (next_if(PuncColn)) {
    if (next_if(PuncLBrk)) {
      range.first = recurse_type_range_start();
      range.second = recurse_type_range_end();

      if (next_if(PuncColn)) {
        width = recurse_expr(bit_width_terminaters);
      }
    } else {
      width = recurse_expr(bit_width_terminaters);
    }
  }

  base->set_range_begin(range.first);
  base->set_range_end(range.second);
  base->set_width(width);

  if (next_if(OpTernary)) {
    auto args = CallArgs{{"0", make<TypeExpr>(base)()}};
    auto opt_type =
        make<TemplType>(make<NamedTy>("__builtin_result")(), args)();

    opt_type->set_offset(current().get_start());

    base = opt_type;
  }

  return base;
}

FlowPtr<parse::Type> Parser::recurse_function_type() {
  auto fn = recurse_function(true);

  if (!fn->is<Function>() || !fn->as<Function>()->is_declaration()) {
    diagnostic << current()
               << "Expected a function declaration but got something else";
    return mock_type();
  }

  FlowPtr<Function> fn_def = fn.as<Function>();

  auto func_ty = make<FuncTy>(fn_def->get_return(), fn_def->get_params(),
                              fn_def->is_variadic(), fn_def->get_purity(),
                              fn_def->get_attributes())();

  func_ty->set_offset(fn->begin());

  return func_ty;
}

FlowPtr<parse::Type> Parser::recurse_opaque_type() {
  if (!next_if(PuncLPar)) {
    diagnostic << current() << "Expected '(' after 'opaque'";
    return mock_type();
  }

  if (auto name = next_if(Name)) {
    if (next_if(PuncRPar)) {
      auto opaque = make<OpaqueTy>(name->as_string())();
      opaque->set_offset(current().get_start());

      return opaque;
    } else {
      diagnostic << current() << "Expected ')' after 'opaque(name'";
    }
  } else {
    diagnostic << current() << "Expected a name after 'opaque('";
  }

  return mock_type();
}

FlowPtr<parse::Type> Parser::recurse_type_by_keyword(Keyword key) {
  switch (key) {
    case Fn: {
      return recurse_function_type();
    }

    case Opaque: {
      return recurse_opaque_type();
    }

    default: {
      diagnostic << current() << "Keyword is not valid in this context";
      return mock_type();
    }
  }
}

FlowPtr<parse::Type> Parser::recurse_type_by_operator(Operator op) {
  switch (op) {
    case OpTimes: {
      auto start = current().get_start();
      auto pointee = recurse_type();
      auto ptr_ty = make<PtrTy>(pointee)();

      ptr_ty->set_offset(start);

      return ptr_ty;
    }

    case OpBitAnd: {
      auto start = current().get_start();
      auto refee = recurse_type();
      auto ref_ty = make<RefTy>(refee)();

      ref_ty->set_offset(start);

      return ref_ty;
    }

    case OpTernary: {
      auto infer = make<InferTy>()();

      infer->set_offset(current().get_start());

      return infer;
    }

    default: {
      diagnostic << current() << "Operator is not valid in this context";
      return mock_type();
    }
  }
}

FlowPtr<parse::Type> Parser::recurse_array_or_vector() {
  auto start = current().get_start();

  auto first = recurse_type();

  if (next_if(PuncRBrk)) {
    auto args = CallArgs{{"0", make<TypeExpr>(first)()}};
    auto vector = make<TemplType>(make<NamedTy>("__builtin_vec")(), args)();

    vector->set_offset(start);

    return vector;
  }

  if (!next_if(PuncSemi)) {
    diagnostic << current()
               << "Expected ';' separator in array type before size";
  }

  auto size = recurse_expr({Token(Punc, PuncRBrk)});

  if (!next_if(PuncRBrk)) {
    diagnostic << current() << "Expected ']' after array size";
  }

  auto array = make<ArrayTy>(first, size)();
  array->set_offset(start);

  return array;
}

FlowPtr<parse::Type> Parser::recurse_set_type() {
  auto start = current().get_start();

  auto set_type = recurse_type();

  if (!next_if(PuncRCur)) {
    diagnostic << current() << "Expected '}' after set type";
  }

  auto args = CallArgs{{"0", make<TypeExpr>(set_type)()}};
  auto set = make<TemplType>(make<NamedTy>("__builtin_uset")(), args)();

  set->set_offset(start);

  return set;
}

FlowPtr<parse::Type> Parser::recurse_tuple_type() {
  TupleTyItems items;

  auto start = current().get_start();

  while (true) {
    if (next_if(EofF)) {
      diagnostic << current() << "Unexpected EOF in tuple type";
      return mock_type();
    }

    if (next_if(PuncRPar)) {
      break;
    }

    auto type = recurse_type();
    items.push_back(type);

    next_if(PuncComa);
  }

  auto tuple = make<TupleTy>(items)();
  tuple->set_offset(start);

  return tuple;
}

FlowPtr<parse::Type> Parser::recurse_type_by_punctuation(Punctor punc) {
  switch (punc) {
    case PuncLBrk: {
      return recurse_array_or_vector();
    }

    case PuncLCur: {
      return recurse_set_type();
    }

    case PuncLPar: {
      return recurse_tuple_type();
    }

    default: {
      diagnostic << current() << "Punctuation is not valid in this context";
      return mock_type();
    }
  }
}

FlowPtr<parse::Type> Parser::recurse_type_by_name(string name) {
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
    diagnostic << current() << "Unknown type name: " << name;
    return mock_type();
  }

  type.value()->set_offset(current().get_start());

  return type.value();
}

FlowPtr<parse::Type> Parser::recurse_type() {
  switch (auto tok = next(); tok.get_type()) {
    case KeyW: {
      auto type = recurse_type_by_keyword(tok.as_key());

      return recurse_type_suffix(type);
    }

    case Oper: {
      auto type = recurse_type_by_operator(tok.as_op());

      return recurse_type_suffix(type);
    }

    case Punc: {
      auto type = recurse_type_by_punctuation(tok.as_punc());

      return recurse_type_suffix(type);
    }

    case Name: {
      auto type = recurse_type_by_name(tok.as_string());

      return recurse_type_suffix(type);
    }

    default: {
      diagnostic << current() << "Expected a type";

      auto type = mock_type();

      return recurse_type_suffix(type);
    }
  }
}
