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

#define __QXIR_IMPL__
#include <quix-core/Error.h>
#include <quix-qxir/Module.h>

#include <mutex>
#include <passes/PassManager.hh>

using namespace qxir::passes;

static void seed_passes() {
  PassFunc placeholder = [](qxir::Module& module) -> PassResult {
    /// TODO: Write passes
    qcore_implement("placeholder");
  };

  Pass::create("ds-acyclic", placeholder);
  Pass::create("ds-nilchk", placeholder);
  Pass::create("ds-chtype", placeholder);
  Pass::create("ds-discov", placeholder);
  Pass::create("ns-flatten", placeholder);
  Pass::create("fnflatten", placeholder);
  Pass::create("tyinfer", placeholder);
  Pass::create("nm-premangle", placeholder);

  /* Read-only passes */
  PassGroup::create("g0", {"ds-acyclic", "ds-nilchk"}, {});
  PassGroup::create("g1", {"ds-chtype"}, {{"g0", DependencyFrequency::Once}});
  PassGroup::create("g2", {"ds-discov"},
                    {{"g0", DependencyFrequency::Once}, {"g1", DependencyFrequency::Once}});

  /* Transformative passes */
  PassGroup::create("g3", {"ns-flatten"},
                    {{"g0", DependencyFrequency::Once},
                     {"g1", DependencyFrequency::Once},
                     {"g2", DependencyFrequency::Once}});
  PassGroup::create("g4", {"fnflatten"},
                    {{"g0", DependencyFrequency::Once},
                     {"g1", DependencyFrequency::Once},
                     {"g2", DependencyFrequency::Once},
                     {"g3", DependencyFrequency::Once}});
  PassGroup::create("g5", {"tyinfer"},
                    {{"g0", DependencyFrequency::Once},
                     {"g1", DependencyFrequency::Once},
                     {"g2", DependencyFrequency::Once},
                     {"g3", DependencyFrequency::Once},
                     {"g4", DependencyFrequency::Once}});
  PassGroup::create("g6", {"nm-premangle"},
                    {{"g0", DependencyFrequency::Once},
                     {"g1", DependencyFrequency::Once},
                     {"g2", DependencyFrequency::Once},
                     {"g3", DependencyFrequency::Once},
                     {"g4", DependencyFrequency::Once},
                     {"g5", DependencyFrequency::Once}});

  /* Root pass group */
  PassGroup::create("root", {},
                    {{"g0", DependencyFrequency::Once},
                     {"g1", DependencyFrequency::Once},
                     {"g2", DependencyFrequency::Once},
                     {"g3", DependencyFrequency::Once},
                     {"g4", DependencyFrequency::Once},
                     {"g5", DependencyFrequency::Once},
                     {"g6", DependencyFrequency::Once}});
}

std::weak_ptr<PassGroup> StdTransform::optimize_order(std::vector<PassName> passes) {
  /// TODO:

  return PassGroup::get("root");

  qcore_implement("StdTransform::optimize_order");
}

std::unique_ptr<StdTransform> StdTransform::create() {
  static std::once_flag flag;
  std::call_once(flag, seed_passes);

  std::unique_ptr<StdTransform> ptr = std::make_unique<StdTransform>();

  ptr->m_root = optimize_order({
      "ds-acyclic",   /* Data structure acyclic verification */
      "ds-nilchk",    /* Data structure null pointer verification	*/
      "ds-chtype",    /* Data structure child type */
      "ds-discov",    /* Data structure discovery	*/
      "ns-flatten",   /* Namespace flattening	*/
      "fnflatten",    /* Function flattening */
      "tyinfer",      /* Type inference	*/
      "nm-premangle", /* Name Pre-mangling */
  });

  return ptr;
}

bool StdTransform::transform(qxir::Module& module, std::ostream& out) {
  auto ref = m_root.lock();
  if (!ref) {
    return false;
  }

  auto res = ref->transform(module);
  res.print(out);

  return !!res;
}
