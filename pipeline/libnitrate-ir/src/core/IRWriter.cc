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
#include <nitrate-ir/IRWriter.hh>
#include <nitrate-lexer/Scanner.hh>
#include <variant>

using namespace ncc::ir;
using namespace ncc::ir;

static const std::unordered_map<StorageClass, std::string_view>
    STORAGE_CLASS_REPR = {
        {StorageClass::LLVM_StackAlloa, "auto"},
        {StorageClass::LLVM_Static, "static"},
        {StorageClass::LLVM_ThreadLocal, "thread"},
        {StorageClass::Managed, "managed"},
};

void IRWriter::WriteSourceLocation(FlowPtr<Expr> n) const {
  string("loc");

  if (m_rd.has_value()) {
    lex::IScanner &rd = m_rd->get();

    begin_obj(3);

    auto begin = n->Begin(rd);
    auto end = n->End(rd);

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

      let origin = n.Trace();

      string("src");
      string(origin.File());

      string("sub");
      string(origin.Function());

      string("row");
      uint64(origin.Line());

      string("col");
      uint64(origin.Column());

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

void IRWriter::Visit(FlowPtr<Expr> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Binary> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("op");
  string(op_repr(n->GetOp()));

  string("lhs");
  n->GetLHS().Accept(*this);

  string("rhs");
  n->GetRHS().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Unary> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("op");
  string(op_repr(n->GetOp()));

  string("expr");
  n->GetExpr().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<U1Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<U8Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<U16Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<U32Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<U64Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<U128Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<I8Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<I16Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<I32Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<I64Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<I128Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<F16Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<F32Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<F64Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<F128Ty> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<VoidTy> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<PtrTy> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("pointee");
  n->GetPointee().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<ConstTy> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  n->GetItem().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<OpaqueTy> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  end_obj();
}

void IRWriter::Visit(FlowPtr<StructTy> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  auto fields = n->GetFields();

  string("fields");
  begin_arr(fields.size());

  std::for_each(fields.begin(), fields.end(),
                [&](auto &field) { field->Accept(*this); });

  end_arr();

  end_obj();
}

void IRWriter::Visit(FlowPtr<UnionTy> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  auto fields = n->GetFields();

  string("fields");
  begin_arr(fields.size());

  std::for_each(fields.begin(), fields.end(),
                [&](auto &field) { field->Accept(*this); });

  end_arr();

  end_obj();
}

void IRWriter::Visit(FlowPtr<ArrayTy> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("element");
  n->GetElement().Accept(*this);

  string("size");
  uint64(n->GetCount());

  end_obj();
}

void IRWriter::Visit(FlowPtr<FnTy> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("parameters");

  auto params = n->GetParams();
  begin_arr(params.size());

  std::for_each(params.begin(), params.end(),
                [&](auto &param) { param->Accept(*this); });

  end_arr();

  string("variadic");
  boolean(n->IsVariadic());

  string("return");
  n->GetReturn().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Int> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  string(n->GetValueString());

  string("type");
  uint64(n->GetSize());

  end_obj();
}

void IRWriter::Visit(FlowPtr<Float> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  dbl(n->GetValue());

  string("type");
  uint64(n->GetSize());

  end_obj();
}

void IRWriter::Visit(FlowPtr<List> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("items");
  begin_arr(n->Size());

  std::for_each(n->Begin(), n->End(), [&](auto &item) { item->Accept(*this); });

  end_arr();

  string("homegenous");
  boolean(n->IsHomogenous());

  end_obj();
}

void IRWriter::Visit(FlowPtr<Call> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("callee");
  /// TODO: Handle possibly cyclic reference to callee
  null();

  string("arguments");

  auto args = n->GetArgs();
  begin_arr(args.size());

  std::for_each(args.begin(), args.end(),
                [&](auto &arg) { arg->Accept(*this); });

  end_arr();

  end_obj();
}

void IRWriter::Visit(FlowPtr<Seq> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("items");
  begin_arr(n->Size());

  std::for_each(n->Begin(), n->End(), [&](auto &item) { item->Accept(*this); });

  end_arr();

  end_obj();
}

void IRWriter::Visit(FlowPtr<Index> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("base");
  n->GetExpr().Accept(*this);

  string("index");
  n->GetIndex().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Identifier> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  end_obj();
}

void IRWriter::Visit(FlowPtr<Extern> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("value");
  n->GetValue().Accept(*this);

  string("abi_name");
  string(n->GetAbiName());

  end_obj();
}

void IRWriter::Visit(FlowPtr<Local> n) {
  begin_obj(7);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  string("abi_name");
  string(n->GetAbiName());

  string("storage");
  string(STORAGE_CLASS_REPR.at(n->GetStorageClass()));

  string("readonly");
  boolean(n->IsReadonly());

  string("value");
  n->GetValue().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Ret> n) {
  begin_obj(3);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("expr");
  n->GetExpr().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Brk> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Cont> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<If> n) {
  begin_obj(5);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("then");
  n->GetThen().Accept(*this);

  string("else");
  n->GetElse().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<While> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<For> n) {
  begin_obj(6);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("init");
  n->GetInit().Accept(*this);

  string("cond");
  n->GetCond().Accept(*this);

  string("step");
  n->GetStep().Accept(*this);

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Case> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("body");
  n->GetBody().Accept(*this);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Switch> n) {
  begin_obj(1);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("cond");
  n->GetCond().Accept(*this);

  string("default");
  n->GetDefault().has_value() ? n->GetDefault().value()->Accept(*this) : null();

  string("cases");
  auto cases = n->GetCases();
  begin_arr(cases.size());

  std::for_each(cases.begin(), cases.end(), [&](auto &c) { c->Accept(*this); });

  end_arr();

  end_obj();
}

void IRWriter::Visit(FlowPtr<Function> n) {
  begin_obj(8);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("name");
  string(n->GetName());

  string("abi_name");
  string(n->GetAbiName());

  string("parameters");
  auto params = n->GetParams();
  begin_arr(params.size());

  std::for_each(params.begin(), params.end(), [&](auto &param) {
    begin_obj(2);

    string("name");
    string(param.second);

    string("type");
    param.first->Accept(*this);

    end_obj();
  });

  end_arr();

  string("variadic");
  boolean(n->IsVariadic());

  string("return");
  n->GetReturn().Accept(*this);

  string("body");
  n->GetBody().has_value() ? n->GetBody().value()->Accept(*this) : null();

  end_obj();
}

void IRWriter::Visit(FlowPtr<Asm> n) {
  begin_obj(2);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  end_obj();
}

void IRWriter::Visit(FlowPtr<Tmp> n) {
  begin_obj(4);

  string("kind");
  string(n->GetKindName());

  WriteSourceLocation(n);

  string("kind");
  uint64((int)n->GetTmpType());

  string("data");

  if (std::holds_alternative<GenericCallArgsTmpNodeCradle<void>>(
          n->GetData())) {
    auto data = std::get<GenericCallArgsTmpNodeCradle<void>>(n->GetData());

    begin_obj(2);

    string("base");
    data.m_base->Accept(*this);

    string("arguments");
    auto args = data.m_args;
    begin_arr(args.size());

    std::for_each(args.begin(), args.end(), [&](auto &arg) {
      begin_obj(2);

      string("name");
      string(arg.first);

      string("value");
      arg.second->Accept(*this);

      end_obj();
    });

    end_arr();

    end_obj();

  } else if (std::holds_alternative<ncc::string>(n->GetData())) {
    string(std::get<ncc::string>(n->GetData()));
  } else {
    qcore_panic("Unknown TmpNodeCradle type");
  }

  end_obj();
}
