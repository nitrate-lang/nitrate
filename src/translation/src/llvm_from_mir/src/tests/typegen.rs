//! Tests for MIR → LLVM type translation (`gen_ty`, `gen_fn_ret_ty`).

use crate::test_common::Harness;
use crate::ty::{TypegenCtx, gen_fn_ret_ty, gen_ty};
use nitrate_mir::prelude as mir;

/// Lower a `MirType` to an LLVM type string using a fresh module.
fn type_string(harness: &Harness, ty: &mir::MirType) -> String {
    let module = harness.llvm.create_module("t");
    let mut tctx = TypegenCtx {
        llvm: &harness.llvm,
        module: &module,
    };
    gen_ty(ty, &mut tctx).print_to_string().to_string()
}

fn ret_type_string(harness: &Harness, ty: &mir::MirType) -> String {
    let module = harness.llvm.create_module("t");
    let mut tctx = TypegenCtx {
        llvm: &harness.llvm,
        module: &module,
    };
    gen_fn_ret_ty(ty, &mut tctx)
        .map(|t| t.print_to_string().to_string())
        .unwrap_or_else(|| "void".to_string())
}

#[test]
fn unit_maps_to_empty_struct() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::Unit), "{}");
}

#[test]
fn never_maps_to_empty_struct() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::Never), "{}");
}

#[test]
fn bool_maps_to_i1() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::Bool), "i1");
}

#[test]
fn unsigned_ints_map_to_llvm_ints() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::U8), "i8");
    assert_eq!(type_string(&h, &mir::MirType::U16), "i16");
    assert_eq!(type_string(&h, &mir::MirType::U32), "i32");
    assert_eq!(type_string(&h, &mir::MirType::U64), "i64");
    assert_eq!(type_string(&h, &mir::MirType::U128), "i128");
}

#[test]
fn signed_ints_map_to_llvm_ints() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::I8), "i8");
    assert_eq!(type_string(&h, &mir::MirType::I16), "i16");
    assert_eq!(type_string(&h, &mir::MirType::I32), "i32");
    assert_eq!(type_string(&h, &mir::MirType::I64), "i64");
    assert_eq!(type_string(&h, &mir::MirType::I128), "i128");
}

#[test]
fn usize_maps_to_pointer_sized_int() {
    let h = Harness::new();
    let expected = if h.llvm.ptr_size() == 8 { "i64" } else { "i32" };
    assert_eq!(type_string(&h, &mir::MirType::USize), expected);
}

#[test]
fn floats_map_to_float_and_double() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::F32), "float");
    assert_eq!(type_string(&h, &mir::MirType::F64), "double");
}

#[test]
fn str_maps_to_ptr() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::Str), "ptr");
}

#[test]
fn range_maps_to_empty_struct() {
    let h = Harness::new();
    assert_eq!(type_string(&h, &mir::MirType::Range), "{}");
}

#[test]
fn reference_maps_to_ptr() {
    let h = Harness::new();
    let ty = mir::MirType::Reference {
        exclusive: false,
        mutable: false,
        to: mir::MirType::I32.into(),
    };
    assert_eq!(type_string(&h, &ty), "ptr");
}

#[test]
fn pointer_maps_to_ptr() {
    let h = Harness::new();
    let ty = mir::MirType::Pointer {
        exclusive: false,
        mutable: true,
        to: mir::MirType::I32.into(),
    };
    assert_eq!(type_string(&h, &ty), "ptr");
}

#[test]
fn function_maps_to_ptr() {
    let h = Harness::new();
    let ty = mir::MirType::Function {
        params: Default::default(),
        return_type: mir::MirType::Unit.into(),
        is_c_variadic: false,
    };
    assert_eq!(type_string(&h, &ty), "ptr");
}

#[test]
fn slice_ref_maps_to_fat_pointer() {
    let h = Harness::new();
    let ty = mir::MirType::SliceRef {
        exclusive: false,
        mutable: false,
        element_type: mir::MirType::I8.into(),
    };
    let expected = if h.llvm.ptr_size() == 8 {
        "{ ptr, i64 }"
    } else {
        "{ ptr, i32 }"
    };
    assert_eq!(type_string(&h, &ty), expected);
}

#[test]
fn slice_ptr_maps_to_fat_pointer() {
    let h = Harness::new();
    let ty = mir::MirType::SlicePtr {
        exclusive: false,
        mutable: false,
        element_type: mir::MirType::I8.into(),
    };
    let expected = if h.llvm.ptr_size() == 8 {
        "{ ptr, i64 }"
    } else {
        "{ ptr, i32 }"
    };
    assert_eq!(type_string(&h, &ty), expected);
}

#[test]
fn tuple_maps_to_struct() {
    let h = Harness::new();
    let ty = mir::MirType::Tuple {
        element_types: thin_vec::thin_vec![mir::MirType::I32.into(), mir::MirType::F64.into()],
    };
    assert_eq!(type_string(&h, &ty), "{ i32, double }");
}

#[test]
fn empty_tuple_maps_to_empty_struct() {
    let h = Harness::new();
    let ty = mir::MirType::Tuple {
        element_types: Default::default(),
    };
    assert_eq!(type_string(&h, &ty), "{}");
}

#[test]
fn array_maps_to_llvm_array() {
    let h = Harness::new();
    let ty = mir::MirType::Array {
        element_type: mir::MirType::I32.into(),
        len: 4,
    };
    assert_eq!(type_string(&h, &ty), "[4 x i32]");
}

#[test]
fn nested_array_maps_recursively() {
    let h = Harness::new();
    let ty = mir::MirType::Array {
        element_type: mir::MirType::Array {
            element_type: mir::MirType::I8.into(),
            len: 2,
        }
        .into(),
        len: 3,
    };
    assert_eq!(type_string(&h, &ty), "[3 x [2 x i8]]");
}

#[test]
fn struct_maps_to_named_struct_with_fields() {
    let h = Harness::new();
    let ty = mir::MirType::Struct {
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
    assert_eq!(type_string(&h, &ty), "%Point = type { i32, i32 }");
}

#[test]
fn struct_maps_padding_to_byte_arrays() {
    let h = Harness::new();
    let ty = mir::MirType::Struct {
        name: "Padded".into(),
        fields: thin_vec::thin_vec![("a".into(), mir::MirType::I8.into())],
        layout: thin_vec::thin_vec![
            mir::MirStructLayoutCell::Field { field_name: "a".into() },
            mir::MirStructLayoutCell::Padding(std::num::NonZeroU32::new(3).unwrap()),
        ],
    };
    assert_eq!(type_string(&h, &ty), "%Padded = type { i8, [3 x i8] }");
}

#[test]
fn struct_type_is_cached_by_name() {
    let h = Harness::new();
    let module = h.llvm.create_module("t");
    let mut tctx = TypegenCtx {
        llvm: &h.llvm,
        module: &module,
    };
    let ty = mir::MirType::Struct {
        name: "S".into(),
        fields: thin_vec::thin_vec![("f".into(), mir::MirType::I32.into())],
        layout: thin_vec::thin_vec![mir::MirStructLayoutCell::Field { field_name: "f".into() }],
    };
    let a = gen_ty(&ty, &mut tctx);
    let b = gen_ty(&ty, &mut tctx);
    assert_eq!(a, b);
}

#[test]
fn enum_uses_tag_width_for_variant_count() {
    let h = Harness::new();

    // <= 256 variants -> i8 tag
    let ty8 = mir::MirType::Enum {
        name: "E8".into(),
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
    };
    assert!(type_string(&h, &ty8).ends_with(", i8 }"));

    // 257..=65536 variants -> i16 tag
    let mut variants = thin_vec::ThinVec::new();
    for i in 0..300usize {
        variants.push(mir::MirEnumVariant {
            name: format!("V{i}").into(),
            payload: None,
        });
    }
    let ty16 = mir::MirType::Enum {
        name: "E16".into(),
        variants,
    };
    assert!(type_string(&h, &ty16).ends_with(", i16 }"));
}

#[test]
fn enum_payload_sized_to_largest_variant() {
    let h = Harness::new();
    let ty = mir::MirType::Enum {
        name: "E".into(),
        variants: thin_vec::thin_vec![
            mir::MirEnumVariant {
                name: "Unit".into(),
                payload: None
            },
            mir::MirEnumVariant {
                name: "Small".into(),
                payload: Some(mir::MirType::I8.into())
            },
            mir::MirEnumVariant {
                name: "Big".into(),
                payload: Some(mir::MirType::I64.into())
            },
        ],
    };
    // The payload array must be at least 8 bytes to fit the i64 payload.
    let s = type_string(&h, &ty);
    assert!(s.contains("[8 x i8]"), "unexpected enum layout: {s}");
}

#[test]
fn gen_fn_ret_ty_void_for_unit() {
    let h = Harness::new();
    assert_eq!(ret_type_string(&h, &mir::MirType::Unit), "void");
}

#[test]
fn gen_fn_ret_ty_void_for_never() {
    let h = Harness::new();
    assert_eq!(ret_type_string(&h, &mir::MirType::Never), "void");
}

#[test]
fn gen_fn_ret_ty_concrete_for_i32() {
    let h = Harness::new();
    assert_eq!(ret_type_string(&h, &mir::MirType::I32), "i32");
}
