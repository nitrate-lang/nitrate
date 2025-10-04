use crate::ty::Type;
use hashbrown::HashMap;
use serde::{Deserialize, Serialize};

#[derive(Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct TypeId {
    id: u32,
}

#[derive(Default)]
pub struct TypeStore {
    types: HashMap<TypeId, Type>,
    next_id: u32,
}

impl TypeStore {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn store(&mut self, ty: Type) -> TypeId {
        let id = self.next_id;
        self.types.insert(TypeId { id }, ty);

        self.next_id = self
            .next_id
            .checked_add(1)
            .expect("TypeStore overflowed u32");

        TypeId { id }
    }

    fn get(&self, id: &TypeId) -> &Type {
        self.types.get(id).expect("TypeId not found in TypeStore")
    }

    fn get_mut(&mut self, id: &TypeId) -> &mut Type {
        self.types
            .get_mut(id)
            .expect("TypeId not found in TypeStore")
    }
}

impl std::ops::Index<&TypeId> for TypeStore {
    type Output = Type;

    fn index(&self, index: &TypeId) -> &Self::Output {
        self.get(index)
    }
}

impl std::ops::IndexMut<&TypeId> for TypeStore {
    fn index_mut(&mut self, index: &TypeId) -> &mut Self::Output {
        self.get_mut(index)
    }
}
