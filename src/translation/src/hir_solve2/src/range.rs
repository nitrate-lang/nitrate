use nitrate_hir::{
    StructDef, StructDefId, StructField, StructMemoryLayoutCell, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::{BTreeMap, BTreeSet};
use thin_vec::ThinVec;

// Range struct names could conflict with user-defined types. If a user defines
// a struct with the same name (e.g., "Range"), the user definition wins silently
// (ensure_range_structs skips if the name already exists in the symbol table).
const RANGE_NAMES: [&str; 6] = [
    "Range",
    "RangeInclusive",
    "RangeFrom",
    "RangeTo",
    "RangeToInclusive",
    "RangeFull",
];

pub(crate) fn range_struct_name(has_start: bool, has_end: bool, inclusive: bool) -> &'static str {
    match (has_start, has_end) {
        (true, true) if inclusive => "RangeInclusive",
        (true, true) => "Range",
        (true, false) => "RangeFrom",
        (false, true) if inclusive => "RangeToInclusive",
        (false, true) => "RangeTo",
        (false, false) => "RangeFull",
    }
}

/// Ensures that synthetic range struct definitions exist in the symbol table.
/// This is idempotent — if a struct with the same name already exists (including
/// a user-defined one), it is silently kept and no synthetic definition is created.
pub(crate) fn ensure_range_structs(tab: &mut SymbolTab) {
    let generic_t: NString = NString::from("T");
    for name_str in RANGE_NAMES {
        let name: NString = NString::from(name_str);
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
        for (fn_str, ft) in &field_vec {
            let fname: NString = NString::from(*fn_str);
            fields.insert(
                fname.clone(),
                StructField {
                    span: ByteSpan::default(),
                    visibility: nitrate_hir::Visibility::Pub,
                    attributes: BTreeSet::new(),
                    name: fname.clone(),
                    ty: *ft,
                    default_value: None,
                },
            );
            layout.push(StructMemoryLayoutCell::Field { field_name: fname });
        }
        let generics = if has_generics {
            let mut g = BTreeMap::new();
            g.insert(generic_t.clone(), Some(gen_ty_id));
            Some(g)
        } else {
            None
        };
        let sd = StructDef {
            span: ByteSpan::default(),
            visibility: nitrate_hir::Visibility::Pub,
            name,
            attributes: BTreeSet::new(),
            fields,
            generics,
            layout,
        };
        tab.add_struct(StructDefId::from(sd));
    }
}

pub(crate) fn make_range_struct_object(
    tab: &SymbolTab,
    span: ByteSpan,
    start: Option<ValueId>,
    end: Option<ValueId>,
    inclusive: bool,
    has_start: bool,
    has_end: bool,
) -> Value {
    let struct_name = range_struct_name(has_start, has_end, inclusive);
    let name_ns: NString = NString::from(struct_name);
    let struct_def = match tab.get_struct(&name_ns) {
        Some(sd) => sd.clone(),
        None => {
            // Fallback: the range struct wasn't found. This can happen if
            // ensure_range_structs was not called, or if a user-defined struct
            // shadowed the compiler-internal one. In this case, emit a tuple
            // of the range endpoints so that the range expression can still
            // be type-checked.
            let mut elements = Vec::new();
            if let Some(s) = start {
                elements.push(s);
            }
            if let Some(e) = end {
                elements.push(e);
            }
            return Value::Tuple {
                span,
                elements: elements.into(),
            };
        }
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
