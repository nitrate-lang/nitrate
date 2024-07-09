////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <LibMacro.h>
#include <core/Logger.h>
#include <parsetree/Parser.h>

using namespace libquixcc;

static std::map<std::string, std::shared_ptr<TypeNode>> primitive_types = {
    {"u8", std::make_shared<U8TypeNode>()},
    {"u16", std::make_shared<U16TypeNode>()},
    {"u32", std::make_shared<U32TypeNode>()},
    {"u64", std::make_shared<U64TypeNode>()},
    {"u128", std::make_shared<U128TypeNode>()},
    {"i8", std::make_shared<I8TypeNode>()},
    {"i16", std::make_shared<I16TypeNode>()},
    {"i32", std::make_shared<I32TypeNode>()},
    {"i64", std::make_shared<I64TypeNode>()},
    {"i128", std::make_shared<I128TypeNode>()},
    {"f32", std::make_shared<F32TypeNode>()},
    {"f64", std::make_shared<F64TypeNode>()},
    {"i1", std::make_shared<BoolTypeNode>()},
    {"string", std::make_shared<StringTypeNode>()},
    {"void", std::make_shared<VoidTypeNode>()},
    {"null", std::make_shared<NullTypeNode>()}};

bool libquixcc::parse_type(quixcc_job_t &job, libquixcc::Scanner *scanner,
                           std::shared_ptr<TypeNode> &node) {
  Token tok = scanner->next();
  std::shared_ptr<TypeNode> type = nullptr, inner = nullptr;

  if (tok.type == TT::Keyword) {
    std::shared_ptr<StmtNode> fn;

    switch (tok.as<Keyword>()) {
      case Keyword::Void:
        inner = std::make_shared<VoidTypeNode>();
        goto suffix;
      case Keyword::Null:
        inner = std::make_shared<NullTypeNode>();
        goto suffix;
      case Keyword::Fn:
        if (!parse_function(job, scanner, fn)) {
          LOG(ERROR) << core::feedback[TYPE_EXPECTED_FUNCTION] << tok
                     << std::endl;
          goto error;
        }

        if (!fn->is<FunctionDeclNode>()) {
          LOG(ERROR) << core::feedback[TYPE_EXPECTED_FUNCTION] << tok
                     << std::endl;
          goto error;
        }

        inner = std::static_pointer_cast<FunctionDeclNode>(fn)->m_type;
        scanner->push(Token(TT::Punctor, Punctor::Semicolon));
        goto suffix;
      case Keyword::Opaque: {
        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::OpenParen)) {
          LOG(ERROR) << core::feedback[TYPE_OPAQUE_EXPECTED_PAREN] << tok
                     << std::endl;
          goto error;
        }
        tok = scanner->next();
        if (tok.type != TT::Identifier) {
          LOG(ERROR) << core::feedback[TYPE_OPAQUE_EXPECTED_IDENTIFIER] << tok
                     << std::endl;
          goto error;
        }
        std::string name = tok.as<std::string>();
        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::CloseParen)) {
          LOG(ERROR) << core::feedback[TYPE_OPAQUE_EXPECTED_CLOSE_PAREN] << tok
                     << std::endl;
          goto error;
        }

        inner = std::make_shared<OpaqueTypeNode>(name);
        goto suffix;
      }
      default:
        goto error;
    }
  } else if (tok.type == TT::Identifier) {
    if (primitive_types.contains(tok.as<std::string>())) {
      inner = primitive_types[tok.as<std::string>()];
      goto suffix;
    } else {
      inner = std::make_shared<UserTypeNode>(tok.as<std::string>());
      goto suffix;
    }
  } else if (tok.is<Punctor>(Punctor::OpenBracket)) {
    // Array type or Vector type or map type
    // syntax [type; size]
    if (!parse_type(job, scanner, type)) {
      LOG(ERROR) << core::feedback[TYPE_EXPECTED_TYPE] << tok << std::endl;
      goto error;
    }

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::CloseBracket)) {
      inner = std::make_shared<VectorTypeNode>(type);
      goto suffix;
    }

    if (tok.is<Operator>(Operator::Minus)) {
      /// TODO: Implement map type
      tok = scanner->next();
      if (!tok.is<Operator>(Operator::GreaterThan)) {
        LOG(ERROR) << core::feedback[TYPE_EXPECTED_MAP_ARROW] << tok
                   << std::endl;
        goto error;
      }

      std::shared_ptr<TypeNode> value_type;
      if (!parse_type(job, scanner, value_type)) {
        LOG(ERROR) << core::feedback[TYPE_EXPECTED_TYPE] << tok << std::endl;
        goto error;
      }

      tok = scanner->next();
      if (!tok.is<Punctor>(Punctor::CloseBracket)) {
        LOG(ERROR) << core::feedback[TYPE_EXPECTED_CLOSE_BRACKET] << tok
                   << std::endl;
        goto error;
      }

      inner = std::make_shared<MapTypeNode>(type, value_type);
      goto suffix;
    }

    if (!tok.is<Punctor>(Punctor::Semicolon)) {
      LOG(ERROR) << core::feedback[TYPE_EXPECTED_SEMICOLON] << tok << std::endl;
      goto error;
    }

    std::shared_ptr<ConstExprNode> size;
    if (!parse_const_expr(job, scanner,
                          Token(TT::Punctor, Punctor::CloseBracket), size)) {
      LOG(ERROR) << core::feedback[TYPE_EXPECTED_CONST_EXPR] << tok
                 << std::endl;
      goto error;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::CloseBracket)) {
      LOG(ERROR) << core::feedback[TYPE_EXPECTED_CLOSE_BRACKET] << tok
                 << std::endl;
      goto error;
    }

    inner = std::make_shared<ArrayTypeNode>(type, size);
    goto suffix;
  } else if (tok.is<Operator>(Operator::Multiply)) {
    // Pointer type
    if (!parse_type(job, scanner, type)) {
      LOG(ERROR) << core::feedback[TYPE_EXPECTED_TYPE] << tok << std::endl;
      goto error;
    }

    inner = std::make_shared<PointerTypeNode>(type);
    goto suffix;
  } else if (tok.is<Operator>(Operator::LogicalNot)) {
    // ! means mutability
    if (!parse_type(job, scanner, type)) {
      LOG(ERROR) << core::feedback[TYPE_EXPECTED_TYPE] << tok << std::endl;
      goto error;
    }

    inner = std::make_shared<MutTypeNode>(type);
    goto suffix;
  } else {
    LOG(ERROR) << core::feedback[EXPECTED_TYPE] << tok << std::endl;
    goto error;
  }

suffix:
  while (true) {
    tok = scanner->peek();
    if (tok.is<Operator>(Operator::Question)) {
      // ? means Result type
      scanner->next();
      inner = std::make_shared<ResultTypeNode>(inner);
    } else {
      break;
    }
  }

  assert(inner != nullptr);

  node = inner;

  return true;

error:
  return false;
}
