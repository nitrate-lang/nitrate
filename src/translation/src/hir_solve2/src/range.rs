//! Range expression desugaring.
//!
//! Range expressions (`a..b`, `a..=b`, `a..`, `..b`, `..=b`, `..`) are
//! desugared into construction of standard struct types that mirror Rust's
//! `Range`, `RangeInclusive`, `RangeFrom`, `RangeTo`, `RangeToInclusive`,
//! and `RangeFull`. The structs are registered in the SymbolTab as built-in
//! types so that type inference, codegen, and other passes can refer to them.

use nitrate_hir::{
    StructDef, StructDefId, StructField, StructMemoryLayoutCell, SymbolTab, Type, TypeId, Value, ValueId, Visibility,
};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::{BTreeMap, BTreeSet};
use thin_vec::ThinVec;

/// Names of the built-in range struct types.
pub const RANGE_NAMES: [&str; 6] = [
    "Range",
    "RangeInclusive",
    "RangeFrom",
    "RangeTo",
    "RangeToInclusive",
    "RangeFull",
];

/// Map a range combination (has_start, has_end, inclusive) to the built-in
/// struct name (e.g. `"Range"`, `"RangeInclusive"`, etc.).
#[must_use]
pub fn range_struct_name(has_start: bool, has_end: bool, inclusive: bool) -> &'static str {
    match (has_start, has_end) {
        (true, true) if inclusive => "RangeInclusive",
        (true, true) => "Range",
        (true, false) => "RangeFrom",
        (false, true) if inclusive => "RangeToInclusive",
        (false, true) => "RangeTo",
        (false, false) => "RangeFull",
    }
}

/// Insert the six built-in range struct definitions into the symbol table.
///
/// Idempotent: will not insert a struct if one with the given name already
/// exists in the symbol table.
pub fn ensure_range_structs(tab: &mut SymbolTab) {
    let generic_t: NString = NString::from("T");

    for name_str in RANGE_NAMES {
        let name: NString = NString::from(name_str);

        // Skip if already registered.
        if tab.get_struct(&name).is_some() {
            continue;
        }

        let gen_ty = Type::GenericParam {
            span: ByteSpan::default(),
            index: 0,
            name: generic_t.clone(),
        };
        let gen_ty_id: TypeId = gen_ty.into();

        let (field_vec, has_generics): (Vec<(&str, TypeId)>, bool) = match name_str {
            "Range" | "RangeInclusive" => (vec![("start", gen_ty_id), ("end", gen_ty_id)], true),
            "RangeFrom" => (vec![("start", gen_ty_id)], true),
            "RangeTo" | "RangeToInclusive" => (vec![("end", gen_ty_id)], true),
            "RangeFull" => (vec![], false),
            _ => unreachable!(),
        };

        let mut fields: BTreeMap<NString, StructField> = BTreeMap::new();
        let mut layout: ThinVec<StructMemoryLayoutCell> = ThinVec::new();

        for (field_name_str, field_ty) in &field_vec {
            let fname: NString = NString::from(*field_name_str);
            fields.insert(
                fname.clone(),
                StructField {
                    span: ByteSpan::default(),
                    visibility: Visibility::Pub,
                    attributes: BTreeSet::new(),
                    name: fname.clone(),
                    ty: *field_ty,
                    default_value: None,
                },
            );
            layout.push(StructMemoryLayoutCell::Field { field_name: fname });
        }

        let generics: Option<BTreeMap<NString, Option<TypeId>>> = if has_generics {
            let mut g = BTreeMap::new();
            g.insert(generic_t.clone(), Some(gen_ty_id));
            Some(g)
        } else {
            None
        };

        let struct_def = StructDef {
            span: ByteSpan::default(),
            visibility: Visibility::Pub,
            name,
            attributes: BTreeSet::new(),
            fields,
            generics,
            layout,
        };
        let id: StructDefId = struct_def.into();
        tab.add_struct(id);
    }
}

/// Given a range value and its start/end HIR values, resolve the appropriate
/// range struct from the symbol table and return a `StructObject` value that
/// constructs it.
pub fn make_range_struct_object(
    tab: &SymbolTab,
    span: ByteSpan,
    start: Option<ValueId>,
    end: Option<ValueId>,
    inclusive: bool,
    has_start: bool,
    has_end: bool,
) -> Value {
    let struct_name = range_struct_name(has_start, has_end, inclusive);

    let struct_def = match tab.get_struct(&NString::from(struct_name)) {
        Some(def) => def.clone(),
        None => panic!(
            "Range struct `{struct_name}` not found in symbol table. \
             Ensure `ensure_range_structs` was called before type solving."
        ),
    };

    let mut fields: ThinVec<(NString, ValueId)> = ThinVec::new();

    if let Some(s) = start {
        fields.push((NString::from("start"), s));
    }
    if let Some(e) = end {
        fields.push((NString::from("end"), e));
    }

    Value::StructObject {
        span,
        struct_def,
        fields,
    }
}
