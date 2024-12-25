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

#ifndef __NITRATE_IR_VISITOR_H__
#define __NITRATE_IR_VISITOR_H__

#include <nitrate-ir/TypeDecl.hh>

namespace ncc::ir {
  class Expr;
  class Type;
  class BinExpr;
  class UnExpr;
  class PostUnExpr;
  class U1Ty;
  class U8Ty;
  class U16Ty;
  class U32Ty;
  class U64Ty;
  class U128Ty;
  class I8Ty;
  class I16Ty;
  class I32Ty;
  class I64Ty;
  class I128Ty;
  class F16Ty;
  class F32Ty;
  class F64Ty;
  class F128Ty;
  class VoidTy;
  class PtrTy;
  class ConstTy;
  class OpaqueTy;
  class StructTy;
  class UnionTy;
  class ArrayTy;
  class FnTy;
  class Int;
  class Float;
  class List;
  class Call;
  class Seq;
  class Index;
  class Ident;
  class Extern;
  class Local;
  class Ret;
  class Brk;
  class Cont;
  class If;
  class While;
  class For;
  class Case;
  class Switch;
  class Fn;
  class Asm;
  class Tmp;

}  // namespace ncc::ir

namespace ncc::ir {
  class NRVisitor {
  public:
    virtual ~NRVisitor() = default;

    virtual void visit(Expr& n) = 0;
    virtual void visit(Type& n) = 0;
    virtual void visit(BinExpr& n) = 0;
    virtual void visit(UnExpr& n) = 0;
    virtual void visit(PostUnExpr& n) = 0;
    virtual void visit(U1Ty& n) = 0;
    virtual void visit(U8Ty& n) = 0;
    virtual void visit(U16Ty& n) = 0;
    virtual void visit(U32Ty& n) = 0;
    virtual void visit(U64Ty& n) = 0;
    virtual void visit(U128Ty& n) = 0;
    virtual void visit(I8Ty& n) = 0;
    virtual void visit(I16Ty& n) = 0;
    virtual void visit(I32Ty& n) = 0;
    virtual void visit(I64Ty& n) = 0;
    virtual void visit(I128Ty& n) = 0;
    virtual void visit(F16Ty& n) = 0;
    virtual void visit(F32Ty& n) = 0;
    virtual void visit(F64Ty& n) = 0;
    virtual void visit(F128Ty& n) = 0;
    virtual void visit(VoidTy& n) = 0;
    virtual void visit(PtrTy& n) = 0;
    virtual void visit(ConstTy& n) = 0;
    virtual void visit(OpaqueTy& n) = 0;
    virtual void visit(StructTy& n) = 0;
    virtual void visit(UnionTy& n) = 0;
    virtual void visit(ArrayTy& n) = 0;
    virtual void visit(FnTy& n) = 0;
    virtual void visit(Int& n) = 0;
    virtual void visit(Float& n) = 0;
    virtual void visit(List& n) = 0;
    virtual void visit(Call& n) = 0;
    virtual void visit(Seq& n) = 0;
    virtual void visit(Index& n) = 0;
    virtual void visit(Ident& n) = 0;
    virtual void visit(Extern& n) = 0;
    virtual void visit(Local& n) = 0;
    virtual void visit(Ret& n) = 0;
    virtual void visit(Brk& n) = 0;
    virtual void visit(Cont& n) = 0;
    virtual void visit(If& n) = 0;
    virtual void visit(While& n) = 0;
    virtual void visit(For& n) = 0;
    virtual void visit(Case& n) = 0;
    virtual void visit(Switch& n) = 0;
    virtual void visit(Fn& n) = 0;
    virtual void visit(Asm& n) = 0;
    virtual void visit(Tmp& n) = 0;
  };
}  // namespace ncc::ir

#endif  // __NITRATE_IR_VISITOR_H__
