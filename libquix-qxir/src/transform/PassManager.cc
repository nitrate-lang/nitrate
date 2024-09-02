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
#include <transform/PassManager.hh>
#include <transform/passes/Decl.hh>

using namespace qxir::passes;

static void seed_passes() {
  Pass::register_pass("ds-acyclic", impl::ds_acyclic);
  Pass::register_pass("ds-nilchk", impl::ds_nilchk);
  Pass::register_pass("ds-chtype", impl::ds_chtype);
  Pass::register_pass("ds-resolv", impl::ds_resolv);
  Pass::register_pass("ns-flatten", impl::ns_flatten);
  Pass::register_pass("fnflatten", impl::fnflatten);
  Pass::register_pass("tyinfer", impl::tyinfer);
  Pass::register_pass("nm-premangle", impl::nm_premangle);

  /* Read-only passes */
  PassGroup::register_group("g0", {"ds-acyclic", "ds-nilchk"}, {});
  PassGroup::register_group("g1", {"ds-chtype"}, {{"g0", DependencyFrequency::Always}});
  PassGroup::register_group(
      "g2", {"ds-resolv"},
      {{"g0", DependencyFrequency::Always}, {"g1", DependencyFrequency::Once}});

  /* Transformative passes */
  PassGroup::register_group("g3", {"ns-flatten"},
                            {{"g0", DependencyFrequency::Always},
                             {"g1", DependencyFrequency::Once},
                             {"g2", DependencyFrequency::Once}});
  PassGroup::register_group("g4", {"fnflatten"},
                            {{"g0", DependencyFrequency::Always},
                             {"g1", DependencyFrequency::Once},
                             {"g2", DependencyFrequency::Once},
                             {"g3", DependencyFrequency::Once}});
  PassGroup::register_group("g5", {"tyinfer"},
                            {{"g0", DependencyFrequency::Always},
                             {"g1", DependencyFrequency::Once},
                             {"g2", DependencyFrequency::Once},
                             {"g3", DependencyFrequency::Once},
                             {"g4", DependencyFrequency::Once}});
  PassGroup::register_group("g6", {"nm-premangle"},
                            {{"g0", DependencyFrequency::Always},
                             {"g1", DependencyFrequency::Once},
                             {"g2", DependencyFrequency::Once},
                             {"g3", DependencyFrequency::Once},
                             {"g4", DependencyFrequency::Once},
                             {"g5", DependencyFrequency::Once}});

  /* Root pass group */
  PassGroup::register_group("root", {},
                            {{"g1", DependencyFrequency::Once},
                             {"g2", DependencyFrequency::Once},
                             {"g3", DependencyFrequency::Once},
                             {"g4", DependencyFrequency::Once},
                             {"g5", DependencyFrequency::Once},
                             {"g6", DependencyFrequency::Once}});
}

const PassGroup* StdTransform::optimize_order(std::vector<PassName> passes) {
  /// TODO: Implement pass ordering

  return PassGroup::get("root");

  qcore_implement("StdTransform::optimize_order");
}

const StdTransform* StdTransform::create() {
  static std::once_flag flag;
  std::call_once(flag, seed_passes);

  thread_local StdTransform ptr;

  ptr.m_root = optimize_order({
      "ds-chtype",    /* Data structure child type */
      "ds-resolv",    /* Data structure discovery	*/
      "ns-flatten",   /* Namespace flattening	*/
      "fnflatten",    /* Function flattening */
      "tyinfer",      /* Type inference	*/
      "nm-premangle", /* Name Pre-mangling */
  });

  return &ptr;
}

bool StdTransform::transform(qmodule_t* module, std::ostream& out) const {
  auto res = m_root->transform(module);
  res.print(out);

  return !!res;
}
