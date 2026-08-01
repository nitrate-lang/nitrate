use nitrate_hir::{FunctionType, Type, TypeId};
use nitrate_nstring::NString;
use std::collections::BTreeMap;

#[derive(Debug, Clone, Default)]
pub(crate) struct Substitution {
    pub mapping: BTreeMap<u32, TypeId>,
}

impl Substitution {
    pub fn apply(&self, ty: &Type) -> Type {
        match ty {
            Type::GenericParam { index, .. } => {
                if let Some(concrete) = self.mapping.get(index) {
                    (**concrete).clone()
                } else {
                    ty.clone()
                }
            }
            Type::Inferred { id, .. } => {
                if let Some(concrete) = self.mapping.get(&id.get()) {
                    (**concrete).clone()
                } else {
                    ty.clone()
                }
            }
            Type::Struct { .. } => ty.clone(),
            Type::Parameterized { base, .. } => self.apply(base),
            Type::Array { element_type, len, .. } => {
                let new_elem = self.apply(element_type);
                Type::Array {
                    span: ty.span(),
                    element_type: TypeId::from(new_elem),
                    len: *len,
                }
            }
            Type::Tuple { element_types, .. } => {
                let new_elements: Vec<TypeId> = element_types.iter().map(|et| TypeId::from(self.apply(et))).collect();
                Type::Tuple {
                    span: ty.span(),
                    element_types: new_elements.into(),
                }
            }
            Type::Function { function_type, .. } => {
                let new_params: Vec<(NString, TypeId)> = function_type
                    .params
                    .iter()
                    .map(|(n, p)| (n.clone(), TypeId::from(self.apply(p))))
                    .collect();
                let new_ret = self.apply(&function_type.return_type);
                Type::Function {
                    span: ty.span(),
                    function_type: Box::new(FunctionType {
                        attributes: function_type.attributes.clone(),
                        params: new_params.into(),
                        return_type: TypeId::from(new_ret),
                    }),
                }
            }
            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
                ..
            } => {
                let new_to = self.apply(to);
                Type::Reference {
                    span: ty.span(),
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: TypeId::from(new_to),
                }
            }
            Type::Pointer {
                lifetime,
                exclusive,
                mutable,
                to,
                ..
            } => {
                let new_to = self.apply(to);
                Type::Pointer {
                    span: ty.span(),
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: TypeId::from(new_to),
                }
            }
            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
                ..
            } => {
                let new_elem = self.apply(element_type);
                Type::SliceRef {
                    span: ty.span(),
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: TypeId::from(new_elem),
                }
            }
            Type::SlicePtr {
                lifetime,
                exclusive,
                mutable,
                element_type,
                ..
            } => {
                let new_elem = self.apply(element_type);
                Type::SlicePtr {
                    span: ty.span(),
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: TypeId::from(new_elem),
                }
            }
            Type::TraitObject { bounds, .. } => Type::TraitObject {
                span: ty.span(),
                bounds: bounds.clone(),
            },
            Type::Refine { base, min, max, .. } => {
                let new_base = self.apply(base);
                Type::Refine {
                    span: ty.span(),
                    base: TypeId::from(new_base),
                    min: *min,
                    max: *max,
                }
            }
            Type::TypeAlias { def, .. } => {
                let type_alias = def.borrow();
                self.apply(&type_alias.type_id)
            }
            Type::Never { .. }
            | Type::Unit { .. }
            | Type::Bool { .. }
            | Type::U8 { .. }
            | Type::U16 { .. }
            | Type::U32 { .. }
            | Type::U64 { .. }
            | Type::U128 { .. }
            | Type::USize { .. }
            | Type::I8 { .. }
            | Type::I16 { .. }
            | Type::I32 { .. }
            | Type::I64 { .. }
            | Type::I128 { .. }
            | Type::F32 { .. }
            | Type::F64 { .. }
            | Type::InferredInteger { .. }
            | Type::InferredFloat { .. }
            | Type::Enum { .. } => ty.clone(),
        }
    }
}
