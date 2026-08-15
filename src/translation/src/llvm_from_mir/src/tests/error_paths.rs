//! Tests that exercise error/panic paths in place, rvalue, and statement
//! lowering. These are `#[should_panic]` tests that drive deliberately
//! malformed (but representable) MIR into the relevant diagnostic branches so
//! those code paths are covered.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

// ── gen_place panics ──

#[test]
#[should_panic(expected = "not found in codegen context")]
fn local_not_found_panics() {
    let h = Harness::new();
    // A LocalId is never registered, so a Place::Local referencing a bogus id
    // lookup fails. We use `using_storage` to intern a local then reference it
    // in a different function's place (with a different codegen context it is
    // still missing). Since the local is not in the function being codegen'd,
    // the lookup panics.
    let (grafted, module) = mir::using_storage(&h.store, || {
        let mut builder = mir::MirBuilder::new();
        let mut other = builder.start_function("other".into(), mir::MirType::Unit.into());
        let ghost = other.new_temp(mir::MirType::I32.into(), false);
        other.create_block();
        other.ret(None);
        other.finish_function();

        // Build a *different* function that references the ghost local.
        let mut f = builder.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(ghost))));
        f.finish_function();
        (true, builder.build_module(mir::PtrSize::U64))
    });
    let _ = grafted;
    let _ = h.ir(&module); // panics: local not in codegen context
}

#[test]
#[should_panic(expected = "not found")]
fn static_not_found_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        f.ret(Some(mir::Operand::Copy(mir::Place::Static("missing_global".into()))));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "Cannot dereference non-pointer type")]
fn deref_non_pointer_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        let deref = mir::Place::Deref(Box::new(mir::Place::Local(x)));
        f.ret(Some(mir::Operand::Copy(deref)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "Field access on non-struct type")]
fn field_on_non_struct_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        let field = mir::Place::Field {
            base: Box::new(mir::Place::Local(x)),
            field_name: "x".into(),
        };
        f.ret(Some(mir::Operand::Copy(field)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "field 'missing' not found in struct layout")]
fn field_not_in_layout_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let struct_ty = mir::MirType::Struct {
            name: "S".into(),
            fields: thin_vec::thin_vec![("a".into(), mir::MirType::I32.into())],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "a".into() }],
        };
        let struct_id: mir::MirTypeId = struct_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let s = f.new_temp(struct_id, false);
        f.create_block();
        let field = mir::Place::Field {
            base: Box::new(mir::Place::Local(s)),
            field_name: "missing".into(),
        };
        f.ret(Some(mir::Operand::Copy(field)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "Index must be an integer")]
fn index_with_float_index_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let arr_ty = mir::MirType::Array {
            element_type: mir::MirType::I32.into(),
            len: 4,
        };
        let arr_id: mir::MirTypeId = arr_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let arr = f.new_temp(arr_id, false);
        let fidx = f.new_temp(mir::MirType::F32.into(), false);
        f.create_block();
        let idx = mir::Place::Index {
            base: Box::new(mir::Place::Local(arr)),
            index: Box::new(mir::Place::Local(fidx)),
        };
        f.ret(Some(mir::Operand::Copy(idx)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "Index access requires an array or slice type")]
fn index_on_non_array_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        let idx = f.new_temp(mir::MirType::USize.into(), false);
        f.create_block();
        let elem = mir::Place::Index {
            base: Box::new(mir::Place::Local(x)),
            index: Box::new(mir::Place::Local(idx)),
        };
        f.ret(Some(mir::Operand::Copy(elem)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "Downcast on non-enum type")]
fn downcast_on_non_enum_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        let downcast = mir::Place::Downcast {
            base: Box::new(mir::Place::Local(x)),
            variant_name: "A".into(),
        };
        f.ret(Some(mir::Operand::Copy(downcast)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "variant 'Missing' not found in enum")]
fn downcast_variant_not_found_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let enum_ty = mir::MirType::Enum {
            name: "E".into(),
            variants: thin_vec::thin_vec![mir::MirEnumVariant {
                name: "A".into(),
                payload: Some(mir::MirType::I32.into())
            }],
        };
        let enum_id: mir::MirTypeId = enum_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let e = f.new_temp(enum_id, false);
        f.create_block();
        let downcast = mir::Place::Downcast {
            base: Box::new(mir::Place::Local(e)),
            variant_name: "Missing".into(),
        };
        f.ret(Some(mir::Operand::Copy(downcast)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "variant 'Unit' has no payload to downcast to")]
fn downcast_payloadless_variant_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let enum_ty = mir::MirType::Enum {
            name: "E".into(),
            variants: thin_vec::thin_vec![
                mir::MirEnumVariant {
                    name: "Unit".into(),
                    payload: None
                },
                mir::MirEnumVariant {
                    name: "I32".into(),
                    payload: Some(mir::MirType::I32.into())
                },
            ],
        };
        let enum_id: mir::MirTypeId = enum_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let e = f.new_temp(enum_id, false);
        f.create_block();
        let downcast = mir::Place::Downcast {
            base: Box::new(mir::Place::Local(e)),
            variant_name: "Unit".into(),
        };
        f.ret(Some(mir::Operand::Copy(downcast)));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

// ── rvalue panics ──

#[test]
#[should_panic(expected = "Len on non-slice type")]
fn len_on_non_slice_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::USize.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        let len_tmp = f.new_temp(mir::MirType::USize.into(), false);
        f.push_assign(mir::Place::Local(len_tmp), mir::Rvalue::Len(mir::Place::Local(x)));
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(len_tmp))));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

// ── stmt panics ──

#[test]
#[should_panic(expected = "SetDiscriminant on non-enum type")]
fn set_discriminant_on_non_enum_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_set_discriminant(mir::Place::Local(x), 0);
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(x))));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "not found in LLVM module")]
fn direct_call_to_missing_function_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("caller".into(), mir::MirType::I32.into());
        let dest = f.new_temp(mir::MirType::I32.into(), false);
        let ret_block = f.reserve_block();
        f.create_block();
        f.set_terminator(mir::Terminator::Call {
            callee: mir::Operand::Copy(mir::Place::Static("does_not_exist".into())),
            args: thin_vec::ThinVec::new(),
            destination: Some(mir::Place::Local(dest)),
            target: Some(ret_block),
            target_args: thin_vec::ThinVec::new(),
        });
        f.switch_to_block(ret_block);
        f.ret(Some(mir::Operand::Constant(mir::MirLiteral::I32(0))));
        f.finish_function();
    });
    let _ = h.ir(&module);
}

#[test]
#[should_panic(expected = "Call target must be a function pointer")]
fn indirect_call_with_non_pointer_callee_panics() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("caller".into(), mir::MirType::Unit.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.set_terminator(mir::Terminator::Call {
            callee: mir::Operand::Copy(mir::Place::Local(x)),
            args: thin_vec::ThinVec::new(),
            destination: None,
            target: None,
            target_args: thin_vec::ThinVec::new(),
        });
        f.finish_function();
    });
    let _ = h.ir(&module);
}
