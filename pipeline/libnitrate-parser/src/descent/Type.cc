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

#include <nitrate-lexer/Lexer.h>
#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Node.h>

#include <descent/Recurse.hh>

using namespace npar;

static std::optional<Expr *> recurse_type_range_start(npar_t &S, qlex_t &rd) {
  if (next_if(qPuncColn)) {
    return std::nullopt;
  }

  let min_value = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncColn)});

  if (!next_if(qPuncColn)) {
    diagnostic << current() << "Expected ':' after range start";
  }

  return min_value;
}

static std::optional<Expr *> recurse_type_range_end(npar_t &S, qlex_t &rd) {
  if (next_if(qPuncRBrk)) {
    return std::nullopt;
  }

  let max_val = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRBrk)});

  if (!next_if(qPuncRBrk)) {
    diagnostic << current() << "Expected ']' after range";
  }

  return max_val;
}

static void recurse_type_metadata(npar_t &S, qlex_t &rd, Type *base) {
  static let bit_width_terminaters = {
      qlex_tok_t(qPunc, qPuncRPar), qlex_tok_t(qPunc, qPuncRBrk),
      qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qPunc, qPuncRCur),
      qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncColn),
      qlex_tok_t(qPunc, qPuncSemi), qlex_tok_t(qOper, qOpSet),
      qlex_tok_t(qOper, qOpMinus),  qlex_tok_t(qOper, qOpGT)};

  std::pair<std::optional<Expr *>, std::optional<Expr *>> range;
  std::optional<Expr *> width;

  if (next_if(qPuncColn)) {
    if (next_if(qPuncLBrk)) {
      range.first = recurse_type_range_start(S, rd);
      range.second = recurse_type_range_end(S, rd);

      if (next_if(qPuncColn)) {
        width = recurse_expr(S, rd, bit_width_terminaters);
      }
    } else {
      width = recurse_expr(S, rd, bit_width_terminaters);
    }
  }

  base->set_range(range.first.value_or(nullptr),
                  range.second.value_or(nullptr));
  base->set_width(width.value_or(nullptr));
}

static Type *recurse_function_type(npar_t &S, qlex_t &rd) {
  let fn = recurse_function(S, rd);

  if (!fn->is<FnDecl>()) {
    diagnostic << current()
               << "Expected a function declaration but got something else";
    return mock_type();
  }

  let type = fn->as<FnDecl>()->get_type();
  type->set_end_pos(current().end);

  return type;
}

static Type *recurse_opaque_type(qlex_t &rd) {
  if (!next_if(qPuncLPar)) {
    diagnostic << current() << "Expected '(' after 'opaque'";
    return mock_type();
  }

  if (let name = next_if(qName)) {
    if (next_if(qPuncRPar)) {
      let opaque = OpaqueTy::get(name->as_string(&rd));
      opaque->set_end_pos(current().end);

      return opaque;
    } else {
      diagnostic << current() << "Expected ')' after 'opaque(name'";
    }
  } else {
    diagnostic << current() << "Expected a name after 'opaque('";
  }

  return mock_type();
}

static Type *recurse_type_by_keyword(npar_t &S, qlex_t &rd, qlex_key_t key) {
  switch (key) {
    case qKFn: {
      return recurse_function_type(S, rd);
    }

    case qKOpaque: {
      return recurse_opaque_type(rd);
    }

    default: {
      diagnostic << current() << "Keyword is not valid in this context";
      return mock_type();
    }
  }
}

static Type *recurse_type_by_operator(npar_t &S, qlex_t &rd, qlex_op_t op) {
  switch (op) {
    case qOpTimes: {
      let pointee = recurse_type(S, rd);

      let ptr_ty = PtrTy::get(pointee);
      ptr_ty->set_end_pos(current().end);

      return ptr_ty;
    }

    case qOpBitAnd: {
      let refee = recurse_type(S, rd);

      let ref_ty = RefTy::get(refee);
      ref_ty->set_end_pos(current().end);

      return ref_ty;
    }

    default: {
      diagnostic << current() << "Operator is not valid in this context";
      return mock_type();
    }
  }
}

static Type *recurse_array_or_vector(npar_t &S, qlex_t &rd) {
  let first = recurse_type(S, rd);

  if (next_if(qPuncRBrk)) {
    let vector = TemplType::get(NamedTy::get("__builtin_vec"),
                                TemplTypeArgs{TypeExpr::get(first)});
    vector->set_end_pos(current().end);

    return vector;
  }

  if (!next_if(qPuncSemi)) {
    diagnostic << current()
               << "Expected ';' separator in array type before size";
  }

  let size = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRBrk)});

  if (!next_if(qPuncRBrk)) {
    diagnostic << current() << "Expected ']' after array size";
  }

  let array = ArrayTy::get(first, size);
  array->set_end_pos(current().end);

  return array;
}

static Type *recurse_set_type(npar_t &S, qlex_t &rd) {
  let set_type = recurse_type(S, rd);

  if (!next_if(qPuncRCur)) {
    diagnostic << current() << "Expected '}' after set type";
  }

  let set = TemplType::get(NamedTy::get("__builtin_uset"),
                           TemplTypeArgs{TypeExpr::get(set_type)});
  set->set_end_pos(current().end);

  return set;
}

static Type *recurse_tuple_type(npar_t &S, qlex_t &rd) {
  TupleTyItems items;

  while (true) {
    if (peek().is(qEofF)) {
      diagnostic << current() << "Unexpected EOF in tuple type";
      return mock_type();
    }

    if (next_if(qPuncRPar)) {
      break;
    }

    let type = recurse_type(S, rd);
    items.push_back(type);

    next_if(qPuncComa);
  }

  let tuple = TupleTy::get(std::move(items));
  tuple->set_end_pos(current().end);

  return tuple;
}

static Type *recurse_type_by_punctuation(npar_t &S, qlex_t &rd,
                                         qlex_punc_t punc) {
  switch (punc) {
    case qPuncLBrk: {
      return recurse_array_or_vector(S, rd);
    }

    case qPuncLCur: {
      return recurse_set_type(S, rd);
    }

    case qPuncLPar: {
      return recurse_tuple_type(S, rd);
    }

    default: {
      diagnostic << current() << "Punctuation is not valid in this context";
      return mock_type();
    }
  }
}

static Type *recurse_type_by_name(qlex_t &rd, std::string_view name) {
  std::optional<Type *> type;

  if (name == "u1") {
    type = U1::get();
  } else if (name == "u8") {
    type = U8::get();
  } else if (name == "u16") {
    type = U16::get();
  } else if (name == "u32") {
    type = U32::get();
  } else if (name == "u64") {
    type = U64::get();
  } else if (name == "u128") {
    type = U128::get();
  } else if (name == "i8") {
    type = I8::get();
  } else if (name == "i16") {
    type = I16::get();
  } else if (name == "i32") {
    type = I32::get();
  } else if (name == "i64") {
    type = I64::get();
  } else if (name == "i128") {
    type = I128::get();
  } else if (name == "f16") {
    type = F16::get();
  } else if (name == "f32") {
    type = F32::get();
  } else if (name == "f64") {
    type = F64::get();
  } else if (name == "f128") {
    type = F128::get();
  } else if (name == "void") {
    type = VoidTy::get();
  } else {
    type = NamedTy::get(name);
  }

  if (!type.has_value()) {
    diagnostic << current() << "Unknown type name: " << name;
    return mock_type();
  }

  type.value()->set_end_pos(current().end);

  return type.value();
}

Type *npar::recurse_type(npar_t &S, qlex_t &rd) {
  switch (let tok = next(); tok.ty) {
    case qKeyW: {
      let type = recurse_type_by_keyword(S, rd, tok.v.key);
      recurse_type_metadata(S, rd, type);

      return type;
    }

    case qOper: {
      let type = recurse_type_by_operator(S, rd, tok.v.op);
      recurse_type_metadata(S, rd, type);

      return type;
    }

    case qPunc: {
      let type = recurse_type_by_punctuation(S, rd, tok.v.punc);
      recurse_type_metadata(S, rd, type);

      return type;
    }

    case qName: {
      let type = recurse_type_by_name(rd, tok.as_string(&rd));
      recurse_type_metadata(S, rd, type);

      return type;
    }

    default: {
      diagnostic << current() << "Expected a type";

      let type = mock_type();
      recurse_type_metadata(S, rd, type);

      return type;
    }
  }
}
