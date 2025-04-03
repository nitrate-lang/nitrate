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

#include <core/PImpl.hh>
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-seq/Sequencer.hh>

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc::lex;
using namespace ncc::seq;

void PushTokenObject(lua_State* lua, Token tok) {
  lua_newtable(lua);

  lua_pushstring(lua, "ty");
  lua_pushstring(lua, to_string(tok.GetKind())->c_str());
  lua_settable(lua, -3);

  lua_pushstring(lua, "v");
  switch (tok.GetKind()) {
    case EofF: {
      lua_pushstring(lua, "");
      break;
    }

    case KeyW: {
      lua_pushstring(lua, ncc::lex::LEXICAL_KEYWORDS.right.at(tok.GetKeyword()).data());
      break;
    }

    case Oper: {
      lua_pushstring(lua, ncc::lex::LEXICAL_OPERATORS.right.at(tok.GetOperator()).data());
      break;
    }

    case Punc: {
      lua_pushstring(lua, punct_repr(tok.GetPunctor()));
      break;
    }

    case IntL:
    case NumL:
    case Text:
    case Char:
    case Name:
    case MacB:
    case Macr:
    case Note: {
      lua_pushstring(lua, tok.GetString()->c_str());
      break;
    }
  }

  lua_settable(lua, -3);
}

auto Sequencer::SysNext() -> int32_t {
  PushTokenObject(m_shared->m_L, Next());

  return 1;
}
