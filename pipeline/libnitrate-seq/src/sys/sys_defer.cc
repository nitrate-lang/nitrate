// ////////////////////////////////////////////////////////////////////////////////
// /// ///
// ///     .-----------------.    .----------------.     .----------------. ///
// ///    | .--------------. |   | .--------------. |   | .--------------. | ///
// ///    | | ____  _____  | |   | |     ____     | |   | |    ______    | | ///
// ///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | | ///
// ///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | | ///
// ///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | | ///
// ///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | | ///
// ///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | | ///
// ///    | |              | |   | |              | |   | |              | | ///
// ///    | '--------------' |   | '--------------' |   | '--------------' | ///
// ///     '----------------'     '----------------'     '----------------' ///
// /// ///
// ///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language.
// ///
// ///   * Copyright (C) 2024 Wesley C. Jones ///
// /// ///
// ///   The Nitrate Toolchain is free software; you can redistribute it or ///
// ///   modify it under the terms of the GNU Lesser General Public ///
// ///   License as published by the Free Software Foundation; either ///
// ///   version 2.1 of the License, or (at your option) any later version. ///
// /// ///
// ///   The Nitrate Toolcain is distributed in the hope that it will be ///
// ///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// ///
// ///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU ///
// ///   Lesser General Public License for more details. ///
// /// ///
// ///   You should have received a copy of the GNU Lesser General Public ///
// ///   License along with the Nitrate Toolchain; if not, see ///
// ///   <https://www.gnu.org/licenses/>. ///
// /// ///
// ////////////////////////////////////////////////////////////////////////////////

// #include <nitrate-seq/Sequencer.hh>

// extern "C" {
// #include <lua/lauxlib.h>
// }

// using namespace ncc::seq;
// using namespace ncc::lex;

// auto Sequencer::SysDefer() -> int {
//   auto* lua = m_shared->m_L;

//   int nargs = lua_gettop(lua);
//   if (nargs < 1) {
//     return luaL_error(lua, "sys_defer: expected at least 1 argument, got %d",
//                       nargs);
//   }

//   if (!lua_isfunction(lua, 1)) {
//     return luaL_error(lua,
//                       "sys_defer: expected function as first argument, got
//                       %s", luaL_typename(lua, 1));
//   }

//   int id = luaL_ref(lua, LUA_REGISTRYINDEX);
//   if (id == LUA_REFNIL) {
//     return luaL_error(lua, "sys_defer: failed to store callback in
//     registry");
//   }

//   DeferCallback cb = [&](Sequencer* engine, Token tok) -> DeferOp {
//     lua_rawgeti(lua, LUA_REGISTRYINDEX, id); /* Get the function */

//     { /* Push the function arguments */
//       lua_newtable(lua);

//       lua_pushstring(lua, "ty");
//       lua_pushstring(lua, to_string(tok.GetKind())->c_str());
//       lua_settable(lua, -3);

//       lua_pushstring(lua, "v");
//       switch (tok.GetKind()) {
//         case EofF:
//         case KeyW: {
//           lua_pushstring(lua, kw_repr(tok.GetKeyword()));
//           break;
//         }
//         case Oper: {
//           lua_pushstring(lua, op_repr(tok.GetOperator()));
//           break;
//         }
//         case Punc: {
//           lua_pushstring(lua, punct_repr(tok.GetPunctor()));
//           break;
//         }
//         case IntL:
//         case NumL:
//         case Text:
//         case Char:
//         case Name:
//         case MacB:
//         case Macr:
//         case Note: {
//           lua_pushstring(lua, std::string(tok.GetString()).c_str());
//           break;
//         }
//       }

//       lua_settable(lua, -3);
//     }

//     int err = lua_pcall(lua, 1, 1, 0);
//     DeferOp r;

//     switch (err) {
//       case LUA_OK: {
//         if (lua_isnil(lua, -1)) {
//           return UninstallHandler;
//         }

//         if (!lua_isboolean(lua, -1)) {
//           ncc::Log << Error
//                    << "sys_defer: expected boolean return value or nil, got "
//                    << luaL_typename(lua, -1);

//           engine->SetFailBit();
//           return EmitToken;
//         }

//         r = (lua_toboolean(lua, -1) != 0) ? EmitToken : SkipToken;
//         break;
//       }
//       case LUA_ERRRUN: {
//         ncc::Log << Error << "sys_defer: lua: " << lua_tostring(lua, -1);
//         engine->SetFailBit();
//         r = EmitToken;
//         break;
//       }
//       case LUA_ERRMEM: {
//         ncc::Log << Error << "sys_defer: memory allocation error\n";
//         engine->SetFailBit();
//         r = EmitToken;
//         break;
//       }
//       case LUA_ERRERR: {
//         ncc::Log << Error << "sys_defer: error in error handler\n";
//         engine->SetFailBit();
//         r = EmitToken;
//         break;
//       }
//       default: {
//         ncc::Log << Error << "sys_defer: unexpected error " << err << "\n";
//         engine->SetFailBit();
//         r = EmitToken;
//         break;
//       }
//     }

//     lua_pop(lua, 1);

//     return r;
//   };

//   m_shared->m_defer.push_back(cb);

//   return 0;
// }
