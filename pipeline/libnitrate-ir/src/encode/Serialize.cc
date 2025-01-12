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
///   version 2.1 of the License, or (at your option) any later version-> ///
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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/encode/Serialize.hh>
#include <nitrate-lexer/Lexer.hh>
#include <variant>

using namespace ncc::ir;
using namespace ncc::ir::encode;

static const std::unordered_map<StorageClass, std::string_view>
    storage_class_repr = {
        {StorageClass::LLVM_StackAlloa, "auto"},
        {StorageClass::LLVM_Static, "static"},
        {StorageClass::LLVM_ThreadLocal, "thread"},
        {StorageClass::Managed, "managed"},
};

void IR_Writer::write_source_location(FlowPtr<Expr> n) const {
  string("loc");

  if (m_rd.has_value()) {
    lex::IScanner &rd = m_rd->get();

    begin_obj(3);

    auto begin = n->begin(rd);
    auto end = n->end(rd);

    {
      string("begin");
      begin_obj(4);

      string("off");
      uint64(begin.GetOffset());

      string("row");
      uint64(begin.GetRow());

      string("col");
      uint64(begin.GetCol());

      string("src");
      string(begin.GetFilename());

      end_obj();
    }

    {
      string("end");
      begin_obj(4);

      string("off");
      uint64(end.GetOffset());

      string("row");
      uint64(end.GetRow());

      string("col");
      uint64(end.GetCol());

      string("src");
      string(end.GetFilename());

      end_obj();
    }

    {
      string("trace");

#if NITRATE_FLOWPTR_TRACE
      begin_obj(4);

      let origin = n.trace();

      string("src");
      string(origin.file_name());

      string("sub");
      string(origin.function_name());

      string("row");
      uint64(origin.line());

      string("col");
      uint64(origin.column());

      end_obj();
#else
      null();
#endif
    }

    end_obj();
  } else {
    null();
  }
}

void IR_Writer::visit(FlowPtr<Expr> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<BinExpr> n) {
  begin_obj(5);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("op");
  string(op_repr(n->getOp()));

  string("lhs");
  n->getLHS().accept(*this);

  string("rhs");
  n->getRHS().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Unary> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("op");
  string(op_repr(n->getOp()));

  string("expr");
  n->getExpr().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<U1Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<U8Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<U16Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<U32Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<U64Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<U128Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<I8Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<I16Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<I32Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<I64Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<I128Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<F16Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<F32Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<F64Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<F128Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<VoidTy> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<PtrTy> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("pointee");
  n->getPointee().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<ConstTy> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("value");
  n->getItem().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<OpaqueTy> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("name");
  string(n->getName());

  end_obj();
}

void IR_Writer::visit(FlowPtr<StructTy> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  auto fields = n->getFields();

  string("fields");
  begin_arr(fields.size());

  std::for_each(fields.begin(), fields.end(),
                [&](auto &field) { field->accept(*this); });

  end_arr();

  end_obj();
}

void IR_Writer::visit(FlowPtr<UnionTy> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  auto fields = n->getFields();

  string("fields");
  begin_arr(fields.size());

  std::for_each(fields.begin(), fields.end(),
                [&](auto &field) { field->accept(*this); });

  end_arr();

  end_obj();
}

void IR_Writer::visit(FlowPtr<ArrayTy> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("element");
  n->getElement().accept(*this);

  string("size");
  uint64(n->getCount());

  end_obj();
}

void IR_Writer::visit(FlowPtr<FnTy> n) {
  begin_obj(5);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("params");

  auto params = n->getParams();
  begin_arr(params.size());

  std::for_each(params.begin(), params.end(),
                [&](auto &param) { param->accept(*this); });

  end_arr();

  string("variadic");
  boolean(n->isVariadic());

  string("return");
  n->getReturn().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Int> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("value");
  string(n->getValueString());

  string("type");
  uint64(n->getSize());

  end_obj();
}

void IR_Writer::visit(FlowPtr<Float> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("value");
  dbl(n->getValue());

  string("type");
  uint64(n->getSize());

  end_obj();
}

void IR_Writer::visit(FlowPtr<List> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("items");
  begin_arr(n->size());

  std::for_each(n->begin(), n->end(), [&](auto &item) { item->accept(*this); });

  end_arr();

  string("homegenous");
  boolean(n->isHomogenous());

  end_obj();
}

void IR_Writer::visit(FlowPtr<Call> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("callee");
  /// TODO: Handle possibly cyclic reference to callee
  null();

  string("arguments");

  auto args = n->getArgs();
  begin_arr(args.size());

  std::for_each(args.begin(), args.end(),
                [&](auto &arg) { arg->accept(*this); });

  end_arr();

  end_obj();
}

void IR_Writer::visit(FlowPtr<Seq> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("items");
  begin_arr(n->size());

  std::for_each(n->begin(), n->end(), [&](auto &item) { item->accept(*this); });

  end_arr();

  end_obj();
}

void IR_Writer::visit(FlowPtr<Index> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("base");
  n->getExpr().accept(*this);

  string("index");
  n->getIndex().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Ident> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("name");
  string(n->getName());

  end_obj();
}

void IR_Writer::visit(FlowPtr<Extern> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("value");
  n->getValue().accept(*this);

  string("abi_name");
  string(n->getAbiName());

  end_obj();
}

void IR_Writer::visit(FlowPtr<Local> n) {
  begin_obj(7);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("name");
  string(n->getName());

  string("abi_name");
  string(n->getAbiName());

  string("storage");
  string(storage_class_repr.at(n->getStorageClass()));

  string("readonly");
  boolean(n->isReadonly());

  string("value");
  n->getValue().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Ret> n) {
  begin_obj(3);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("expr");
  n->getExpr().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Brk> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Cont> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<If> n) {
  begin_obj(5);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("cond");
  n->getCond().accept(*this);

  string("then");
  n->getThen().accept(*this);

  string("else");
  n->getElse().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<While> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("cond");
  n->getCond().accept(*this);

  string("body");
  n->getBody().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<For> n) {
  begin_obj(6);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("init");
  n->getInit().accept(*this);

  string("cond");
  n->getCond().accept(*this);

  string("step");
  n->getStep().accept(*this);

  string("body");
  n->getBody().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Case> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("cond");
  n->getCond().accept(*this);

  string("body");
  n->getBody().accept(*this);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Switch> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("cond");
  n->getCond().accept(*this);

  string("default");
  n->getDefault().has_value() ? n->getDefault().value()->accept(*this) : null();

  string("cases");
  auto cases = n->getCases();
  begin_arr(cases.size());

  std::for_each(cases.begin(), cases.end(), [&](auto &c) { c->accept(*this); });

  end_arr();

  end_obj();
}

void IR_Writer::visit(FlowPtr<Function> n) {
  begin_obj(8);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("name");
  string(n->getName());

  string("abi_name");
  string(n->getAbiName());

  string("params");
  auto params = n->getParams();
  begin_arr(params.size());

  std::for_each(params.begin(), params.end(), [&](auto &param) {
    begin_obj(2);

    string("name");
    string(param.second);

    string("type");
    param.first->accept(*this);

    end_obj();
  });

  end_arr();

  string("variadic");
  boolean(n->isVariadic());

  string("return");
  n->getReturn().accept(*this);

  string("body");
  n->getBody().has_value() ? n->getBody().value()->accept(*this) : null();

  end_obj();
}

void IR_Writer::visit(FlowPtr<Asm> n) {
  begin_obj(2);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  end_obj();
}

void IR_Writer::visit(FlowPtr<Tmp> n) {
  begin_obj(4);

  string("kind");
  string(n->getKindName());

  write_source_location(n);

  string("kind");
  uint64((int)n->getTmpType());

  string("data");

  if (std::holds_alternative<IR_Vertex_CallArgsTmpNodeCradle<void>>(
          n->getData())) {
    auto data = std::get<IR_Vertex_CallArgsTmpNodeCradle<void>>(n->getData());

    begin_obj(2);

    string("base");
    data.base->accept(*this);

    string("args");
    auto args = data.args;
    begin_arr(args.size());

    std::for_each(args.begin(), args.end(), [&](auto &arg) {
      begin_obj(2);

      string("name");
      string(arg.first);

      string("value");
      arg.second->accept(*this);

      end_obj();
    });

    end_arr();

    end_obj();

  } else if (std::holds_alternative<ncc::string>(n->getData())) {
    string(std::get<ncc::string>(n->getData()));
  } else {
    qcore_panic("Unknown TmpNodeCradle type");
  }

  end_obj();
}
