use nitrate_hir::{
    StructDef, StructDefId, StructField, StructMemoryLayoutCell, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_nstring::NString;
use nitrate_tree::SrcPos;
use std::collections::{BTreeMap, BTreeSet};
use thin_vec::ThinVec;

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

pub(crate) fn ensure_range_structs(tab: &mut SymbolTab) {
    let generic_t: NString = NString::from("T");
    for name_str in RANGE_NAMES {
        let name: NString = NString::from(name_str);
        if tab.get_struct(&name).is_some() {
            continue;
        }
        let gen_ty = Type::GenericParam {
            span: SrcPos::default(),
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
                    span: SrcPos::default(),
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
            span: SrcPos::default(),
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
    span: SrcPos,
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

// ── Tests ────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use nitrate_hir::{PtrSize, Store, Value, ValueId, using_storage};
    use nitrate_nstring::NString;
    use nitrate_tree::SrcPos;

    fn sp() -> SrcPos {
        SrcPos::default()
    }

    fn new_tab() -> SymbolTab {
        SymbolTab::new(PtrSize::U64)
    }

    // ── range_struct_name ────────────────────────────────────────

    #[test]
    fn name_range() {
        assert_eq!(range_struct_name(true, true, false), "Range");
    }

    #[test]
    fn name_range_inclusive() {
        assert_eq!(range_struct_name(true, true, true), "RangeInclusive");
    }

    #[test]
    fn name_range_from() {
        assert_eq!(range_struct_name(true, false, false), "RangeFrom");
        assert_eq!(range_struct_name(true, false, true), "RangeFrom");
    }

    #[test]
    fn name_range_to() {
        assert_eq!(range_struct_name(false, true, false), "RangeTo");
    }

    #[test]
    fn name_range_to_inclusive() {
        assert_eq!(range_struct_name(false, true, true), "RangeToInclusive");
    }

    #[test]
    fn name_range_full() {
        assert_eq!(range_struct_name(false, false, false), "RangeFull");
        assert_eq!(range_struct_name(false, false, true), "RangeFull");
    }

    // ── ensure_range_structs ─────────────────────────────────────

    #[test]
    fn ensure_creates_structs() {
        with_store(|| {
            let mut tab = new_tab();
            for name_str in RANGE_NAMES {
                let name = NString::from(name_str);
                assert!(tab.get_struct(&name).is_none());
            }
            ensure_range_structs(&mut tab);
            for name_str in RANGE_NAMES {
                let name = NString::from(name_str);
                assert!(tab.get_struct(&name).is_some());
            }
        });
    }

    #[test]
    fn ensure_is_idempotent() {
        with_store(|| {
            let mut tab = new_tab();
            ensure_range_structs(&mut tab);
            ensure_range_structs(&mut tab);
            ensure_range_structs(&mut tab);
            for name_str in RANGE_NAMES {
                let name = NString::from(name_str);
                assert!(tab.get_struct(&name).is_some());
            }
        });
    }

    #[test]
    fn range_struct_has_fields() {
        with_store(|| {
            let mut tab = new_tab();
            ensure_range_structs(&mut tab);
            let sd = tab.get_struct(&NString::from("Range")).unwrap();
            let s = sd.borrow();
            assert!(s.fields.contains_key(&NString::from("start")));
            assert!(s.fields.contains_key(&NString::from("end")));
            assert!(s.generics.is_some());
        });
    }

    #[test]
    fn range_from_has_start_only() {
        with_store(|| {
            let mut tab = new_tab();
            ensure_range_structs(&mut tab);
            let sd = tab.get_struct(&NString::from("RangeFrom")).unwrap();
            let s = sd.borrow();
            assert!(s.fields.contains_key(&NString::from("start")));
            assert!(!s.fields.contains_key(&NString::from("end")));
        });
    }

    #[test]
    fn range_full_has_no_fields() {
        with_store(|| {
            let mut tab = new_tab();
            ensure_range_structs(&mut tab);
            let sd = tab.get_struct(&NString::from("RangeFull")).unwrap();
            let s = sd.borrow();
            assert!(s.fields.is_empty());
            assert!(s.generics.is_none());
        });
    }

    #[test]
    fn ensure_does_not_overwrite_user_defined() {
        with_store(|| {
            let mut tab = new_tab();
            let existing_name = NString::from("Range");
            let existing = StructDef {
                span: SrcPos::default(),
                visibility: nitrate_hir::Visibility::Sec,
                name: existing_name.clone(),
                attributes: BTreeSet::new(),
                fields: BTreeMap::new(),
                generics: None,
                layout: ThinVec::new(),
            };
            tab.add_struct(StructDefId::from(existing));
            ensure_range_structs(&mut tab);
            let sd = tab.get_struct(&existing_name).unwrap();
            let s = sd.borrow();
            assert_eq!(s.visibility, nitrate_hir::Visibility::Sec);
        });
    }

    // ── make_range_struct_object ─────────────────────────────────

    fn with_store<R>(f: impl FnOnce() -> R) -> R {
        let store = Store::new();
        using_storage(&store, f)
    }

    #[test]
    fn make_range_creates_struct_object() {
        with_store(|| {
            let mut tab = new_tab();
            ensure_range_structs(&mut tab);
            let val = make_range_struct_object(
                &tab,
                sp(),
                Some(ValueId::from(Value::I32 { span: sp(), value: 1 })),
                Some(ValueId::from(Value::I32 { span: sp(), value: 10 })),
                false,
                true,
                true,
            );
            if let Value::StructObject { struct_def, fields, .. } = val {
                assert_eq!(struct_def.borrow().name, NString::from("Range"));
                assert_eq!(fields.len(), 2);
            } else {
                panic!("expected StructObject, got {val:?}");
            }
        });
    }

    #[test]
    fn make_range_inclusive() {
        with_store(|| {
            let mut tab = new_tab();
            ensure_range_structs(&mut tab);
            let val = make_range_struct_object(
                &tab,
                sp(),
                Some(ValueId::from(Value::U8 { span: sp(), value: 5 })),
                Some(ValueId::from(Value::U8 { span: sp(), value: 10 })),
                true,
                true,
                true,
            );
            if let Value::StructObject { struct_def, .. } = val {
                assert_eq!(struct_def.borrow().name, NString::from("RangeInclusive"));
            } else {
                panic!("expected StructObject");
            }
        });
    }

    #[test]
    fn make_range_fallback_to_tuple_when_not_registered() {
        with_store(|| {
            let tab = new_tab(); // no range structs registered
            let val = make_range_struct_object(
                &tab,
                sp(),
                Some(ValueId::from(Value::I32 { span: sp(), value: 1 })),
                Some(ValueId::from(Value::I32 { span: sp(), value: 2 })),
                false,
                true,
                true,
            );
            if let Value::Tuple { elements, .. } = val {
                assert_eq!(elements.len(), 2);
            } else {
                panic!("expected Tuple fallback, got {val:?}");
            }
        });
    }
}
