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

#ifndef __QUIXCC_IR_Q_NODES_OOP_H__
#define __QUIXCC_IR_Q_NODES_OOP_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/Function.h>
#include <IR/Q/QIR.h>

namespace libquixcc::ir::q {
class RegionDef : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  RegionDef(std::string name,
            std::vector<std::pair<std::string, Value *>> fields,
            std::map<std::string, Segment *> methods)
      : name(name), fields(fields), methods(methods) {
    ntype = (int)QType::RegionDef;
  }

 public:
  static RegionDef *create(std::string name,
                           std::vector<std::pair<std::string, Value *>> fields,
                           std::map<std::string, Segment *> methods);

  std::string name;
  std::vector<std::pair<std::string, Value *>> fields;
  std::map<std::string, Segment *> methods;
};

class GroupDef : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  GroupDef(std::string name, std::map<std::string, Value *> fields,
           std::map<std::string, Segment *> methods)
      : name(name), fields(fields), methods(methods) {
    ntype = (int)QType::GroupDef;
  }

 public:
  static GroupDef *create(std::string name,
                          std::map<std::string, Value *> fields,
                          std::map<std::string, Segment *> methods);

  std::string name;
  std::map<std::string, Value *> fields;
  std::map<std::string, Segment *> methods;
};

class UnionDef : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  UnionDef(std::string name, std::map<std::string, Value *> fields,
           std::map<std::string, Segment *> methods)
      : name(name), fields(fields), methods(methods) {
    ntype = (int)QType::UnionDef;
  }

 public:
  static UnionDef *create(std::string name,
                          std::map<std::string, Value *> fields,
                          std::map<std::string, Segment *> methods);

  std::string name;
  std::map<std::string, Value *> fields;
  std::map<std::string, Segment *> methods;
};
}  // namespace libquixcc::ir::q

#endif  // __QUIXCC_IR_Q_NODES_OOP_H__