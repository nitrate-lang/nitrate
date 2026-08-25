//! Additional MIR type → LLVM type translation coverage: nested and compound
//! layouts beyond the basic mappings in `typegen.rs`.

use crate::test_common::Harness;
use crate::ty::{TypegenCtx, gen_ty};
use nitrate_mir::prelude as mir;

fn type_string(harness: &Harness, make_ty: impl FnOnce() -> mir::MirType) -> String {
    mir::using_storage(&harness.store, || {
        let ty = make_ty();
        let module = harness.llvm.create_module("t");
        let mut tctx = TypegenCtx {
            llvm: &harness.llvm,
            module: &module,
        };
        gen_ty(&ty, &mut tctx).print_to_string().to_string()
    })
}

#[test]
fn array_of_structs() {
    let h = Harness::new();
    let s = type_string(&h, || {
        let inner = mir::MirType::Struct {
            name: "Inner".into(),
            fields: thin_vec::thin_vec![("x".into(), mir::MirType::I32.into())],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "x".into() }],
        };
        mir::MirType::Array {
            element_type: inner.into(),
            len: 3,
        }
    });
    assert!(s.contains("[3 x %Inner]"), "unexpected: {s}");
}

#[test]
fn array_of_array() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Array {
        element_type: mir::MirType::Array {
            element_type: mir::MirType::Array {
                element_type: mir::MirType::U8.into(),
                len: 2,
            }
            .into(),
            len: 3,
        }
        .into(),
        len: 4,
    });
    assert!(s.contains("[4 x [3 x [2 x i8]]]"), "unexpected: {s}");
}

#[test]
fn tuple_of_tuple() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Tuple {
        element_types: thin_vec::thin_vec![
            mir::MirType::Tuple {
                element_types: thin_vec::thin_vec![mir::MirType::I8.into(), mir::MirType::I8.into()],
            }
            .into(),
            mir::MirType::I32.into(),
        ],
    });
    assert!(s.contains("{ { i8, i8 }, i32 }"), "unexpected: {s}");
}

#[test]
fn struct_with_nested_struct_field() {
    let h = Harness::new();
    let s = type_string(&h, || {
        let inner = mir::MirType::Struct {
            name: "Inner".into(),
            fields: thin_vec::thin_vec![("a".into(), mir::MirType::I16.into())],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "a".into() }],
        };
        let inner_id: mir::MirTypeId = inner.into();
        mir::MirType::Struct {
            name: "Outer".into(),
            fields: thin_vec::thin_vec![("inner".into(), inner_id)],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field {
                field_name: "inner".into()
            }],
        }
    });
    assert!(s.contains("%Outer = type { %Inner }"), "unexpected: {s}");
}

#[test]
fn enum_payload_zero_variants() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Enum {
        name: "Empty".into(),
        variants: thin_vec::ThinVec::new(),
    });
    assert!(s.contains("i8"), "tag should default to i8: {s}");
}

#[test]
fn enum_with_unit_payload_variants() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Enum {
        name: "UnitOnly".into(),
        variants: thin_vec::thin_vec![
            mir::MirEnumVariant {
                name: "A".into(),
                payload: None
            },
            mir::MirEnumVariant {
                name: "B".into(),
                payload: None
            },
        ],
    });
    assert!(s.ends_with(", i8 }"), "unexpected: {s}");
}

#[test]
fn enum_payload_alignment_rounds_up() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Enum {
        name: "Align".into(),
        variants: thin_vec::thin_vec![
            mir::MirEnumVariant {
                name: "I8".into(),
                payload: Some(mir::MirType::I8.into())
            },
            mir::MirEnumVariant {
                name: "I64".into(),
                payload: Some(mir::MirType::I64.into())
            },
        ],
    });
    assert!(s.contains("[8 x i8]"), "payload should round to 8 bytes: {s}");
}

#[test]
fn enum_payload_uses_largest_aligned() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Enum {
        name: "Mixed".into(),
        variants: thin_vec::thin_vec![
            mir::MirEnumVariant {
                name: "Small".into(),
                payload: Some(mir::MirType::I16.into())
            },
            mir::MirEnumVariant {
                name: "Big".into(),
                payload: Some(mir::MirType::I128.into())
            },
        ],
    });
    assert!(s.contains("[16 x i8]"), "expected 16-byte payload: {s}");
}

#[test]
fn many_variants_use_i16_tag() {
    let h = Harness::new();
    let s = type_string(&h, || {
        let mut variants = thin_vec::ThinVec::new();
        for i in 0..300usize {
            variants.push(mir::MirEnumVariant {
                name: format!("V{i}").into(),
                payload: None,
            });
        }
        mir::MirType::Enum {
            name: "Many".into(),
            variants,
        }
    });
    assert!(s.ends_with(", i16 }"), "300 variants should use i16 tag: {s}");
}

#[test]
fn slice_ref_fat_pointer_with_64_elem() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::SliceRef {
        exclusive: false,
        mutable: false,
        element_type: mir::MirType::I64.into(),
    });
    let expected = if h.llvm.ptr_size() == 8 {
        "{ ptr, i64 }"
    } else {
        "{ ptr, i32 }"
    };
    assert_eq!(s, expected);
}

#[test]
fn slice_ptr_fat_pointer_with_ptr_elem() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::SlicePtr {
        exclusive: false,
        mutable: false,
        element_type: mir::MirType::Pointer {
            exclusive: false,
            mutable: false,
            to: mir::MirType::I8.into(),
        }
        .into(),
    });
    let expected = if h.llvm.ptr_size() == 8 {
        "{ ptr, i64 }"
    } else {
        "{ ptr, i32 }"
    };
    assert_eq!(s, expected);
}

#[test]
fn function_pointer_type_maps_to_ptr() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Function {
        params: thin_vec::thin_vec![("a".into(), mir::MirType::I32.into())],
        return_type: mir::MirType::I64.into(),
        is_c_variadic: false,
    });
    assert_eq!(s, "ptr");
}

#[test]
fn struct_cache_reuses_identical_name() {
    let h = Harness::new();
    mir::using_storage(&h.store, || {
        let module = h.llvm.create_module("t");
        let mut tctx = TypegenCtx {
            llvm: &h.llvm,
            module: &module,
        };
        let make = || mir::MirType::Struct {
            name: "Cached".into(),
            fields: thin_vec::thin_vec![("f".into(), mir::MirType::I32.into())],
            layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "f".into() }],
        };
        let a = gen_ty(&make(), &mut tctx);
        let b = gen_ty(&make(), &mut tctx);
        assert_eq!(a, b);
    });
}

#[test]
fn enum_cache_reuses_identical_name() {
    let h = Harness::new();
    mir::using_storage(&h.store, || {
        let module = h.llvm.create_module("t");
        let mut tctx = TypegenCtx {
            llvm: &h.llvm,
            module: &module,
        };
        let make = || mir::MirType::Enum {
            name: "ECached".into(),
            variants: thin_vec::thin_vec![
                mir::MirEnumVariant {
                    name: "A".into(),
                    payload: Some(mir::MirType::I32.into())
                },
                mir::MirEnumVariant {
                    name: "B".into(),
                    payload: None
                },
            ],
        };
        let a = gen_ty(&make(), &mut tctx);
        let b = gen_ty(&make(), &mut tctx);
        assert_eq!(a, b);
    });
}

#[test]
fn range_is_empty_struct() {
    let h = Harness::new();
    assert_eq!(type_string(&h, || mir::MirType::Range), "{}");
}

#[test]
fn never_is_empty_struct() {
    let h = Harness::new();
    assert_eq!(type_string(&h, || mir::MirType::Never), "{}");
}

#[test]
fn empty_array_zero_len() {
    let h = Harness::new();
    let s = type_string(&h, || mir::MirType::Array {
        element_type: mir::MirType::I32.into(),
        len: 0,
    });
    assert_eq!(s, "[0 x i32]");
}
