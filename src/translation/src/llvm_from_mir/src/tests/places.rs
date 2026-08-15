//! Tests for place lowering: locals, statics, deref, field, and index.

use crate::test_common::Harness;
use nitrate_mir::prelude as mir;

#[test]
fn local_place_reads_back_assigned_value() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let x = f.new_temp(mir::MirType::I32.into(), false);
        f.create_block();
        f.push_assign(
            mir::Place::Local(x.clone()),
            mir::Rvalue::Use(mir::Operand::Constant(mir::MirLiteral::I32(7))),
        );
        f.ret(Some(mir::Operand::Copy(mir::Place::Local(x))));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("store i32 7"), "expected store: {ir}");
    // Without optimization, the SSA local is stored, then loaded back.
    assert!(ir.contains("load i32"), "expected load: {ir}");
    assert!(ir.contains("ret i32 %load"), "expected load + ret: {ir}");
}

#[test]
fn field_access_geps_into_struct() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let struct_ty = mir::MirType::Struct {
            name: "Point".into(),
            fields: thin_vec::thin_vec![
                ("x".into(), mir::MirType::I32.into()),
                ("y".into(), mir::MirType::I32.into()),
            ],
            layout: thin_vec::thin_vec![
                mir::MirStructLayoutCell::Field { field_name: "x".into() },
                mir::MirStructLayoutCell::Field { field_name: "y".into() },
            ],
        };
        let struct_id: mir::MirTypeId = struct_ty.into();

        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let p = f.new_temp(struct_id, false);
        f.create_block();
        let x_place = mir::Place::Field {
            base: Box::new(mir::Place::Local(p.clone())),
            field_name: "x".into(),
        };
        f.ret(Some(mir::Operand::Copy(x_place)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected GEP for field: {ir}");
}

#[test]
fn array_index_geps_by_element() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let arr_ty = mir::MirType::Array {
            element_type: mir::MirType::I32.into(),
            len: 4,
        };
        let arr_id: mir::MirTypeId = arr_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let arr = f.new_temp(arr_id, false);
        let idx = f.new_temp(mir::MirType::USize.into(), false);
        f.create_block();
        let elem = mir::Place::Index {
            base: Box::new(mir::Place::Local(arr)),
            index: Box::new(mir::Place::Local(idx)),
        };
        f.ret(Some(mir::Operand::Copy(elem)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("getelementptr"), "expected GEP for index: {ir}");
}

#[test]
fn deref_place_returns_pointer_value() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let ref_ty = mir::MirType::Reference {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I32.into(),
        };
        let ref_id: mir::MirTypeId = ref_ty.into();
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        let r = f.new_temp(ref_id, false);
        f.create_block();
        let deref = mir::Place::Deref(Box::new(mir::Place::Local(r)));
        f.ret(Some(mir::Operand::Copy(deref)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("load"), "expected load through reference: {ir}");
}

#[test]
fn static_global_read_loads_global() {
    let h = Harness::new();
    let mut module = h.build_module(|b| {
        let mut f = b.start_function("f".into(), mir::MirType::I32.into());
        f.create_block();
        f.ret(Some(mir::Operand::Copy(mir::Place::Static("g".into()))));
        f.finish_function();
    });
    let global_ty: mir::MirTypeId = mir::using_storage(&h.store, || mir::MirType::I32.into());
    module.globals.push(mir::MirGlobal {
        name: "g".into(),
        ty: global_ty,
        body: None,
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("@g"), "expected global: {ir}");
}

#[test]
fn downcast_returns_base_pointer() {
    let h = Harness::new();
    let module = h.build_module(|b| {
        let enum_ty = mir::MirType::Enum {
            name: "E".into(),
            variants: thin_vec::thin_vec![
                mir::MirEnumVariant {
                    name: "A".into(),
                    payload: Some(mir::MirType::I32.into())
                },
                mir::MirEnumVariant {
                    name: "B".into(),
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
            variant_name: "A".into(),
        };
        f.ret(Some(mir::Operand::Copy(downcast)));
        f.finish_function();
    });
    let ir = h.ir(&module);
    assert!(h.verify(&module));
    assert!(ir.contains("ret"), "expected downcast to lower: {ir}");
}
