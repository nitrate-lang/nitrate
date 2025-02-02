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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Nodes.hh>

using namespace ncc;
using namespace ncc::ir;

class CloneVisitor : public IRVisitor<void> {
  std::optional<Expr *> m_r;

  void for_each(  // NOLINT
      const auto &container, const auto &callback) {
    for (const auto &item : container) {
      callback(item);
    }
  }

public:
  CloneVisitor() {}
  virtual ~CloneVisitor() = default;

  Expr *GetClone() { return m_r.value(); }

  void Visit(FlowPtr<Expr> n) override { m_r = Create<Expr>(n->GetKind()); }

  void Visit(FlowPtr<Binary> n) override {
    auto lhs = n->GetLHS()->Clone();
    auto rhs = n->GetRHS()->Clone();
    auto op = n->GetOp();

    m_r = Create<Binary>(lhs, rhs, op);
  }

  void Visit(FlowPtr<Unary> n) override {
    auto expr = n->GetExpr()->Clone();
    auto op = n->GetOp();
    auto postfix = n->IsPostfix();

    m_r = Create<Unary>(expr, op, postfix);
  }

  void Visit(FlowPtr<U1Ty>) override { m_r = GetU1Ty(); }
  void Visit(FlowPtr<U8Ty>) override { m_r = GetU8Ty(); }
  void Visit(FlowPtr<U16Ty>) override { m_r = GetU16Ty(); }
  void Visit(FlowPtr<U32Ty>) override { m_r = GetU32Ty(); }
  void Visit(FlowPtr<U64Ty>) override { m_r = GetU64Ty(); }
  void Visit(FlowPtr<U128Ty>) override { m_r = GetU128Ty(); }
  void Visit(FlowPtr<I8Ty>) override { m_r = GetI8Ty(); }
  void Visit(FlowPtr<I16Ty>) override { m_r = GetI16Ty(); }
  void Visit(FlowPtr<I32Ty>) override { m_r = GetI32Ty(); }
  void Visit(FlowPtr<I64Ty>) override { m_r = GetI64Ty(); }
  void Visit(FlowPtr<I128Ty>) override { m_r = GetI128Ty(); }
  void Visit(FlowPtr<F16Ty>) override { m_r = GetF16Ty(); }
  void Visit(FlowPtr<F32Ty>) override { m_r = GetF32Ty(); }
  void Visit(FlowPtr<F64Ty>) override { m_r = GetF64Ty(); }
  void Visit(FlowPtr<F128Ty>) override { m_r = GetF128Ty(); }
  void Visit(FlowPtr<VoidTy>) override { m_r = GetVoidTy(); }
  void Visit(FlowPtr<OpaqueTy> n) override { m_r = GetOpaqueTy(n->GetName()); }

  void Visit(FlowPtr<StructTy> n) override {
    std::vector<FlowPtr<Type>> fields;
    fields.reserve(n->GetFields().size());

    for_each(n->GetFields(), [&](auto item) { fields.push_back(item->template Clone<Type>()); });

    m_r = GetStructTy(fields);
  }

  void Visit(FlowPtr<UnionTy> n) override {
    std::vector<FlowPtr<Type>> fields;
    fields.reserve(n->GetFields().size());

    for_each(n->GetFields(), [&](auto item) { fields.push_back(item->template Clone<Type>()); });

    m_r = GetUnionTy(fields);
  }

  void Visit(FlowPtr<PtrTy> n) override { m_r = GetPtrTy(n->GetPointee()->Clone<Type>(), n->GetNativeSize()); }

  void Visit(FlowPtr<ConstTy> n) override { m_r = GetConstTy(n->GetItem()->Clone<Type>()); }

  void Visit(FlowPtr<ArrayTy> n) override { m_r = GetArrayTy(n->GetElement()->Clone<Type>(), n->GetCount()); }

  void Visit(FlowPtr<FnTy> n) override {
    std::vector<FlowPtr<Type>> params;
    params.reserve(n->GetParams().size());
    for_each(n->GetParams(), [&](auto item) { params.push_back(item->template Clone<Type>()); });

    m_r = GetFnTy(params, n->GetReturn()->Clone<Type>(), n->IsVariadic(), n->GetNativeSize());
  }

  void Visit(FlowPtr<Int> n) override { m_r = Create<Int>(n->GetValue(), n->GetSize()); }

  void Visit(FlowPtr<Float> n) override { m_r = Create<Float>(n->GetValue(), n->GetSize()); }

  void Visit(FlowPtr<List> n) override {
    GenericListItems<void> items;
    items.reserve(n->Size());

    std::for_each(n->Begin(), n->End(), [&](auto item) { items.push_back(item->Clone()); });

    m_r = Create<List>(items, n->IsHomogenous());
  }

  void Visit(FlowPtr<Call> n) override {
    GenericCallArgs<void> args;
    args.reserve(n->GetArgs().size());

    for_each(n->GetArgs(), [&](auto item) { args.push_back(item->Clone()); });

    auto old_ref = n->GetTarget();  // Resolve later

    m_r = Create<Call>(old_ref, args);
  }

  void Visit(FlowPtr<Seq> n) override {
    GenericSeqItems<void> items;
    items.reserve(n->Size());

    for_each(n->GetItems(), [&](auto item) { items.push_back(item->Clone()); });

    m_r = Create<Seq>(items);
  }

  void Visit(FlowPtr<Index> n) override {
    auto base = n->GetExpr()->Clone();
    auto index = n->GetIndex()->Clone();

    m_r = Create<Index>(base, index);
  }

  void Visit(FlowPtr<Identifier> n) override {
    auto name = n->GetName();
    auto old_ref = n->GetWhat();  // Resolve later

    m_r = Create<Identifier>(name, old_ref);
  }

  void Visit(FlowPtr<Extern> n) override {
    auto value = n->GetValue()->Clone();
    auto abi_name = n->GetAbiName();

    m_r = Create<Extern>(value, abi_name);
  }

  void Visit(FlowPtr<Local> n) override {
    auto name = n->GetName();
    auto value = n->GetValue()->Clone();
    auto abi_name = n->GetAbiName();
    auto readonly = n->IsReadonly();
    auto storage_class = n->GetStorageClass();

    m_r = Create<Local>(name, value, abi_name, readonly, storage_class);
  }

  void Visit(FlowPtr<Ret> n) override {
    auto expr = n->GetExpr()->Clone();

    m_r = Create<Ret>(expr);
  }

  void Visit(FlowPtr<Brk>) override { m_r = Create<Brk>(); }
  void Visit(FlowPtr<Cont>) override { m_r = Create<Cont>(); }

  void Visit(FlowPtr<If> n) override {
    auto cond = n->GetCond()->Clone();
    auto then = n->GetThen()->Clone();
    auto ele = n->GetElse()->Clone();

    m_r = Create<If>(cond, then, ele);
  }

  void Visit(FlowPtr<While> n) override {
    auto cond = n->GetCond()->Clone();
    auto body = n->GetBody()->Clone<Seq>();

    m_r = Create<While>(cond, body);
  }

  void Visit(FlowPtr<For> n) override {
    auto init = n->GetInit()->Clone();
    auto cond = n->GetCond()->Clone();
    auto step = n->GetStep()->Clone();
    auto body = n->GetBody()->Clone();

    m_r = Create<For>(init, cond, step, body);
  }

  void Visit(FlowPtr<Case> n) override {
    auto cond = n->GetCond()->Clone();
    auto body = n->GetBody()->Clone();

    m_r = Create<Case>(cond, body);
  }

  void Visit(FlowPtr<Switch> n) override {
    GenericSwitchCases<void> cases;
    cases.reserve(n->GetCases().size());

    for_each(n->GetCases(), [&](auto item) { cases.push_back(item->template Clone<Case>()); });

    auto cond = n->GetCond()->Clone();
    auto default_case = n->GetDefault().has_value() ? n->GetDefault().value()->Clone() : nullptr;

    m_r = Create<Switch>(cond, cases, default_case);
  }

  void Visit(FlowPtr<Function> n) override {
    GenericParams<void> params;
    params.reserve(n->GetParams().size());

    for_each(n->GetParams(), [&](auto item) { params.push_back({item.first->template Clone<Type>(), item.second}); });

    auto body = n->GetBody().has_value() ? n->GetBody().value()->Clone<Seq>() : nullptr;
    auto return_type = n->GetReturn()->Clone<Type>();
    auto name = n->GetName();
    auto abi_name = n->GetAbiName();
    auto is_variadic = n->IsVariadic();

    m_r = Create<Function>(name, params, return_type, body, is_variadic, abi_name);
  }

  void Visit(FlowPtr<Asm>) override { qcore_panic("Cannot Clone Asm node because it is not implemented"); }

  void Visit(FlowPtr<Tmp> n) override {
    if (std::holds_alternative<string>(n->GetData())) {
      m_r = Create<Tmp>(n->GetTmpType(), std::get<string>(n->GetData()));
    } else if (std::holds_alternative<GenericCallArgsTmpNodeCradle<void>>(n->GetData())) {
      auto data = std::get<GenericCallArgsTmpNodeCradle<void>>(n->GetData());
      auto base = data.m_base->Clone();
      GenericCallArguments<void> args;
      args.reserve(data.m_args.size());

      for_each(data.m_args, [&](auto item) { args.push_back({item.first, item.second->Clone()}); });

      m_r = Create<Tmp>(n->GetTmpType(), GenericCallArgsTmpNodeCradle<void>{base, args});
    } else {
      qcore_panic("Unknown Tmp node data type");
    }
  }
};

///===========================================================================///

NCC_EXPORT Expr *detail::ExprGetCloneImpl(Expr *self) {
  static thread_local struct {
    std::unordered_map<Expr *, Expr *> m_in_out;
    size_t m_depth = 0;
  } state; /* The state behaves like a recursive argument */

  {
    state.m_depth++;

    CloneVisitor v;
    self->Accept(v);

    FlowPtr<Expr> e = v.GetClone();
    e->SetLoc(self->GetLoc());

    state.m_depth--;

    if (state.m_depth == 0) {
      // Resolve internal cyclic references

      for_each(e, [](auto ty, auto n) {
        switch (ty) {
          case IR_eIDENT: {
            auto ident = n->template As<Identifier>();

            if (auto what = ident->GetWhat()) {
              if (auto it = state.m_in_out.find(what.value().get()); it != state.m_in_out.end()) {
                ident->SetWhat(it->second);
              }
            }
          }

          case IR_eCALL: {
            auto call = n->template As<Call>();

            if (auto target = call->GetTarget()) {
              if (auto it = state.m_in_out.find(target.value().get()); it != state.m_in_out.end()) {
                call->SetTarget(it->second);
              }
            }
          }

          default: {
            break;
          }
        }
      });

      state.m_in_out.clear();
    }

    return e.get();
  }
}
