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

#include "quix-qxir/TypeDecl.h"
#define __QUIX_IMPL__

#define QXIR_USE_CPP_API

#include <core/LibMacro.h>
#include <quix-core/Error.h>
#include <quix-qxir/Inference.h>
#include <quix-qxir/Node.h>

#include <chrono>
#include <core/Config.hh>
#include <map>
#include <transcode/Targets.hh>

using namespace qxir;

//  static const std::unordered_map<Op, std::string_view> post_unexprs_ops = {
// {Op::Plus,      "?"},
// {Op::Minus,     "?"},
// {Op::Times,     "?"},
// {Op::Slash,     "?"},
// {Op::Percent,   "?"},
// {Op::BitAnd,    "?"},
// {Op::BitOr,     "?"},
// {Op::BitXor,    "?"},
// {Op::BitNot,    "?"},
// {Op::LogicAnd,  "?"},
// {Op::LogicOr,   "?"},
// {Op::LogicNot,  "?"},
// {Op::LShift,    "?"},
// {Op::RShift,    "?"},
// {Op::ROTR,      "?"},
// {Op::ROTL,      "?"},
// {Op::Inc,       "?"},
// {Op::Dec,       "?"},
// {Op::Set,       "?"},
// {Op::LT,        "?"},
// {Op::GT,        "?"},
// {Op::LE,        "?"},
// {Op::GE,        "?"},
// {Op::Eq,        "?"},
// {Op::NE,        "?"},
// {Op::Alignof,   "?"},
// {Op::Typeof,    "?"},
// {Op::Offsetof,  "?"},
// {Op::BitcastAs, "?"},
// {Op::CastAs,    "?"},
// {Op::Bitsizeof, "?"},
//       };

static void write_header(std::ostream &out) {
  auto now = std::chrono::system_clock::now();

  auto now_time = std::chrono::system_clock::to_time_t(now);
  std::string datetime_content = std::ctime(&now_time);
  datetime_content.pop_back();
  datetime_content = "* Generated " + datetime_content;
  if (datetime_content.size() < 68) {
    datetime_content += std::string(68 - datetime_content.size(), ' ');
  }

  out << "////////////////////////////////////////////////////////////////////////////////\n";
  out << "///                                                                          ///\n";
  out << "///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///\n";
  out << "/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///\n";
  out << "/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///\n";
  out << "/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///\n";
  out << "/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///\n";
  out << "/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///\n";
  out << "///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░   ///\n";
  out << "///    ░▒▓█▓▒░                                                               ///\n";
  out << "///     ░▒▓██▓▒░                                                             ///\n";
  out << "///                                                                          ///\n";
  out << "///   * Generated by the QUIX Compiler Suite                                 ///\n";
  out << "///     Copyright (C) 2024 Wesley C. Jones                                   ///\n";
  out << "///                                                                          ///\n";
  out << "///   Unless otherwise specified, ALL generated code is licensed under the   ///\n";
  out << "///   same license (if any) of the original work that was used to derive     ///\n";
  out << "///   this output.                                                           ///\n";
  out << "///                                                                          ///\n";
  out << "///   " << datetime_content << "   ///\n";
  out << "///                                                                          ///\n";
  out << "////////////////////////////////////////////////////////////////////////////////\n\n";
}

static void write_stdinc(std::ostream &out) {
  static const std::set<std::string_view> stdincs = {
      "stdint.h",
      "stdbool.h",
  };

  out << "#define QUIX_TRANSCODE 1\n\n";

  for (const auto &stdinc : stdincs) {
    out << "#include <" << stdinc << ">\n";
  }

  out << "\n";
}

static void write_coretypes(std::ostream &out) {
  /// TODO: Implement floating point types

  static const std::map<std::string_view, std::string_view> types = {
      {"qint128_t", "void"},  /* 128-bit signed integer*/
      {"quint128_t", "void"}, /* 128-bit unsigned integer*/

      {"qf16_t", "void"},   /* 16-bit float*/
      {"qf32_t", "float"},  /* 32-bit float*/
      {"qf64_t", "double"}, /* 64-bit float*/
      {"qf128_t", "void"},  /* 128-bit float*/
  };

  for (const auto &[alias, c11_type] : types) {
    out << "typedef " << c11_type << " " << alias << ";\n";
  }

  out << "\n";
}

struct ConvState {
  bool inside_func = false;
};

static void escape_string(std::ostream &out, std::string_view input) {
  out << '"';

  for (char ch : input) {
    switch (ch) {
      case '"':
        out << "\\\"";
        break;
      case '\\':
        out << "\\\\";
        break;
      case '\b':
        out << "\\b";
        break;
      case '\f':
        out << "\\f";
        break;
      case '\n':
        out << "\\n";
        break;
      case '\r':
        out << "\\r";
        break;
      case '\t':
        out << "\\t";
        break;
      case '\0':
        out << "\\0";
        break;
      default:
        if (ch >= 32 && ch < 127) {
          out << ch;
        } else {
          char hex[5];
          snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
          out << hex;
        }
        break;
    }
  }

  out << '"';
}

static bool serialize_recurse(Expr *n, std::ostream &out, ConvState &state) {
  if (!n) {
    qcore_panic("null node in serialization");
  }

#define recurse(x) serialize_recurse(x, out, state)

  switch (n->getKind()) {
    case QIR_NODE_BINEXPR: {
      out << '(';
      if (n->as<BinExpr>()->getOp() == Op::CastAs) {
        out << '(';
        recurse(n->as<BinExpr>()->getRHS());
        out << ")(";
        recurse(n->as<BinExpr>()->getLHS());
        out << ')';
      } else {
        recurse(n->as<BinExpr>()->getLHS());

        static const std::unordered_map<Op, std::string_view> binexpr_ops = {
            {Op::Plus, "+"},      {Op::Minus, "-"},    {Op::Times, "*"},   {Op::Slash, "/"},
            {Op::Percent, "%"},   {Op::BitAnd, "&"},   {Op::BitOr, "|"},   {Op::BitXor, "^"},
            {Op::LogicAnd, "&&"}, {Op::LogicOr, "||"}, {Op::LShift, "<<"}, {Op::RShift, ">>"},
            {Op::ROTR, "?"},      {Op::ROTL, "?"},     {Op::Set, "="},     {Op::LT, "<"},
            {Op::GT, ">"},        {Op::LE, "<="},      {Op::GE, ">="},     {Op::Eq, "=="},
            {Op::NE, "!="},
            // {Op::BitcastAs, "?"},
            // {Op::CastAs,    "?"},
        };

        auto it = binexpr_ops.find(n->as<BinExpr>()->getOp());
        if (it != binexpr_ops.end()) {
          out << it->second;
        } else {
          qcore_panicf("illegal binary expression operator: %d", (int)n->as<BinExpr>()->getOp());
        }

        recurse(n->as<BinExpr>()->getRHS());
      }
      out << ')';
      break;
    }

    case QIR_NODE_UNEXPR: {
      out << '(';
      static const std::unordered_map<Op, std::string_view> unexprs_ops = {
          {Op::Plus, "+"},   {Op::Minus, "-"},    {Op::Times, "*"}, {Op::BitAnd, "&"},
          {Op::BitNot, "~"}, {Op::LogicNot, "!"}, {Op::Inc, "++"},  {Op::Dec, "--"},
      };

      auto it = unexprs_ops.find(n->as<UnExpr>()->getOp());
      if (it != unexprs_ops.end()) {
        out << it->second;
      } else {
        qcore_panicf("illegal unary expression operator: %d", (int)n->as<UnExpr>()->getOp());
      }

      recurse(n->as<UnExpr>()->getExpr());
      out << ')';
      break;
    }

    case QIR_NODE_POST_UNEXPR: {
      out << '(';
      recurse(n->as<PostUnExpr>()->getExpr());

      static const std::unordered_map<Op, std::string_view> post_unexprs_ops = {
          {Op::Inc, "++"},
          {Op::Dec, "--"},
      };
      auto it = post_unexprs_ops.find(n->as<PostUnExpr>()->getOp());
      if (it != post_unexprs_ops.end()) {
        out << it->second;
      } else {
        qcore_panicf("illegal post unary expression operator: %d",
                     (int)n->as<PostUnExpr>()->getOp());
      }
      out << ')';
      break;
    }

    case QIR_NODE_INT: {
      out << n->as<Int>()->getValue();
      break;
    }

    case QIR_NODE_FLOAT: {
      out << n->as<Float>()->getValue();
      break;
    }

    case QIR_NODE_STRING: {
      escape_string(out, n->as<String>()->getValue());
      break;
    }

    case QIR_NODE_LIST: {
      out << '{';
      for (auto it = n->as<List>()->getItems().begin(); it != n->as<List>()->getItems().end();
           ++it) {
        recurse(*it);
        if (std::next(it) != n->as<List>()->getItems().end()) {
          out << ',';
        }
      }
      out << '}';
      break;
    }

    case QIR_NODE_CALL: {
      auto tkind = n->as<Call>()->getTarget()->getKind();
      if (tkind == QIR_NODE_LOCAL) {
        out << n->as<Call>()->getTarget()->as<Local>()->getName();
      } else if (tkind == QIR_NODE_FN) {
        out << n->as<Call>()->getTarget()->as<Fn>()->getName();
      } else {
        recurse(n->as<Call>()->getTarget());
      }
      out << '(';
      for (auto it = n->as<Call>()->getArgs().begin(); it != n->as<Call>()->getArgs().end(); ++it) {
        recurse(*it);
        if (std::next(it) != n->as<Call>()->getArgs().end()) {
          out << ',';
        }
      }
      out << ')';
      break;
    }

    case QIR_NODE_SEQ: {
      if (state.inside_func) {
        for (auto it = n->as<Seq>()->getItems().begin(); it != n->as<Seq>()->getItems().end();
             ++it) {
          if (it != n->as<Seq>()->getItems().begin()) {
            out << ',';
          }
          recurse(*it);
        }
      } else {
        for (auto &child : n->as<Seq>()->getItems()) {
          if (child->getKind() == QIR_NODE_VOID_TY) {
            continue;
          }

          recurse(child);

          if (child->getKind() != QIR_NODE_FN) {
            out << ';';
          }
        }
      }
      break;
    }

    case QIR_NODE_INDEX: {
      /// TODO:
      out << "/* TODO */";
      break;
    }

    case QIR_NODE_IDENT: {
      out << n->as<Ident>()->getName();
      break;
    }

    case QIR_NODE_EXTERN: {
      /// TODO:
      out << "/* TODO */";
      break;
    }

    case QIR_NODE_LOCAL: {
      auto T = static_cast<Expr *>(qxir_infer(n->as<Local>()->getValue()));
      recurse(T);
      out << ' ' << n->as<Local>()->getName();
      if (T->getKind() == QIR_NODE_ARRAY_TY) {
        out << '[' << T->as<ArrayTy>()->getCount() << "]";
      } else if (T->getKind() == QIR_NODE_LIST_TY) {
        out << "[]";
      }
      out << "=";
      recurse(n->as<Local>()->getValue());
      break;
    }

    case QIR_NODE_RET: {
      out << "return ";
      recurse(n->as<Ret>()->getExpr());
      break;
    }

    case QIR_NODE_BRK: {
      out << "break";
      break;
    }

    case QIR_NODE_CONT: {
      out << "continue";
      break;
    }

    case QIR_NODE_IF: {
      /// TODO:
      out << "/* TODO */";
      break;
    }

    case QIR_NODE_WHILE: {
      out << "while(";
      recurse(n->as<While>()->getCond());
      out << "){";
      for (auto &child : n->as<While>()->getBody()->getItems()) {
        if (child->getKind() == QIR_NODE_VOID_TY) {
          continue;
        }

        recurse(child);
        out << ';';
      }
      out << '}';
      break;
    }

    case QIR_NODE_FOR: {
      /// TODO:
      out << "/* TODO */";
      break;
    }

    case QIR_NODE_FORM: {
      /// TODO:
      out << "/* TODO */";
      break;
    }

    case QIR_NODE_FOREACH: {
      /// TODO:
      out << "/* TODO */";
      break;
    }

    case QIR_NODE_CASE: {
      out << "case ";
      recurse(n->as<Case>()->getCond());
      out << ":{";
      recurse(n->as<Case>()->getBody());
      out << ";}";
      break;
    }

    case QIR_NODE_SWITCH: {
      out << "switch(";
      recurse(n->as<Switch>()->getCond());
      out << "){";
      for (auto &child : n->as<Switch>()->getCases()) {
        recurse(child);
        out << ';';
      }
      out << "default:{";
      recurse(n->as<Switch>()->getDefault());
      out << ";}}";
      break;
    }

    case QIR_NODE_FN: {
      recurse(static_cast<Expr *>(qxir_infer(n->as<Fn>()->getBody())));
      out << ' ' << n->as<Fn>()->getName() << "(";
      for (size_t i = 0; i < n->as<Fn>()->getParams().size(); i++) {
        if (i != 0) {
          out << ",";
        }
        recurse(n->as<Fn>()->getParams()[i]);
        out << " __" << i;
      }
      out << "){";

      bool old = state.inside_func;
      state.inside_func = true;
      for (auto &child : n->as<Fn>()->getBody()->getItems()) {
        if (child->getKind() == QIR_NODE_VOID_TY) {
          continue;
        }

        recurse(child);
        out << ';';
      }
      state.inside_func = old;
      out << '}';
      break;
    }

    case QIR_NODE_ASM: {
      qcore_implement("Inline assembly");
      break;
    }

    case QIR_NODE_U1_TY: {
      out << "bool";
      break;
    }

    case QIR_NODE_U8_TY: {
      out << "uint8_t";
      break;
    }

    case QIR_NODE_U16_TY: {
      out << "uint16_t";
      break;
    }

    case QIR_NODE_U32_TY: {
      out << "uint32_t";
      break;
    }

    case QIR_NODE_U64_TY: {
      out << "uint64_t";
      break;
    }

    case QIR_NODE_U128_TY: {
      out << "quint128_t";
      break;
    }

    case QIR_NODE_I8_TY: {
      out << "int8_t";
      break;
    }

    case QIR_NODE_I16_TY: {
      out << "int16_t";
      break;
    }

    case QIR_NODE_I32_TY: {
      out << "int32_t";
      break;
    }

    case QIR_NODE_I64_TY: {
      out << "int64_t";
      break;
    }

    case QIR_NODE_I128_TY: {
      out << "qint128_t";
      break;
    }

    case QIR_NODE_F16_TY: {
      out << "qf16_t";
      break;
    }

    case QIR_NODE_F32_TY: {
      out << "qf32_t";
      break;
    }

    case QIR_NODE_F64_TY: {
      out << "qf64_t";
      break;
    }

    case QIR_NODE_F128_TY: {
      out << "qf128_t";
      break;
    }

    case QIR_NODE_VOID_TY: {
      out << "void";
      break;
    }

    case QIR_NODE_PTR_TY: {
      recurse(n->as<PtrTy>()->getPointee());
      out << '*';
      break;
    }

    case QIR_NODE_OPAQUE_TY: {
      out << n->as<OpaqueTy>()->getName();
      break;
    }

    case QIR_NODE_STRING_TY: {
      /// TODO:
      out << "const char*";
      break;
    }

    case QIR_NODE_STRUCT_TY: {
      out << "struct __S" << n->as<StructTy>()->getTypeIncrement();
      break;
    }

    case QIR_NODE_UNION_TY: {
      out << "union __U" << n->as<UnionTy>()->getTypeIncrement();
      break;
    }

    case QIR_NODE_ARRAY_TY: {
      recurse(n->as<ArrayTy>()->getElement());
      break;
    }

    case QIR_NODE_LIST_TY: {
      recurse(n->as<ListTy>()->getElement());
      break;
    }

    case QIR_NODE_FN_TY: {
      /// TODO:
      out << "/* TODO */";
      break;
    }

    case QIR_NODE_TMP: {
      qcore_panic("unexpected temporary node in serialization");
    }
  }
  return true;
}

bool codegen::for_c11(qmodule_t *module, std::ostream &err, std::ostream &out) {
  write_header(out);
  write_stdinc(out);
  write_coretypes(out);

  ConvState state;
  return serialize_recurse(module->getRoot(), out, state);
}
