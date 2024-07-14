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

#ifndef __QUIXCC_IR_DELTA_NODES_SEGMENT_H__
#define __QUIXCC_IR_DELTA_NODES_SEGMENT_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/IR/delta/DeltaIR.h>
#include <quixcc/IR/delta/Expr.h>
#include <quixcc/IR/delta/Type.h>

namespace libquixcc::ir::delta {
  class Block : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Block(std::vector<const Value *> stmts) : stmts(stmts) { ntype = (int)NodeType::Block; }

public:
    static const Block *create(std::vector<const Value *> stmts);

    std::vector<const Value *> stmts;
  };

  class Segment : public Expr {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Segment(const Type *ret, bool variadic,
            const std::vector<std::pair<std::string, const Type *>> &params, const Block *block)
        : params(params), block(block), ret(ret), variadic(variadic) {
      ntype = (int)NodeType::Segment;
    }

public:
    static const Segment *create(const Type *ret, bool variadic,
                                 const std::vector<std::pair<std::string, const Type *>> &params,
                                 const Block *block);
    const Type *infer() const override;

    const FType *getType() const {
      std::vector<const Type *> param_types;
      for (auto &param : params)
        param_types.push_back(param.second);
      return FType::create(param_types, ret, variadic);
    }

    std::vector<std::pair<std::string, const Type *>> params;
    const Block *block;
    const Type *ret;
    bool variadic;
  };

  class Asm : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Asm(const std::string &asm_str,
        const std::vector<std::pair<std::string, const Value *>> &inputs,
        const std::vector<std::pair<std::string, const Value *>> &outputs,
        const std::vector<std::string> &clobbers)
        : asm_str(asm_str), inputs(inputs), outputs(outputs), clobbers(clobbers) {
      ntype = (int)NodeType::Asm;
    }

public:
    static const Asm *create(const std::string &asm_str,
                             const std::vector<std::pair<std::string, const Value *>> &inputs,
                             const std::vector<std::pair<std::string, const Value *>> &outputs,
                             const std::vector<std::string> &clobbers);

    std::string asm_str;
    std::vector<std::pair<std::string, const Value *>> inputs;
    std::vector<std::pair<std::string, const Value *>> outputs;
    std::vector<std::string> clobbers;
  };

} // namespace libquixcc::ir::delta

#endif // __QUIXCC_IR_DELTA_NODES_SEGMENT_H__