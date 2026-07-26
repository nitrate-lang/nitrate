use nitrate_hir::{FunctionType, Type, TypeId};
use nitrate_nstring::NString;
use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(crate) enum TypeConstraint {
    Equal(TypeId),
}

pub(crate) enum NodeAction {
    NoChange,
    Replace(nitrate_hir::Value),
}

/// A substitution maps generic parameter indices (or inference variable IDs) to concrete types.
#[derive(Debug, Clone, Default)]
pub(crate) struct Substitution {
    /// Maps inference variable IDs or generic param indices to concrete types
    pub mapping: HashMap<u32, TypeId>,
}

impl Substitution {
    pub fn apply(&self, ty: &Type) -> Type {
        match ty {
            Type::GenericParam { index, .. } => {
                if let Some(concrete) = self.mapping.get(index) {
                    (&**concrete).clone()
                } else {
                    ty.clone()
                }
            }
            Type::Inferred { id, .. } => {
                if let Some(concrete) = self.mapping.get(&id.get()) {
                    (&**concrete).clone()
                } else {
                    ty.clone()
                }
            }
            Type::Struct { .. } => {
                // Return struct types as-is (monomorphized structs are handled elsewhere)
                ty.clone()
            }
            Type::Parameterized { base, .. } => {
                // Resolve parameterized type by applying substitution to base
                self.apply(base)
            }
            Type::Array { element_type, len } => {
                let new_elem = self.apply(element_type);
                Type::Array {
                    element_type: TypeId::from(new_elem),
                    len: *len,
                }
            }
            Type::Tuple { element_types } => {
                let new_elements: Vec<TypeId> = element_types.iter().map(|et| TypeId::from(self.apply(et))).collect();
                Type::Tuple {
                    element_types: new_elements.into(),
                }
            }
            Type::Function { function_type } => {
                let new_params: Vec<(NString, TypeId)> = function_type
                    .params
                    .iter()
                    .map(|(n, p)| (n.clone(), TypeId::from(self.apply(p))))
                    .collect();
                let new_ret = self.apply(&function_type.return_type);
                Type::Function {
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
            } => {
                let new_to = self.apply(to);
                Type::Reference {
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    to: TypeId::from(new_to),
                }
            }
            Type::Pointer { exclusive, mutable, to } => {
                let new_to = self.apply(to);
                Type::Pointer {
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
            } => {
                let new_elem = self.apply(element_type);
                Type::SliceRef {
                    lifetime: lifetime.clone(),
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: TypeId::from(new_elem),
                }
            }
            Type::SlicePtr {
                exclusive,
                mutable,
                element_type,
            } => {
                let new_elem = self.apply(element_type);
                Type::SlicePtr {
                    exclusive: *exclusive,
                    mutable: *mutable,
                    element_type: TypeId::from(new_elem),
                }
            }
            Type::Refine { base, min, max } => {
                let new_base = self.apply(base);
                Type::Refine {
                    base: TypeId::from(new_base),
                    min: *min,
                    max: *max,
                }
            }
            Type::TypeAlias { def } => {
                let type_alias = def.borrow();
                self.apply(&type_alias.type_id)
            }
            _ => ty.clone(),
        }
    }
}
