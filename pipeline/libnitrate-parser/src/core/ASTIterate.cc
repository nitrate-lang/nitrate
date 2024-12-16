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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-lexer/Lexer.h>

#include <algorithm>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>

using namespace npar;

CPP_EXPORT void detail::dfs_pre_impl(Expr **base, IterCallback cb,
                                     ChildSelect cs) {
  /// TODO: Implement iterator
  qcore_implement();
}

CPP_EXPORT void detail::dfs_post_impl(Expr **base, IterCallback cb,
                                      ChildSelect cs) {
  /// TODO: Implement iterator
  qcore_implement();
}

CPP_EXPORT void detail::bfs_pre_impl(Expr **base, IterCallback cb,
                                     ChildSelect cs) {
  /// TODO: Implement iterator
  qcore_implement();
}

CPP_EXPORT void detail::bfs_post_impl(Expr **base, IterCallback cb,
                                      ChildSelect cs) {
  /// TODO: Implement iterator
  qcore_implement();
}

CPP_EXPORT void detail::iter_children(Expr **base, IterCallback cb,
                                      ChildSelect cs) {
  /// TODO: Implement iterator
  qcore_implement();
}

CPP_EXPORT void detail::inorder_impl(Expr **base, IterCallback cb,
                                     ChildSelect cs) {
  /// TODO: Implement iterator
  qcore_implement();
}
