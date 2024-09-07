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

#include <quix-core/Error.h>
#include <quix-lexer/Token.h>

#include <quix-lexer/Base.hh>
#include <new>

#include "LibMacro.h"

// class Engine final {
//   qlex_t *m_lexer;

// public:
//   Engine(FILE *file, const char *filename) {
//     m_lexer = qlex_new(file, filename);
//     if (!m_lexer) {
//       qcore_panic("Engine: failed to create lexer");
//     }
//   }

//   ~Engine() { qlex_free(m_lexer); }

//   qlex_t *inner() { return m_lexer; }
// };

// #define ctx(_ptr) reinterpret_cast<Engine *>(_ptr->inner)

// static qlex_tok_t _impl_peek(qlex_t *self) {
//   /// TODO:

//   // return qlex_peek(ctx(self)->inner());

//   return {};
// }

// static void _impl_push(qlex_t *self, const qlex_tok_t *tok) {
//   /// TODO:

//   qlex_push(ctx(self)->inner(), *tok);
// }

// static void _impl_collect(qlex_t *self, const qlex_tok_t *tok) {
//   /// TODO:

//   qlex_collect(ctx(self)->inner(), tok);
// }

// static void _impl_destruct(qlex_t *self) {
//   /// TODO:

//   delete ctx(self);
//   self->inner = nullptr;
// }

LIB_EXPORT qlex_t *qprep_new(FILE *file, const char *filename) {
  try {
    qcore_assert(file, "qprep_new: file is NULL");

    if (!filename) {
      filename = "<unknown>";
    }

    // return new qlex_t{
    //     .peek = _impl_peek,
    //     .push = _impl_push,
    //     .collect = _impl_collect,
    //     .destruct = _impl_destruct,
    //     .cur = {.ty = qErro},
    //     .flags = QLEX_FLAG_NONE,
    //     .filename = filename,
    //     .inner = new Engine(file, filename),
    // };
    return qlex_new(file, filename);
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qprep_new: failed to create lexer");
  }
}
