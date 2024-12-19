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

using namespace ncc::lex;
using namespace ncc::parse;

std::optional<Expr *> Parser::recurse_type_range_start() {
  if (next_if(qPuncColn)) {
    return std::nullopt;
  }

  let min_value = recurse_expr({Token(qPunc, qPuncColn)});

  if (!next_if(qPuncColn)) {
    diagnostic << current() << "Expected ':' after range start";
  }

  return min_value;
}

std::optional<Expr *> Parser::recurse_type_range_end() {
  if (next_if(qPuncRBrk)) {
    return std::nullopt;
  }

  let max_val = recurse_expr({Token(qPunc, qPuncRBrk)});

  if (!next_if(qPuncRBrk)) {
    diagnostic << current() << "Expected ']' after range";
  }

  return max_val;
}

std::optional<CallArgs> Parser::recurse_type_template_arguments() {
  if (!next_if(qOpLT)) {
    return std::nullopt;
  }

  auto args = recurse_caller_arguments(Token(qOper, qOpGT), 0);

  if (!next_if(qOpGT)) {
    diagnostic << current() << "Expected '>' after template arguments";
  }

  return args;
}

Type *Parser::recurse_type_suffix(Type *base) {
  static let bit_width_terminaters = {
      Token(qPunc, qPuncRPar), Token(qPunc, qPuncRBrk), Token(qPunc, qPuncLCur),
      Token(qPunc, qPuncRCur), Token(qPunc, qPuncComa), Token(qPunc, qPuncColn),
      Token(qPunc, qPuncSemi), Token(qOper, qOpSet),    Token(qOper, qOpMinus),
      Token(qOper, qOpGT)};

  let template_arguments = recurse_type_template_arguments();

  if (template_arguments.has_value()) {
    let templ = make<TemplType>(base, template_arguments.value());
    templ->set_offset(base->get_offset());

    base = templ;
  }

  std::pair<std::optional<Expr *>, std::optional<Expr *>> range;
  std::optional<Expr *> width;

  if (next_if(qPuncColn)) {
    if (next_if(qPuncLBrk)) {
      range.first = recurse_type_range_start();
      range.second = recurse_type_range_end();

      if (next_if(qPuncColn)) {
        width = recurse_expr(bit_width_terminaters);
      }
    } else {
      width = recurse_expr(bit_width_terminaters);
    }
  }

  base->set_range(range.first.value_or(nullptr),
                  range.second.value_or(nullptr));
  base->set_width(width.value_or(nullptr));

  if (next_if(qOpTernary)) {
    let args = CallArgs{{SaveString("0"), make<TypeExpr>(base)}};
    let opt_type =
        make<TemplType>(make<NamedTy>(SaveString("__builtin_result")), args);

    opt_type->set_offset(current().start);

    base = opt_type;
  }

  return base;
}

Type *Parser::recurse_function_type() {
  let fn = recurse_function(true);

  if (!fn->is<Function>() || !fn->as<Function>()->is_decl()) {
    diagnostic << current()
               << "Expected a function declaration but got something else";
    return mock_type();
  }

  Function *fn_def = fn->as<Function>();

  let func_ty = make<FuncTy>(fn_def->get_return(), fn_def->get_params(),
                             fn_def->get_purity(), fn_def->get_attributes());

  func_ty->set_offset(fn->get_offset());

  return func_ty;
}

Type *Parser::recurse_opaque_type() {
  if (!next_if(qPuncLPar)) {
    diagnostic << current() << "Expected '(' after 'opaque'";
    return mock_type();
  }

  if (let name = next_if(qName)) {
    if (next_if(qPuncRPar)) {
      let opaque = make<OpaqueTy>(SaveString(name->as_string()));
      opaque->set_offset(current().start);

      return opaque;
    } else {
      diagnostic << current() << "Expected ')' after 'opaque(name'";
    }
  } else {
    diagnostic << current() << "Expected a name after 'opaque('";
  }

  return mock_type();
}

Type *Parser::recurse_type_by_keyword(qlex_key_t key) {
  switch (key) {
    case qKFn: {
      return recurse_function_type();
    }

    case qKOpaque: {
      return recurse_opaque_type();
    }

    default: {
      diagnostic << current() << "Keyword is not valid in this context";
      return mock_type();
    }
  }
}

Type *Parser::recurse_type_by_operator(qlex_op_t op) {
  switch (op) {
    case qOpTimes: {
      let start = current().start;
      let pointee = recurse_type();
      let ptr_ty = make<PtrTy>(pointee);

      ptr_ty->set_offset(start);

      return ptr_ty;
    }

    case qOpBitAnd: {
      let start = current().start;
      let refee = recurse_type();
      let ref_ty = make<RefTy>(refee);

      ref_ty->set_offset(start);

      return ref_ty;
    }

    case qOpTernary: {
      let infer = make<InferTy>();

      infer->set_offset(current().start);

      return infer;
    }

    default: {
      diagnostic << current() << "Operator is not valid in this context";
      return mock_type();
    }
  }
}

Type *Parser::recurse_array_or_vector() {
  let start = current().start;

  let first = recurse_type();

  if (next_if(qPuncRBrk)) {
    let args = CallArgs{{SaveString("0"), make<TypeExpr>(first)}};
    let vector =
        make<TemplType>(make<NamedTy>(SaveString("__builtin_vec")), args);

    vector->set_offset(start);

    return vector;
  }

  if (!next_if(qPuncSemi)) {
    diagnostic << current()
               << "Expected ';' separator in array type before size";
  }

  let size = recurse_expr({Token(qPunc, qPuncRBrk)});

  if (!next_if(qPuncRBrk)) {
    diagnostic << current() << "Expected ']' after array size";
  }

  let array = make<ArrayTy>(first, size);

  array->set_offset(start);

  return array;
}

Type *Parser::recurse_set_type() {
  let start = current().start;

  let set_type = recurse_type();

  if (!next_if(qPuncRCur)) {
    diagnostic << current() << "Expected '}' after set type";
  }

  let args = CallArgs{{SaveString("0"), make<TypeExpr>(set_type)}};
  let set = make<TemplType>(make<NamedTy>(SaveString("__builtin_uset")), args);

  set->set_offset(start);

  return set;
}

Type *Parser::recurse_tuple_type() {
  TupleTyItems items;

  let start = current().start;

  while (true) {
    if (next_if(qEofF)) {
      diagnostic << current() << "Unexpected EOF in tuple type";
      return mock_type();
    }

    if (next_if(qPuncRPar)) {
      break;
    }

    let type = recurse_type();
    items.push_back(type);

    next_if(qPuncComa);
  }

  let tuple = make<TupleTy>(std::move(items));

  tuple->set_offset(start);

  return tuple;
}

Type *Parser::recurse_type_by_punctuation(qlex_punc_t punc) {
  switch (punc) {
    case qPuncLBrk: {
      return recurse_array_or_vector();
    }

    case qPuncLCur: {
      return recurse_set_type();
    }

    case qPuncLPar: {
      return recurse_tuple_type();
    }

    default: {
      diagnostic << current() << "Punctuation is not valid in this context";
      return mock_type();
    }
  }
}

Type *Parser::recurse_type_by_name(std::string_view name) {
  std::optional<Type *> type;

  if (name == "u1") {
    type = make<U1>();
  } else if (name == "u8") {
    type = make<U8>();
  } else if (name == "u16") {
    type = make<U16>();
  } else if (name == "u32") {
    type = make<U32>();
  } else if (name == "u64") {
    type = make<U64>();
  } else if (name == "u128") {
    type = make<U128>();
  } else if (name == "i8") {
    type = make<I8>();
  } else if (name == "i16") {
    type = make<I16>();
  } else if (name == "i32") {
    type = make<I32>();
  } else if (name == "i64") {
    type = make<I64>();
  } else if (name == "i128") {
    type = make<I128>();
  } else if (name == "f16") {
    type = make<F16>();
  } else if (name == "f32") {
    type = make<F32>();
  } else if (name == "f64") {
    type = make<F64>();
  } else if (name == "f128") {
    type = make<F128>();
  } else if (name == "void") {
    type = make<VoidTy>();
  } else {
    type = make<NamedTy>(SaveString(name));
  }

  if (!type.has_value()) {
    diagnostic << current() << "Unknown type name: " << name;
    return mock_type();
  }

  type.value()->set_offset(current().start);

  return type.value();
}

Type *Parser::recurse_type() {
  switch (let tok = next(); tok.ty) {
    case qKeyW: {
      let type = recurse_type_by_keyword(tok.as_key());

      return recurse_type_suffix(type);
    }

    case qOper: {
      let type = recurse_type_by_operator(tok.as_op());

      return recurse_type_suffix(type);
    }

    case qPunc: {
      let type = recurse_type_by_punctuation(tok.as_punc());

      return recurse_type_suffix(type);
    }

    case qName: {
      let type = recurse_type_by_name(tok.as_string());

      return recurse_type_suffix(type);
    }

    default: {
      diagnostic << current() << "Expected a type";

      let type = mock_type();

      return recurse_type_suffix(type);
    }
  }
}
