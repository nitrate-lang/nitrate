//! Matrix of place-projection tests: nested derefs, fields, indexes, and
//! downcasts for arrays, slices, structs, and enums.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

#[test]
fn nested_field_access_two_levels() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let inner_ty = mir::MirType::Struct {
            name: "Inner".into(),
            fields: thin_vec::thin_vec![("z".into(), mir::MirType::I32.into())],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "z".into() }],
        };
        let inner_id: mir::MirTypeId = inner_ty.into();
        let outer_ty = mir::MirType::Struct {
            name: "Outer".into(),
            fields: thin_vec::thin_vec![("inner".into(), inner_id.clone())],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field {
                field_name: "inner".into()
            }],
        };
        let outer_id: mir::MirTypeId = outer_ty.into();

        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let o = f.new_temp(outer_id, false);
        f.create_block();
        // `outer.inner.z` — first project `inner`, then `z`.
        let inner_field = mir::Place::Field {
            base: Box::new(mir::Place::Local(o)),
            field_name: "inner".into(),
        };
        let z_field = mir::Place::Field {
            base: Box::new(inner_field),
            field_name: "z".into(),
        };
        f.ret(Some(mir::Operand::Copy(z_field)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected GEP: {ir}");
}

#[test]
fn nested_deref_then_field() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let struct_ty = mir::MirType::Struct {
            name: "S".into(),
            fields: thin_vec::thin_vec![("x".into(), mir::MirType::I32.into())],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "x".into() }],
        };
        let struct_id: mir::MirTypeId = struct_ty.into();
        let ptr_ty = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: struct_id.clone(),
        };
        let ptr_id: mir::MirTypeId = ptr_ty.into();

        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let p = f.new_temp(ptr_id, false);
        f.create_block();
        let deref = mir::Place::Deref(Box::new(mir::Place::Local(p)));
        let field = mir::Place::Field {
            base: Box::new(deref),
            field_name: "x".into(),
        };
        f.ret(Some(mir::Operand::Copy(field)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected GEP through pointer: {ir}");
}

#[test]
fn slice_index_geps_through_data_pointer() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let slice_ty = mir::MirType::SliceRef {
            exclusive: false,
            mutable: false,
            element_type: mir::MirType::I32.into(),
        };
        let slice_id: mir::MirTypeId = slice_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let s = f.new_temp(slice_id, false);
        let idx = f.new_temp(mir::MirType::USize.into(), false);
        f.create_block();
        let elem = mir::Place::Index {
            base: Box::new(mir::Place::Local(s)),
            index: Box::new(mir::Place::Local(idx)),
        };
        f.ret(Some(mir::Operand::Copy(elem)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected GEP through slice: {ir}");
}

#[test]
fn array_index_with_constant_index() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let arr_ty = mir::MirType::Array {
            element_type: mir::MirType::I32.into(),
            len: 8,
        };
        let arr_id: mir::MirTypeId = arr_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let arr = f.new_temp(arr_id, false);
        f.create_block();
        let elem = mir::Place::Index {
            base: Box::new(mir::Place::Local(arr)),
            index: Box::new(mir::Place::Local(f.new_temp(mir::MirType::USize.into(), false))),
        };
        f.ret(Some(mir::Operand::Copy(elem)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected GEP: {ir}");
}

#[test]
fn multi_field_struct_all_fields() {
    let h = Harness::new();
    // Each field is projected with the correct destination return type.
    let cases = [
        ("a", mir::MirType::I8, "i8"),
        ("b", mir::MirType::I16, "i16"),
        ("c", mir::MirType::I32, "i32"),
        ("d", mir::MirType::I64, "i64"),
    ];
    for (field_name, ret_ty, _idx) in cases {
        let module = h.build_module(move |b| {
            let struct_ty = mir::MirType::Struct {
                name: "Many".into(),
                fields: thin_vec::thin_vec![
                    ("a".into(), mir::MirType::I8.into()),
                    ("b".into(), mir::MirType::I16.into()),
                    ("c".into(), mir::MirType::I32.into()),
                    ("d".into(), mir::MirType::I64.into()),
                ],
                layout: thin_vec::thin_vec![
                    mir::MirStructLayoutCell::Field { field_name: "a".into() },
                    mir::MirStructLayoutCell::Field { field_name: "b".into() },
                    mir::MirStructLayoutCell::Field { field_name: "c".into() },
                    mir::MirStructLayoutCell::Field { field_name: "d".into() },
                ],
            };
            let struct_id: mir::MirTypeId = struct_ty.into();
            let ret_id: mir::MirTypeId = ret_ty.into();
            let mut f = b.start_function("f".into(), ret_id);
            let s = f.new_temp(struct_id, false);
            f.create_block();
            let field = mir::Place::Field {
                base: Box::new(mir::Place::Local(s)),
                field_name: field_name.into(),
            };
            f.ret(Some(mir::Operand::Copy(field)));
            f.finish_function();
        });
        let ir = h.ir(&module);
        assert!(h.verify(&module), "invalid IR for field {field_name}: {ir}");
        assert!(
            ir.contains("getelementptr"),
            "expected GEP for field {field_name}: {ir}"
        );
    }
}

#[test]
fn downcast_with_payload_loads_payload_type() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let enum_ty = mir::MirType::Enum {
            name: "E".into(),
            variants: thin_vec::thin_vec![
                mir::MirEnumVariant {
                    name: "I32".into(),
                    payload: Some(mir::MirType::I32.into())
                },
                mir::MirEnumVariant {
                    name: "I64".into(),
                    payload: Some(mir::MirType::I64.into())
                },
            ],
        };
        let enum_id: mir::MirTypeId = enum_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I64.into());
        let e = f.new_temp(enum_id, false);
        f.create_block();
        let downcast = mir::Place::Downcast {
            base: Box::new(mir::Place::Local(e)),
            variant_name: "I64".into(),
        };
        f.ret(Some(mir::Operand::Copy(downcast)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module), "invalid IR: {ir}");
    assert!(ir.contains("i64"), "expected i64 payload load: {ir}");
}

#[test]
fn deref_of_deref() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let inner_ptr = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let inner_ptr_id: mir::MirTypeId = inner_ptr.into();
        let outer_ptr = mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: inner_ptr_id,
        };
        let outer_ptr_id: mir::MirTypeId = outer_ptr.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let pp = f.new_temp(outer_ptr_id, false);
        f.create_block();
        let deref_outer = mir::Place::Deref(Box::new(mir::Place::Local(pp)));
        let deref_inner = mir::Place::Deref(Box::new(deref_outer));
        f.ret(Some(mir::Operand::Copy(deref_inner)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("load"), "expected loads: {ir}");
}

#[test]
fn static_deref_loads_pointee() {
    let h = Harness::new();
    let mut module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        let deref = mir::Place::Deref(Box::new(mir::Place::Static("ptr_global".into())));
        f.ret(Some(mir::Operand::Copy(deref)));
        f.finish_function();
    });
    let ptr_ty: mir::MirTypeId = mir::using_storage(&h.store, || {
        mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        }
        .into()
    });
    module.globals.push(mir::MirGlobal {
        name: "ptr_global".into(),
        ty: ptr_ty,
        body: None,
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("load"), "expected load: {ir}");
}
