use nitrate_hir::{Arguments, Type, TypeId};
use nitrate_nstring::NString;
use std::collections::BTreeMap;
use thin_vec::ThinVec;

#[derive(Debug, Clone)]
pub(crate) struct Substitution {
    /// Maps GenericParam indices to their concrete types.
    pub generic_mapping: BTreeMap<u32, TypeId>,
    /// Maps Inferred type IDs (via id.get()) to their concrete resolved types.
    pub inferred_mapping: BTreeMap<u32, TypeId>,
}

impl Default for Substitution {
    fn default() -> Self {
        Self {
            generic_mapping: BTreeMap::new(),
            inferred_mapping: BTreeMap::new(),
        }
    }
}

impl Substitution {
    pub fn apply(&self, ty: &Type) -> Type {
        match ty {
            Type::GenericParam { index, .. } => self
                .generic_mapping
                .get(index)
                .map(|c| (**c).clone())
                .unwrap_or_else(|| ty.clone()),
            Type::Inferred { id, .. } => self
                .inferred_mapping
                .get(&id.get())
                .map(|c| (**c).clone())
                .unwrap_or_else(|| ty.clone()),
            Type::Struct { .. } | Type::Enum { .. } => ty.clone(),
            Type::Parameterized { base, args, .. } => {
                let new_base = self.apply(base);
                let new_positional: ThinVec<TypeId> =
                    args.positional.iter().map(|a| TypeId::from(self.apply(a))).collect();
                let new_named: ThinVec<(NString, TypeId)> = args
                    .named
                    .iter()
                    .map(|(k, v)| (k.clone(), TypeId::from(self.apply(v))))
                    .collect();
                Type::Parameterized {
                    span: ty.span(),
                    base: TypeId::from(new_base),
                    args: Arguments {
                        positional: new_positional,
                        named: new_named,
                    },
                }
            }
            Type::Array { element_type, len, .. } => Type::Array {
                span: ty.span(),
                element_type: TypeId::from(self.apply(element_type)),
                len: *len,
            },
            Type::Tuple { element_types, .. } => {
                let els: Vec<TypeId> = element_types.iter().map(|et| TypeId::from(self.apply(et))).collect();
                Type::Tuple {
                    span: ty.span(),
                    element_types: els.into(),
                }
            }
            Type::Function { function_type, .. } => {
                let params: Vec<(NString, TypeId)> = function_type
                    .params
                    .iter()
                    .map(|(n, p)| (n.clone(), TypeId::from(self.apply(p))))
                    .collect();
                let ret = self.apply(&function_type.return_type);
                Type::Function {
                    span: ty.span(),
                    function_type: Box::new(nitrate_hir::FunctionType {
                        attributes: function_type.attributes.clone(),
                        params: params.into(),
                        return_type: TypeId::from(ret),
                    }),
                }
            }
            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
                ..
            } => Type::Reference {
                span: ty.span(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                to: TypeId::from(self.apply(to)),
            },
            Type::Pointer {
                lifetime,
                exclusive,
                mutable,
                to,
                ..
            } => Type::Pointer {
                span: ty.span(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                to: TypeId::from(self.apply(to)),
            },
            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
                ..
            } => Type::SliceRef {
                span: ty.span(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                element_type: TypeId::from(self.apply(element_type)),
            },
            Type::SlicePtr {
                lifetime,
                exclusive,
                mutable,
                element_type,
                ..
            } => Type::SlicePtr {
                span: ty.span(),
                lifetime: lifetime.clone(),
                exclusive: *exclusive,
                mutable: *mutable,
                element_type: TypeId::from(self.apply(element_type)),
            },
            Type::TraitObject { bounds, .. } => Type::TraitObject {
                span: ty.span(),
                bounds: bounds.clone(),
            },
            Type::Refine { base, min, max, .. } => Type::Refine {
                span: ty.span(),
                base: TypeId::from(self.apply(base)),
                min: *min,
                max: *max,
            },
            Type::TypeAlias { def, .. } => self.apply(&def.borrow().type_id),
            Type::UnresolvedArray { element_type, len, .. } => Type::UnresolvedArray {
                span: ty.span(),
                element_type: TypeId::from(self.apply(element_type)),
                len: len.clone(),
            },
            Type::UnresolvedRefine { base, min, max, .. } => Type::UnresolvedRefine {
                span: ty.span(),
                base: TypeId::from(self.apply(base)),
                min: min.clone(),
                max: max.clone(),
            },
            _ => ty.clone(),
        }
    }
}

// ── Tests ────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use nitrate_hir::{
        Arguments, FunctionType, Lifetime, Lit, LiteralId, Store, Type, TypeId, Value, ValueId, using_storage,
    };
    use nitrate_nstring::NString;
    use nitrate_tree::SrcPos;
    use std::collections::BTreeSet;
    use std::num::NonZeroU32;

    fn sp() -> SrcPos {
        SrcPos::default()
    }

    fn with_store<R>(f: impl FnOnce() -> R) -> R {
        let store = Store::new();
        using_storage(&store, f)
    }

    #[test]
    fn substitution_default_is_empty() {
        let s = Substitution::default();
        assert!(s.generic_mapping.is_empty());
        assert!(s.inferred_mapping.is_empty());
    }

    #[test]
    fn apply_generic_param_resolved() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let param = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            assert_eq!(s.apply(&param), Type::I32 { span: sp() });
        });
    }

    #[test]
    fn apply_generic_param_unresolved() {
        let s = Substitution::default();
        let param = Type::GenericParam {
            span: sp(),
            index: 1,
            name: NString::from("T"),
        };
        assert_eq!(s.apply(&param), param);
    }

    #[test]
    fn apply_inferred_resolved() {
        with_store(|| {
            let mut s = Substitution::default();
            let id = NonZeroU32::new(42).unwrap();
            s.inferred_mapping.insert(42, TypeId::from(Type::I64 { span: sp() }));
            let inferred = Type::Inferred {
                span: sp(),
                id,
                name: None,
            };
            assert_eq!(s.apply(&inferred), Type::I64 { span: sp() });
        });
    }

    #[test]
    fn apply_inferred_unresolved() {
        let s = Substitution::default();
        let id = NonZeroU32::new(1).unwrap();
        let inferred = Type::Inferred {
            span: sp(),
            id,
            name: Some(NString::from("_")),
        };
        assert_eq!(s.apply(&inferred), inferred);
    }

    #[test]
    fn apply_primitives_no_change() {
        let s = Substitution::default();
        assert_eq!(s.apply(&Type::Bool { span: sp() }), Type::Bool { span: sp() });
        assert_eq!(s.apply(&Type::Unit { span: sp() }), Type::Unit { span: sp() });
        assert_eq!(s.apply(&Type::Never { span: sp() }), Type::Never { span: sp() });
    }

    #[test]
    fn apply_array_resolves_element() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let arr = Type::Array {
                span: sp(),
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                len: 5,
            };
            let result = s.apply(&arr);
            if let Type::Array { element_type, len, .. } = result {
                assert_eq!(*element_type, Type::I32 { span: sp() });
                assert_eq!(len, 5);
            } else {
                panic!("expected Array");
            }
        });
    }

    #[test]
    fn apply_tuple_resolves_elements() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::U8 { span: sp() }));
            s.generic_mapping.insert(1, TypeId::from(Type::U16 { span: sp() }));
            let tuple = Type::Tuple {
                span: sp(),
                element_types: vec![
                    TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 0,
                        name: NString::from("T"),
                    }),
                    TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 1,
                        name: NString::from("U"),
                    }),
                ]
                .into(),
            };
            let result = s.apply(&tuple);
            if let Type::Tuple { element_types, .. } = result {
                assert_eq!(*element_types[0], Type::U8 { span: sp() });
                assert_eq!(*element_types[1], Type::U16 { span: sp() });
            } else {
                panic!("expected Tuple");
            }
        });
    }

    #[test]
    fn apply_reference_resolves_to() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let ref_ty = Type::Reference {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                to: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let result = s.apply(&ref_ty);
            if let Type::Reference { to, .. } = result {
                assert_eq!(*to, Type::I32 { span: sp() });
            } else {
                panic!("expected Reference");
            }
        });
    }

    #[test]
    fn apply_pointer_resolves_to() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::U8 { span: sp() }));
            let ptr = Type::Pointer {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: true,
                to: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let result = s.apply(&ptr);
            if let Type::Pointer { to, .. } = result {
                assert_eq!(*to, Type::U8 { span: sp() });
            } else {
                panic!("expected Pointer");
            }
        });
    }

    #[test]
    fn apply_slice_ref_resolves_element() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::F64 { span: sp() }));
            let sr = Type::SliceRef {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: false,
                mutable: false,
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let result = s.apply(&sr);
            if let Type::SliceRef { element_type, .. } = result {
                assert_eq!(*element_type, Type::F64 { span: sp() });
            } else {
                panic!("expected SliceRef");
            }
        });
    }

    #[test]
    fn apply_function_type_resolves_params_and_return() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let func = Type::Function {
                span: sp(),
                function_type: Box::new(FunctionType {
                    attributes: BTreeSet::new(),
                    params: vec![(
                        NString::from("x"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 0,
                            name: NString::from("T"),
                        }),
                    )]
                    .into(),
                    return_type: TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 0,
                        name: NString::from("T"),
                    }),
                }),
            };
            let result = s.apply(&func);
            if let Type::Function { function_type, .. } = result {
                assert_eq!(*function_type.return_type, Type::I32 { span: sp() });
                assert_eq!(*function_type.params[0].1, Type::I32 { span: sp() });
            } else {
                panic!("expected Function");
            }
        });
    }

    #[test]
    fn apply_multiple_generic_params() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I8 { span: sp() }));
            s.generic_mapping.insert(1, TypeId::from(Type::I16 { span: sp() }));
            s.generic_mapping.insert(2, TypeId::from(Type::I32 { span: sp() }));

            let t = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("A"),
            };
            let u = Type::GenericParam {
                span: sp(),
                index: 1,
                name: NString::from("B"),
            };
            let v = Type::GenericParam {
                span: sp(),
                index: 2,
                name: NString::from("C"),
            };

            assert_eq!(s.apply(&t), Type::I8 { span: sp() });
            assert_eq!(s.apply(&u), Type::I16 { span: sp() });
            assert_eq!(s.apply(&v), Type::I32 { span: sp() });
        });
    }

    #[test]
    fn generic_and_inferred_mappings_are_independent() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            s.inferred_mapping.insert(10, TypeId::from(Type::Bool { span: sp() }));

            let param = Type::GenericParam {
                span: sp(),
                index: 0,
                name: NString::from("T"),
            };
            assert_eq!(s.apply(&param), Type::I32 { span: sp() });

            let inferred = Type::Inferred {
                span: sp(),
                id: NonZeroU32::new(10).unwrap(),
                name: None,
            };
            assert_eq!(s.apply(&inferred), Type::Bool { span: sp() });
        });
    }

    // ── apply for additional Type variants ────────────────────

    #[test]
    fn apply_slice_ptr_resolves() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I16 { span: sp() }));
            let sp_ty = Type::SlicePtr {
                span: sp(),
                lifetime: Lifetime::Static,
                exclusive: true,
                mutable: true,
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
            };
            let result = s.apply(&sp_ty);
            if let Type::SlicePtr { element_type, .. } = result {
                assert_eq!(*element_type, Type::I16 { span: sp() });
            } else {
                panic!("expected SlicePtr");
            }
        });
    }

    #[test]
    fn apply_trait_object_no_change() {
        let s = Substitution::default();
        let ty = Type::TraitObject {
            span: sp(),
            bounds: vec![],
        };
        assert_eq!(s.apply(&ty), ty);
    }

    #[test]
    fn apply_parameterized_resolves() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let pt = Type::Parameterized {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                args: Arguments {
                    positional: vec![TypeId::from(Type::GenericParam {
                        span: sp(),
                        index: 0,
                        name: NString::from("T"),
                    })]
                    .into(),
                    named: vec![(
                        NString::from("X"),
                        TypeId::from(Type::GenericParam {
                            span: sp(),
                            index: 0,
                            name: NString::from("T"),
                        }),
                    )]
                    .into(),
                },
            };
            let result = s.apply(&pt);
            if let Type::Parameterized { base, args, .. } = result {
                assert_eq!(*base, Type::I32 { span: sp() });
                assert_eq!(*args.positional[0], Type::I32 { span: sp() });
                assert_eq!(*args.named[0].1, Type::I32 { span: sp() });
            } else {
                panic!("expected Parameterized");
            }
        });
    }

    #[test]
    fn apply_unresolved_array_resolves() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let dummy_value = ValueId::from(Value::I32 { span: sp(), value: 0 });
            let ua = Type::UnresolvedArray {
                span: sp(),
                element_type: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                len: dummy_value.clone(),
            };
            let result = s.apply(&ua);
            if let Type::UnresolvedArray { element_type, .. } = result {
                assert_eq!(*element_type, Type::I32 { span: sp() });
            } else {
                panic!("expected UnresolvedArray");
            }
        });
    }

    #[test]
    fn apply_unresolved_refine_resolves() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let dummy_value = ValueId::from(Value::I32 { span: sp(), value: 0 });
            let ur = Type::UnresolvedRefine {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                min: dummy_value.clone(),
                max: dummy_value,
            };
            let result = s.apply(&ur);
            if let Type::UnresolvedRefine { base, .. } = result {
                assert_eq!(*base, Type::I32 { span: sp() });
            } else {
                panic!("expected UnresolvedRefine");
            }
        });
    }

    #[test]
    fn apply_refine_resolves_base() {
        with_store(|| {
            let mut s = Substitution::default();
            s.generic_mapping.insert(0, TypeId::from(Type::I32 { span: sp() }));
            let refine = Type::Refine {
                span: sp(),
                base: TypeId::from(Type::GenericParam {
                    span: sp(),
                    index: 0,
                    name: NString::from("T"),
                }),
                min: LiteralId::from(Lit::I8(0)),
                max: LiteralId::from(Lit::I8(100)),
            };
            let result = s.apply(&refine);
            if let Type::Refine { base, .. } = result {
                assert_eq!(*base, Type::I32 { span: sp() });
            } else {
                panic!("expected Refine");
            }
        });
    }
}
