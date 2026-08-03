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
