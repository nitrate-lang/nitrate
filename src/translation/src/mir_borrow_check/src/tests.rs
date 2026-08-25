//! Integration tests for the MIR borrow checker.
//!
//! These tests build MIR functions directly with `MirBuilder` and assert on
//! the collected borrow violations (or their absence). Each test exercises a
//! specific NLL rule: borrow regions bounded by liveness, aliasing XOR
//! mutation, two-phase borrows, escaping borrows, and move/init tracking.

use crate::collect_errors;
use nitrate_mir::prelude as mir;
use nitrate_mir::{
    AggregateKind, BorrowKind, MirBinaryOp, MirBuilder, MirLiteral, MirType, MirTypeId, Operand, Place, Rvalue,
};
use nitrate_nstring::NString;

/// A test harness that keeps a `MirStore` installed in thread-local storage.
struct Harness {
    store: mir::MirStore,
}

impl Harness {
    fn new() -> Self {
        Self { store: mir::MirStore::new() }
    }

    /// Build a module and return the deduplicated borrow errors for every
    /// function as display strings.
    fn errors(&self, build: impl FnOnce(&mut MirBuilder)) -> Vec<String> {
        mir::using_storage(&self.store, || {
            let mut builder = MirBuilder::new();
            build(&mut builder);
            let module = builder.build_module(mir::PtrSize::U64);
            let mut out = Vec::new();
            for func_id in &module.functions {
                let func = (*func_id).borrow().clone();
                for e in collect_errors(&func, &module) {
                    out.push(e.to_string());
                }
            }
            out.sort();
            out.dedup();
            out
        })
    }

    /// Assert that the module passes borrow checking.
    fn ok(&self, build: impl FnOnce(&mut MirBuilder)) {
        let errors = self.errors(build);
        assert!(errors.is_empty(), "expected no borrow errors, got:\n{errors:#?}");
    }

    /// Assert that the module is rejected with at least one error containing
    /// each expected substring.
    fn reject(&self, build: impl FnOnce(&mut MirBuilder), expected: &[&str]) {
        let errors = self.errors(build);
        let joined = errors.join("\n");
        for exp in expected {
            assert!(
                errors.iter().any(|e| e.contains(exp)),
                "expected an error containing {exp:?}, got:\n{joined}"
            );
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Type helpers
// ─────────────────────────────────────────────────────────────

fn i32_ty() -> MirTypeId {
    MirType::I32.into()
}

fn bool_ty() -> MirTypeId {
    MirType::Bool.into()
}

fn ref_ty(mutable: bool) -> MirTypeId {
    MirType::Reference { exclusive: false, mutable, to: i32_ty() }.into()
}

fn mut_ref_ty() -> MirTypeId {
    ref_ty(true)
}

fn shr_ref_ty() -> MirTypeId {
    ref_ty(false)
}

/// A `S { a: i32, b: i32 }` struct type.
fn struct_ty() -> MirTypeId {
    MirType::Struct {
        name: "S".into(),
        fields: vec![("a".into(), i32_ty()), ("b".into(), i32_ty())].into(),
        layout: vec![
            mir::MirStructLayoutCell::Field { field_name: "a".into() },
            mir::MirStructLayoutCell::Field { field_name: "b".into() },
        ]
        .into(),
    }
    .into()
}

/// Convenience extension for building test functions.
trait FnBuilderExt {
    fn local(&mut self, ty: MirTypeId, mutable: bool) -> mir::LocalId;
    fn assign_const(&mut self, dest: Place, lit: MirLiteral);
    fn borrow(&mut self, kind: BorrowKind, place: Place) -> mir::LocalId;
}

impl FnBuilderExt for mir::MirFunctionBuilder<'_> {
    fn local(&mut self, ty: MirTypeId, mutable: bool) -> mir::LocalId {
        self.new_temp(ty, mutable)
    }
    fn assign_const(&mut self, dest: Place, lit: MirLiteral) {
        self.push_assign(dest, Rvalue::Use(Operand::Constant(lit)));
    }
    fn borrow(&mut self, kind: BorrowKind, place: Place) -> mir::LocalId {
        let ty = match kind {
            BorrowKind::Shared => shr_ref_ty(),
            BorrowKind::Mutable => mut_ref_ty(),
        };
        let dest = self.new_temp(ty, false);
        self.push_assign(Place::Local(dest), Rvalue::Ref { region: kind, place });
        dest
    }
}

// ─────────────────────────────────────────────────────────────
// Basic ownership rules
// ─────────────────────────────────────────────────────────────

#[test]
fn no_borrows_is_clean() {
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("clean".into(), i32_ty());
        let x = f.local(i32_ty(), true);
        let y = f.local(i32_ty(), false);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        f.push_assign(
            Place::Local(y),
            Rvalue::BinaryOp {
                op: MirBinaryOp::Add,
                lhs: Operand::Copy(Place::Local(x)),
                rhs: Operand::Constant(MirLiteral::I32(1)),
            },
        );
        f.ret(Some(Operand::Copy(Place::Local(y))));
        f.finish_function();
    });
}

#[test]
fn mutable_borrow_of_immutable_local_is_rejected() {
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("bad_mut".into(), i32_ty());
            let x = f.local(i32_ty(), false); // immutable
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            f.borrow(BorrowKind::Mutable, Place::Local(x));
            f.ret(Some(Operand::Copy(Place::Local(x))));
            f.finish_function();
        },
        &["cannot borrow _1 as mutable", "not declared mutable"],
    );
}

#[test]
fn shared_borrow_of_immutable_local_is_ok() {
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("shared_ok".into(), i32_ty());
        let x = f.local(i32_ty(), false);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        let r = f.borrow(BorrowKind::Shared, Place::Local(x));
        f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
        f.finish_function();
    });
}

#[test]
fn two_mutable_borrows_conflict() {
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("two_mut".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r1 = f.borrow(BorrowKind::Mutable, Place::Local(x));
            let r2 = f.borrow(BorrowKind::Mutable, Place::Local(x));
            // Both borrows stay live to the end.
            let s = f.local(i32_ty(), false);
            f.push_assign(
                Place::Local(s),
                Rvalue::BinaryOp {
                    op: MirBinaryOp::Add,
                    lhs: Operand::Copy(Place::Deref(Box::new(Place::Local(r1)))),
                    rhs: Operand::Copy(Place::Deref(Box::new(Place::Local(r2)))),
                },
            );
            f.ret(Some(Operand::Copy(Place::Local(s))));
            f.finish_function();
        },
        &["cannot borrow _1 as mutable because it is already borrowed"],
    );
}

#[test]
fn shared_then_mutable_conflicts() {
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("shr_then_mut".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r1 = f.borrow(BorrowKind::Shared, Place::Local(x));
            let r2 = f.borrow(BorrowKind::Mutable, Place::Local(x));
            let s = f.local(i32_ty(), false);
            f.push_assign(
                Place::Local(s),
                Rvalue::BinaryOp {
                    op: MirBinaryOp::Add,
                    lhs: Operand::Copy(Place::Deref(Box::new(Place::Local(r1)))),
                    rhs: Operand::Copy(Place::Deref(Box::new(Place::Local(r2)))),
                },
            );
            f.ret(Some(Operand::Copy(Place::Local(s))));
            f.finish_function();
        },
        &["cannot borrow _1 as mutable because it is already borrowed (shared)"],
    );
}


// ─────────────────────────────────────────────────────────────
// NLL: borrow regions end at the last use
// ─────────────────────────────────────────────────────────────

#[test]
fn nll_mutable_borrow_ends_at_last_use() {
    // var x = 5; r = &mut x; *r += 1; s = &x; ret *s;
    // The mutable borrow of x dies after `*r += 1`, so the later shared
    // borrow is legal under NLL (it would be rejected by a lexical checker).
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("nll_last_use".into(), i32_ty());
        let x = f.local(i32_ty(), true);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        let r = f.borrow(BorrowKind::Mutable, Place::Local(x));
        // *r += 1 via a temp.
        let t = f.local(i32_ty(), false);
        f.push_assign(
            Place::Local(t),
            Rvalue::BinaryOp {
                op: MirBinaryOp::Add,
                lhs: Operand::Copy(Place::Deref(Box::new(Place::Local(r)))),
                rhs: Operand::Constant(MirLiteral::I32(1)),
            },
        );
        f.push_assign(
            Place::Deref(Box::new(Place::Local(r))),
            Rvalue::Use(Operand::Copy(Place::Local(t))),
        );
        let s = f.borrow(BorrowKind::Shared, Place::Local(x));
        f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(s))))));
        f.finish_function();
    });
}

#[test]
fn nll_unused_borrow_dies_immediately() {
    // var x = 5; r = &mut x; s = &mut x; ret x;
    // r is never used, so its borrow region is just its creation point; the
    // second mutable borrow is therefore legal (sound: r is dead code).
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("nll_unused".into(), i32_ty());
        let x = f.local(i32_ty(), true);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        f.borrow(BorrowKind::Mutable, Place::Local(x));
        f.borrow(BorrowKind::Mutable, Place::Local(x));
        f.ret(Some(Operand::Copy(Place::Local(x))));
        f.finish_function();
    });
}

#[test]
fn read_while_mutably_borrowed_is_rejected() {
    // var x = 5; r = &mut x; y = x; ret *r;
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("read_while_mut".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r = f.borrow(BorrowKind::Mutable, Place::Local(x));
            let y = f.local(i32_ty(), false);
            f.push_assign(Place::Local(y), Rvalue::Use(Operand::Copy(Place::Local(x))));
            f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
            f.finish_function();
        },
        &["cannot read _1 because it is mutably borrowed"],
    );
}

#[test]
fn write_while_borrowed_is_rejected() {
    // r = &x; x = 5; ret *r
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("write_while_borrowed".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r = f.borrow(BorrowKind::Shared, Place::Local(x));
            f.assign_const(Place::Local(x), MirLiteral::I32(7));
            f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
            f.finish_function();
        },
        &["cannot assign to _1 because it is borrowed (shared)"],
    );
}

#[test]
fn move_while_borrowed_is_rejected() {
    // r = &x; y = move(x); ret *r
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("move_while_borrowed".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r = f.borrow(BorrowKind::Shared, Place::Local(x));
            let y = f.local(i32_ty(), false);
            f.push_assign(Place::Local(y), Rvalue::Use(Operand::Move(Place::Local(x))));
            f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
            f.finish_function();
        },
        &["cannot move out of _1 because it is borrowed"],
    );
}

#[test]
fn use_after_move_is_rejected() {
    // var x = 5; y = move(x); z = x; ret z
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("use_after_move".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let y = f.local(i32_ty(), false);
            f.push_assign(Place::Local(y), Rvalue::Use(Operand::Move(Place::Local(x))));
            let z = f.local(i32_ty(), false);
            f.push_assign(Place::Local(z), Rvalue::Use(Operand::Copy(Place::Local(x))));
            f.ret(Some(Operand::Copy(Place::Local(z))));
            f.finish_function();
        },
        &["use of moved value _1"],
    );
}

#[test]
fn use_before_init_is_rejected() {
    // ret x  — x never assigned
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("use_before_init".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.ret(Some(Operand::Copy(Place::Local(x))));
            f.finish_function();
        },
        &["use of possibly-uninitialized value _1"],
    );
}


// ─────────────────────────────────────────────────────────────
// Escaping borrows
// ─────────────────────────────────────────────────────────────

#[test]
fn returning_borrow_of_local_is_rejected() {
    // x = 5; r = &x; ret r
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("escape_local".into(), shr_ref_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r = f.borrow(BorrowKind::Shared, Place::Local(x));
            f.ret(Some(Operand::Copy(Place::Local(r))));
            f.finish_function();
        },
        &["cannot return a reference to local variable _1"],
    );
}

#[test]
fn returning_reborrow_of_parameter_is_ok() {
    // fn f(p: &i32) -> &i32 { r = &*p; ret r }
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("reborrow_param".into(), shr_ref_ty());
        let p = f.add_param("p".into(), shr_ref_ty(), false);
        f.create_block();
        let r = f.borrow(BorrowKind::Shared, Place::Deref(Box::new(Place::Local(p))));
        f.ret(Some(Operand::Copy(Place::Local(r))));
        f.finish_function();
    });
}

#[test]
fn returning_borrow_of_static_string_is_ok() {
    // r = &"hello"; ret r  — statics have static lifetime.
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("escape_static".into(), shr_ref_ty());
        f.create_block();
        let sname: NString = "__nitrate_str_0".into();
        let r = f.borrow(BorrowKind::Shared, Place::Static(sname));
        f.ret(Some(Operand::Copy(Place::Local(r))));
        f.finish_function();
    });
}

// ─────────────────────────────────────────────────────────────
// Field-level borrowing
// ─────────────────────────────────────────────────────────────

#[test]
fn disjoint_field_borrows_do_not_conflict() {
    // var s = S{a:1, b:2}; ra = &s.a; sb = &mut s.b; ret *ra
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("field_ok".into(), i32_ty());
        let s = f.local(struct_ty(), true);
        f.create_block();
        f.push_assign(
            Place::Local(s),
            Rvalue::Aggregate(
                AggregateKind::Struct("S".into(), vec!["a".into(), "b".into()].into()),
                vec![
                    Operand::Constant(MirLiteral::I32(1)),
                    Operand::Constant(MirLiteral::I32(2)),
                ]
                .into(),
            ),
        );
        let ra = f.borrow(BorrowKind::Shared, Place::Field { base: Box::new(Place::Local(s)), field_name: "a".into() });
        let rb = f.borrow(BorrowKind::Mutable, Place::Field { base: Box::new(Place::Local(s)), field_name: "b".into() });
        // Use both borrows so they stay live.
        f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(ra))))));
        let _ = rb;
        f.finish_function();
    });
}

#[test]
fn write_whole_struct_while_field_borrowed_conflicts() {
    // var s = S{a:1,b:2}; r = &s.a; s = S{a:3,b:4}; ret *r
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("field_write_whole".into(), i32_ty());
            let s = f.local(struct_ty(), true);
            f.create_block();
            f.push_assign(
                Place::Local(s),
                Rvalue::Aggregate(
                    AggregateKind::Struct("S".into(), vec!["a".into(), "b".into()].into()),
                    vec![
                        Operand::Constant(MirLiteral::I32(1)),
                        Operand::Constant(MirLiteral::I32(2)),
                    ]
                    .into(),
                ),
            );
            let r = f.borrow(BorrowKind::Shared, Place::Field { base: Box::new(Place::Local(s)), field_name: "a".into() });
            f.push_assign(
                Place::Local(s),
                Rvalue::Aggregate(
                    AggregateKind::Struct("S".into(), vec!["a".into(), "b".into()].into()),
                    vec![
                        Operand::Constant(MirLiteral::I32(3)),
                        Operand::Constant(MirLiteral::I32(4)),
                    ]
                    .into(),
                ),
            );
            f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
            f.finish_function();
        },
        &["cannot assign to _1 because it is borrowed (shared)"],
    );
}

#[test]
fn write_disjoint_field_while_field_borrowed_is_ok() {
    // var s = S{a:1,b:2}; r = &s.a; s.b = 9; ret *r
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("field_write_disjoint".into(), i32_ty());
        let s = f.local(struct_ty(), true);
        f.create_block();
        f.push_assign(
            Place::Local(s),
            Rvalue::Aggregate(
                AggregateKind::Struct("S".into(), vec!["a".into(), "b".into()].into()),
                vec![
                    Operand::Constant(MirLiteral::I32(1)),
                    Operand::Constant(MirLiteral::I32(2)),
                ]
                .into(),
            ),
        );
        let r = f.borrow(BorrowKind::Shared, Place::Field { base: Box::new(Place::Local(s)), field_name: "a".into() });
        f.push_assign(
            Place::Field { base: Box::new(Place::Local(s)), field_name: "b".into() },
            Rvalue::Use(Operand::Constant(MirLiteral::I32(9))),
        );
        f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
        f.finish_function();
    });
}


// ─────────────────────────────────────────────────────────────
// Two-phase borrows
// ─────────────────────────────────────────────────────────────

#[test]
fn two_phase_borrow_permits_shared_borrow_during_reservation() {
    // var x = 5;
    // t = &mut x;          (reservation: t used once, as call argument)
    // s = &x;              (shared borrow during reservation — legal)
    // y = use_shared(s);   (s's last use)
    // z = use_mut(t);      (activation at the call)
    // ret z;
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("two_phase_ok".into(), i32_ty());
        let x = f.local(i32_ty(), true);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        let t = f.borrow(BorrowKind::Mutable, Place::Local(x));
        let s = f.borrow(BorrowKind::Shared, Place::Local(x));
        let cont = f.reserve_block();
        let cont2 = f.reserve_block();
        let y = f.local(i32_ty(), false);
        f.call_return(
            Operand::Copy(Place::Static("use_shared".into())),
            vec![Operand::Copy(Place::Local(s))].into(),
            Place::Local(y),
            cont,
        );
        f.switch_to_block(cont);
        let z = f.local(i32_ty(), false);
        f.call_return(
            Operand::Copy(Place::Static("use_mut".into())),
            vec![Operand::Copy(Place::Local(t))].into(),
            Place::Local(z),
            cont2,
        );
        f.switch_to_block(cont2);
        f.ret(Some(Operand::Copy(Place::Local(z))));
        f.finish_function();
    });
}

#[test]
fn two_phase_activation_conflicts_with_still_active_shared_borrow() {
    // var x = 5;
    // t = &mut x;          (reservation)
    // s = &x;              (shared during reservation)
    // z = g(t, s);         (activation while s is still live → conflict)
    // ret z;
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("two_phase_conflict".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let t = f.borrow(BorrowKind::Mutable, Place::Local(x));
            let s = f.borrow(BorrowKind::Shared, Place::Local(x));
            let cont = f.reserve_block();
            let z = f.local(i32_ty(), false);
            f.call_return(
                Operand::Copy(Place::Static("g".into())),
                vec![Operand::Copy(Place::Local(t)), Operand::Copy(Place::Local(s))].into(),
                Place::Local(z),
                cont,
            );
            f.switch_to_block(cont);
            f.ret(Some(Operand::Copy(Place::Local(z))));
            f.finish_function();
        },
        &["was activated while a shared borrow"],
    );
}

#[test]
fn two_phase_mutable_borrow_used_twice_is_not_a_reservation() {
    // t = &mut x; s = &x; y = f(t); z = g(t);
    // t is used as a call argument twice → not two-phase → its mutable borrow
    // is activated immediately, so the shared borrow of x conflicts.
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("two_phase_used_twice".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let t = f.borrow(BorrowKind::Mutable, Place::Local(x));
            let s = f.borrow(BorrowKind::Shared, Place::Local(x));
            let cont = f.reserve_block();
            let cont2 = f.reserve_block();
            let y = f.local(i32_ty(), false);
            f.call_return(
                Operand::Copy(Place::Static("f".into())),
                vec![Operand::Copy(Place::Local(t))].into(),
                Place::Local(y),
                cont,
            );
            f.switch_to_block(cont);
            let z = f.local(i32_ty(), false);
            f.call_return(
                Operand::Copy(Place::Static("g".into())),
                vec![Operand::Copy(Place::Local(t))].into(),
                Place::Local(z),
                cont2,
            );
            f.switch_to_block(cont2);
            f.ret(Some(Operand::Copy(Place::Local(z))));
            f.finish_function();
        },
        &["as shared because it is already mutably borrowed"],
    );
}


// ─────────────────────────────────────────────────────────────
// Loops and control flow
// ─────────────────────────────────────────────────────────────

#[test]
fn borrow_created_in_loop_and_used_after_loop_is_live_at_exit() {
    // var x = 5;
    // r = &mut x;
    // loop { *r += 1; }       (r live on the back edge)
    // x = 5;                  (write while r's borrow still live → conflict)
    // ret x;
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("loop_escape".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            let cond = f.local(bool_ty(), false);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r = f.borrow(BorrowKind::Mutable, Place::Local(x));
            let header = f.reserve_block();
            let body = f.reserve_block();
            let exit = f.reserve_block();
            f.goto(header);

            f.switch_to_block(header);
            f.assign_const(Place::Local(cond), MirLiteral::Bool(true));
            f.if_br(Operand::Copy(Place::Local(cond)), body, exit);

            f.switch_to_block(body);
            let t = f.local(i32_ty(), false);
            f.push_assign(
                Place::Local(t),
                Rvalue::BinaryOp {
                    op: MirBinaryOp::Add,
                    lhs: Operand::Copy(Place::Deref(Box::new(Place::Local(r)))),
                    rhs: Operand::Constant(MirLiteral::I32(1)),
                },
            );
            f.push_assign(
                Place::Deref(Box::new(Place::Local(r))),
                Rvalue::Use(Operand::Copy(Place::Local(t))),
            );
            f.goto(header);

            f.switch_to_block(exit);
            f.assign_const(Place::Local(x), MirLiteral::I32(7));
            f.ret(Some(Operand::Copy(Place::Local(x))));
            f.finish_function();
        },
        &["because it is borrowed (mutable)"],
    );
}

#[test]
fn borrow_ending_before_loop_is_clean() {
    // var x = 5; r = &mut x; *r += 1; loop { x = 6; }  — r dead before loop.
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("loop_clean".into(), i32_ty());
        let x = f.local(i32_ty(), true);
        let cond = f.local(bool_ty(), false);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        let r = f.borrow(BorrowKind::Mutable, Place::Local(x));
        let t = f.local(i32_ty(), false);
        f.push_assign(
            Place::Local(t),
            Rvalue::BinaryOp {
                op: MirBinaryOp::Add,
                lhs: Operand::Copy(Place::Deref(Box::new(Place::Local(r)))),
                rhs: Operand::Constant(MirLiteral::I32(1)),
            },
        );
        f.push_assign(
            Place::Deref(Box::new(Place::Local(r))),
            Rvalue::Use(Operand::Copy(Place::Local(t))),
        );
        let header = f.reserve_block();
        let body = f.reserve_block();
        let exit = f.reserve_block();
        f.goto(header);

        f.switch_to_block(header);
        f.assign_const(Place::Local(cond), MirLiteral::Bool(true));
        f.if_br(Operand::Copy(Place::Local(cond)), body, exit);

        f.switch_to_block(body);
        f.assign_const(Place::Local(x), MirLiteral::I32(6));
        f.goto(header);

        f.switch_to_block(exit);
        f.ret(Some(Operand::Copy(Place::Local(x))));
        f.finish_function();
    });
}

// ─────────────────────────────────────────────────────────────
// Deref and reborrow semantics
// ─────────────────────────────────────────────────────────────

#[test]
fn writing_through_a_borrow_is_allowed() {
    // var x = 5; r = &mut x; *r = 9; ret *r
    let h = Harness::new();
    h.ok(|b| {
        let mut f = b.start_function("write_through".into(), i32_ty());
        let x = f.local(i32_ty(), true);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        let r = f.borrow(BorrowKind::Mutable, Place::Local(x));
        f.push_assign(
            Place::Deref(Box::new(Place::Local(r))),
            Rvalue::Use(Operand::Constant(MirLiteral::I32(9))),
        );
        f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
        f.finish_function();
    });
}

#[test]
fn writing_the_borrowed_place_through_a_reference_conflicts() {
    // var x = 5; r = &mut x; x = 9; ret *r  — direct write while borrowed.
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("write_direct_conflict".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r = f.borrow(BorrowKind::Mutable, Place::Local(x));
            f.assign_const(Place::Local(x), MirLiteral::I32(9));
            f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r))))));
            f.finish_function();
        },
        &["cannot assign to _1 because it is borrowed (mutable)"],
    );
}

#[test]
fn borrowing_an_uninitialized_value_is_rejected() {
    // r = &x  — x never assigned
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("borrow_uninit".into(), shr_ref_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            let r = f.borrow(BorrowKind::Shared, Place::Local(x));
            f.ret(Some(Operand::Copy(Place::Local(r))));
            f.finish_function();
        },
        &["cannot borrow _1 because it is possibly-uninitialized"],
    );
}


// ─────────────────────────────────────────────────────────────
// Borrow-value propagation
// ─────────────────────────────────────────────────────────────

#[test]
fn borrow_propagates_through_copies() {
    // x = 5; r1 = &x; r2 = r1; ret r2  — the borrow of x is live through the
    // copy into r2, so returning r2 is an escaping borrow error.
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("copy_propagation".into(), shr_ref_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r1 = f.borrow(BorrowKind::Shared, Place::Local(x));
            let r2 = f.local(shr_ref_ty(), false);
            f.push_assign(
                Place::Local(r2),
                Rvalue::Use(Operand::Copy(Place::Local(r1))),
            );
            f.ret(Some(Operand::Copy(Place::Local(r2))));
            f.finish_function();
        },
        &["cannot return a reference to local variable _1"],
    );
}

#[test]
fn write_after_borrow_propagated_through_copy_conflicts() {
    // var x = 5; r1 = &x; r2 = r1; x = 9; ret *r2  — x written while the
    // borrow (now held by r2) is live.
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("copy_write_conflict".into(), i32_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r1 = f.borrow(BorrowKind::Shared, Place::Local(x));
            let r2 = f.local(shr_ref_ty(), false);
            f.push_assign(
                Place::Local(r2),
                Rvalue::Use(Operand::Copy(Place::Local(r1))),
            );
            f.assign_const(Place::Local(x), MirLiteral::I32(9));
            f.ret(Some(Operand::Copy(Place::Deref(Box::new(Place::Local(r2))))));
            f.finish_function();
        },
        &["cannot assign to _1 because it is borrowed (shared)"],
    );
}

#[test]
fn borrow_flowing_through_block_arguments_is_tracked() {
    // var x = 5; r = &x; goto merge(r);  merge(v): ret v
    // The borrow flows through the block argument `v`; returning it escapes.
    let h = Harness::new();
    h.reject(
        |b| {
            let mut f = b.start_function("block_arg_escape".into(), shr_ref_ty());
            let x = f.local(i32_ty(), true);
            f.create_block();
            f.assign_const(Place::Local(x), MirLiteral::I32(5));
            let r = f.borrow(BorrowKind::Shared, Place::Local(x));
            let merge = f.reserve_block_with_args(&[shr_ref_ty()]);
            f.goto_with_args(merge.block, vec![Operand::Copy(Place::Local(r))].into());
            f.switch_to_block(merge.block);
            f.ret(Some(Operand::Copy(Place::Local(merge.arg_locals[0]))));
            f.finish_function();
        },
        &["cannot return a reference to local variable _1"],
    );
}


// ─────────────────────────────────────────────────────────────
// Log integration (pipeline-facing API)
// ─────────────────────────────────────────────────────────────

#[test]
fn check_mir_module_reports_errors_to_the_log() {
    use crate::check_mir_module;
    use nitrate_diagnosis::CompilerLog;

    mir::using_storage(&Harness::new().store, || {
        let mut builder = MirBuilder::new();
        let mut f = builder.start_function("log_report".into(), shr_ref_ty());
        let x = f.local(i32_ty(), true);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        let r = f.borrow(BorrowKind::Shared, Place::Local(x));
        f.ret(Some(Operand::Copy(Place::Local(r))));
        f.finish_function();
        let module = builder.build_module(mir::PtrSize::U64);

        let log = CompilerLog::default();
        assert!(check_mir_module(&module, &log).is_err(), "escaping borrow must be reported");
        assert!(log.error_bit(), "the compiler log's error bit must be set");
    });
}

#[test]
fn check_mir_module_passes_valid_module() {
    use crate::check_mir_module;
    use nitrate_diagnosis::CompilerLog;

    mir::using_storage(&Harness::new().store, || {
        let mut builder = MirBuilder::new();
        let mut f = builder.start_function("log_pass".into(), i32_ty());
        let x = f.local(i32_ty(), true);
        f.create_block();
        f.assign_const(Place::Local(x), MirLiteral::I32(5));
        f.ret(Some(Operand::Copy(Place::Local(x))));
        f.finish_function();
        let module = builder.build_module(mir::PtrSize::U64);

        let log = CompilerLog::default();
        assert!(check_mir_module(&module, &log).is_ok(), "valid module must pass");
        assert!(!log.error_bit(), "no errors must be reported");
    });
}

