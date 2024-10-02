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

#define __QUIX_IMPL__
#define QXIR_USE_CPP_API

#include <core/LibMacro.h>
#include <quix-core/Error.h>
#include <quix-qxir/Inference.h>
#include <quix-qxir/Node.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <core/Config.hh>
#include <cstdint>
#include <transcode/Targets.hh>

using namespace qxir;

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

struct PreGenParam {
  bool use_rotl = false;
  bool use_rotr = false;
  bool use_vector = false;
  bool use_array = false;
  bool use_bitcast = false;
  bool use_tuple = false;
  bool use_string = false;
  bool use_functional = false;
};

static PreGenParam pregen_iterate(Expr *root) {
  PreGenParam param;

  const auto cb = [&param](Expr *, Expr **p_cur) -> IterOp {
    Expr *cur = *p_cur;

    switch (cur->getKind()) {
      case QIR_NODE_BINEXPR: {
        auto op = cur->as<BinExpr>()->getOp();
        if (op == Op::ROTL) {
          param.use_rotl = true;
        } else if (op == Op::ROTR) {
          param.use_rotr = true;
        } else if (op == Op::BitcastAs) {
          param.use_bitcast = true;
        }
        break;
      }
      case QIR_NODE_STRUCT_TY:
        param.use_tuple = true;
        break;
      case QIR_NODE_STRING:
      case QIR_NODE_STRING_TY:
        param.use_string = true;
        break;
      case QIR_NODE_LIST:
        param.use_vector = true;
        break;
      case QIR_NODE_ARRAY_TY:
        param.use_array = true;
        break;
      case QIR_NODE_FN:
      case QIR_NODE_FN_TY:
        param.use_functional = true;
        break;
      default:
        break;
    }
    return IterOp::Proceed;
  };

  iterate<dfs_pre, IterMP::none>(root, cb);

  return param;
}

static void write_stdinc(std::ostream &out, const PreGenParam &param) {
  /// NOTE: [Standard] includes are not currently available in the clang setup.
  /// Either eliminate them as a dependency or more likely configure the clang
  /// frongend to properly include the standard headers. Ideally, they could
  /// be bundled as static data in the produced binary and referenced
  /// internally. To avoid external dependencies, this is the best approach.

  out << "#define QUIX_TRANSCODE 1\n";

  if (param.use_rotl || param.use_rotr) {
    out << "#define __qinline inline __attribute__((always_inline))\n";
  }
  out << "#define qexport __attribute__((visibility(\"default\")))\n";
  out << "\n";

  // if (param.use_array) {
  //   out << "#include <array>\n";
  // }
  // if (param.use_bitcast) {
  //   out << "#include <bit>\n";
  // }
  // out << "#include <cmath>\n";
  // out << "#include <cstdint>\n";

  // if (param.use_functional) {
  //   out << "#include <functional>\n";
  // }
  // if (param.use_string) {
  //   out << "#include <string>\n";
  // }
  // if (param.use_tuple) {
  //   out << "#include <tuple>\n";
  // }
  // if (param.use_vector) {
  //   out << "#include <vector>\n";
  // }

  out << "\n";
}

static void write_coretypes(std::ostream &out, const PreGenParam &param) {
  out << "typedef unsigned long long size_t;\n"; /* FIXME: Generate based on target triple */

  // out << "typedef __fp16 qf16_t;\n";
  // out << "typedef float qf32_t;\n";
  // out << "typedef double qf64_t;\n";
  // out << "typedef __float128 qf128_t;\n";

  { /* B*/
    out << R"()";
  }

  { /* qstring implementation */

    out << R"()";
  }

  out << "\n";
}

static void write_builtins(std::ostream &out, const PreGenParam &param) {
  static constexpr std::string_view builtin_rotl =
      R"(static __qinline uint64_t __rotlu64(uint64_t x, uint64_t r, uint8_t sz) {
  r %= sz;
  return (x << r) | (x >> (sz - r));
})";

  static constexpr std::string_view builtin_rotr =
      R"(static __qinline uint64_t __rotru64(uint64_t x, uint64_t r, uint8_t sz) {
  r %= sz;
  return (x >> r) | (x << (sz - r));
})";

  if (param.use_rotl) {
    out << builtin_rotl << "\n";
  }
  if (param.use_rotr) {
    out << builtin_rotr << "\n";
  }

  if (param.use_rotl || param.use_rotr) {
    out << "\n";
  }
}

struct ConvState {
  bool inside_func = false;
  bool stmt_mode = true;
  bool is_static = true;
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
      } else if (n->as<BinExpr>()->getOp() == Op::ROTL) {
        out << "__rotlu64(";
        recurse(n->as<BinExpr>()->getLHS());
        out << ",";
        recurse(n->as<BinExpr>()->getRHS());
        out << ',';
        out << static_cast<Type *>(qxir_infer(n->as<BinExpr>()->getLHS()))->getSizeBits();
        out << ")";
      } else if (n->as<BinExpr>()->getOp() == Op::ROTR) {
        out << "__rotru64(";
        recurse(n->as<BinExpr>()->getLHS());
        out << ",";
        recurse(n->as<BinExpr>()->getRHS());
        out << ',';
        out << static_cast<Type *>(qxir_infer(n->as<BinExpr>()->getLHS()))->getSizeBits();
        out << ")";
      } else if (n->as<BinExpr>()->getOp() == Op::BitcastAs) {
        out << "std::bit_cast<";
        recurse(n->as<BinExpr>()->getRHS());
        out << ">(";
        recurse(n->as<BinExpr>()->getLHS());
        out << ")";
      } else {
        recurse(n->as<BinExpr>()->getLHS());

        static const std::unordered_map<Op, std::string_view> binexpr_ops = {
            {Op::Plus, "+"},      {Op::Minus, "-"},    {Op::Times, "*"},   {Op::Slash, "/"},
            {Op::Percent, "%"},   {Op::BitAnd, "&"},   {Op::BitOr, "|"},   {Op::BitXor, "^"},
            {Op::LogicAnd, "&&"}, {Op::LogicOr, "||"}, {Op::LShift, "<<"}, {Op::RShift, ">>"},
            {Op::Set, "="},       {Op::LT, "<"},       {Op::GT, ">"},      {Op::LE, "<="},
            {Op::GE, ">="},       {Op::Eq, "=="},      {Op::NE, "!="},
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
      errno = 0;
      char *end;
      uint64_t x = std::strtoul(n->as<Int>()->getValue().c_str(), &end, 10);
      if (errno == 0) {
        out << x;
      } else {
        boost::multiprecision::cpp_int y(n->as<Int>()->getValue());
        uint64_t hi = (y >> 64).convert_to<uint64_t>();
        uint64_t lo = (y & 0xFFFFFFFFFFFFFFFF).convert_to<uint64_t>();
        out << "quint128_t(" << hi << "ULL," << lo << "ULL)";
      }
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
      if (state.stmt_mode) {
        for (auto it = n->as<Seq>()->getItems().begin(); it != n->as<Seq>()->getItems().end();
             ++it) {
          if ((*it)->getKind() == QIR_NODE_VOID_TY) {
            continue;
          }
          recurse(*it);
          out << ';';
        }
      } else {
        for (auto it = n->as<Seq>()->getItems().begin(); it != n->as<Seq>()->getItems().end();
             ++it) {
          if ((*it)->getKind() == QIR_NODE_VOID_TY) {
            continue;
          }
          if (it != n->as<Seq>()->getItems().begin()) {
            out << ',';
          }
          recurse(*it);
        }
      }
      break;
    }

    case QIR_NODE_INDEX: {
      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = false;

      recurse(n->as<Index>()->getExpr());
      auto tp = n->as<Index>()->getIndex()->getKind();

      if (tp == QIR_NODE_STRING) {
        out << '.';
        out << n->as<Index>()->getIndex()->as<String>()->getValue();
      } else if (tp == QIR_NODE_INT) {
        out << '[';
        recurse(n->as<Index>()->getIndex());
        out << ']';
      } else {
        qcore_panicf("unexpected index type in serialization: %d", (int)tp);
      }

      state.stmt_mode = old_stmt_mode;
      break;
    }

    case QIR_NODE_IDENT: {
      out << n->as<Ident>()->getName();
      break;
    }

    case QIR_NODE_EXTERN: {
      bool old_is_static = state.is_static;
      state.is_static = false;

      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = true;

      auto val = n->as<Extern>()->getValue();
      auto ty = val->getKind();

      if (ty == QIR_NODE_FN) {
        out << "qexport ";
        recurse(val);
      } else if (ty == QIR_NODE_LOCAL) {
        if (val->as<Local>()->getValue()->getKind() == QIR_NODE_FN_TY) {
          out << "extern ";
          if (n->as<Extern>()->getAbiName() == "c") {
            out << "\"C\" ";
          }
          auto fty = val->as<Local>()->getValue()->as<FnTy>();
          recurse(fty->getReturn());

          out << ' ' << val->as<Local>()->getName() << '(';
          for (size_t i = 0; i < fty->getParams().size(); i++) {
            if (i != 0) {
              out << ',';
            }
            recurse(fty->getParams()[i]);
            out << " _P" << i;
          }
          if (fty->getAttrs().contains(FnAttr::Variadic)) {
            if (fty->getParams().size() != 0) {
              out << ',';
            }

            out << "...";
          }

          out << ')';
        } else {
          out << "qexport ";
          recurse(val);
        }
      }

      state.stmt_mode = old_stmt_mode;
      state.is_static = old_is_static;
      break;
    }

    case QIR_NODE_LOCAL: {
      if (state.is_static) {
        out << "static ";
      }

      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = false;
      auto T = static_cast<Type *>(qxir_infer(n->as<Local>()->getValue()));
      state.stmt_mode = old_stmt_mode;

      recurse(T);
      out << ' ' << n->as<Local>()->getName();
      if (!n->as<Local>()->getValue()->isType()) {
        out << "=";
        recurse(n->as<Local>()->getValue());
      }
      break;
    }

    case QIR_NODE_RET: {
      out << "return";
      if (n->as<Ret>()->getExpr()->getKind() != QIR_NODE_VOID_TY) {
        out << ' ';
        bool old_stmt_mode = state.stmt_mode;
        state.stmt_mode = false;
        recurse(n->as<Ret>()->getExpr());
        state.stmt_mode = old_stmt_mode;
      }
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
      out << "if(";

      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = false;
      recurse(n->as<If>()->getCond());
      state.stmt_mode = true;
      out << "){";
      recurse(n->as<If>()->getThen());
      out << ";}";
      if (n->as<If>()->getElse()->getKind() != QIR_NODE_VOID_TY) {
        out << "else{";
        recurse(n->as<If>()->getElse());
        out << ";}";
      }
      state.stmt_mode = old_stmt_mode;
      break;
    }

    case QIR_NODE_WHILE: {
      out << "while(";
      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = false;
      recurse(n->as<While>()->getCond());
      out << "){";
      state.stmt_mode = true;
      recurse(n->as<While>()->getBody());
      out << ";}";
      state.stmt_mode = old_stmt_mode;
      break;
    }

    case QIR_NODE_FOR: {
      out << "for(";
      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = false;
      recurse(n->as<For>()->getInit());
      out << ';';
      recurse(n->as<For>()->getCond());
      out << ';';
      recurse(n->as<For>()->getStep());
      out << "){";
      state.stmt_mode = true;
      recurse(n->as<For>()->getBody());
      out << "}";
      state.stmt_mode = old_stmt_mode;
      break;
    }

    case QIR_NODE_FORM: {
      /// TODO:
      out << "/* TODO */";

      /**
       * 1. Create thread pool / use library
       * 2. Push to thread pool somehow? Lambdas not supported in C11
       * 3. Join threads at end of For Multi Loop
       */

      break;
    }

    case QIR_NODE_CASE: {
      out << "case ";
      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = false;
      recurse(n->as<Case>()->getCond());
      out << ":{";
      state.stmt_mode = true;
      recurse(n->as<Case>()->getBody());
      out << ";}";
      state.stmt_mode = old_stmt_mode;
      break;
    }

    case QIR_NODE_SWITCH: {
      out << "switch(";
      bool old_stmt_mode = state.stmt_mode;
      state.stmt_mode = false;
      recurse(n->as<Switch>()->getCond());
      state.stmt_mode = true;
      out << "){";
      for (auto &child : n->as<Switch>()->getCases()) {
        recurse(child);
        out << ';';
      }
      if (n->as<Switch>()->getDefault()->getKind() != QIR_NODE_VOID_TY) {
        out << "default:{";
        recurse(n->as<Switch>()->getDefault());
        out << ";}";
      }
      out << '}';
      state.stmt_mode = old_stmt_mode;
      break;
    }

    case QIR_NODE_FN: {
      if (state.stmt_mode) {
        recurse(static_cast<Expr *>(n->as<Fn>()->getReturn()));
        out << ' ' << n->as<Fn>()->getName() << "(";
        for (size_t i = 0; i < n->as<Fn>()->getParams().size(); i++) {
          if (i != 0) {
            out << ",";
          }
          recurse(n->as<Fn>()->getParams()[i].first);
          out << ' ' << n->as<Fn>()->getParams()[i].second;
        }
        out << "){";
        bool old = state.inside_func;
        state.inside_func = true;
        recurse(n->as<Fn>()->getBody());
        state.inside_func = old;
        out << '}';
      } else {
        out << "[](";
        for (size_t i = 0; i < n->as<Fn>()->getParams().size(); i++) {
          if (i != 0) {
            out << ",";
          }
          recurse(n->as<Fn>()->getParams()[i].first);
          out << ' ' << n->as<Fn>()->getParams()[i].second;
        }
        out << ")->";
        recurse(n->as<Fn>()->getReturn());
        out << '{';

        bool old = state.inside_func;
        state.inside_func = true;
        recurse(n->as<Fn>()->getBody());
        state.inside_func = old;
        out << '}';
      }

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
      out << "std::string";
      break;
    }

    case QIR_NODE_STRUCT_TY: {
      out << "std::tuple<";
      for (size_t i = 0; i < n->as<StructTy>()->getFields().size(); i++) {
        if (i != 0) {
          out << ',';
        }
        recurse(n->as<StructTy>()->getFields()[i]);
      }
      out << '>';
      break;
    }

    case QIR_NODE_UNION_TY: {
      out << "union _T" << n->as<UnionTy>()->getTypeIncrement();
      break;
    }

    case QIR_NODE_ARRAY_TY: {
      out << "std::array<";
      recurse(n->as<ArrayTy>()->getElement());
      out << ',';
      recurse(n->as<ArrayTy>()->getCount());
      out << '>';
      break;
    }

    case QIR_NODE_FN_TY: {
      out << "std::function<";
      recurse(n->as<FnTy>()->getReturn());
      out << '(';
      for (size_t i = 0; i < n->as<FnTy>()->getParams().size(); i++) {
        if (i != 0) {
          out << ',';
        }
        recurse(n->as<FnTy>()->getParams()[i]);
      }
      if (n->as<FnTy>()->getAttrs().contains(FnAttr::Variadic)) {
        if (n->as<FnTy>()->getParams().size() != 0) {
          out << ',';
        }

        out << "...";
      }
      out << ")>";
      break;
    }

    case QIR_NODE_TMP: {
      qcore_panic("unexpected temporary node in serialization");
    }
  }
  return true;
}

bool codegen::for_cxx11(qmodule_t *module, std::ostream &err, std::ostream &out) {
  write_header(out);

  PreGenParam param = pregen_iterate(module->getRoot());
  write_stdinc(out, param);
  write_coretypes(out, param);
  write_builtins(out, param);

  ConvState state;
  return serialize_recurse(module->getRoot(), out, state);
}
