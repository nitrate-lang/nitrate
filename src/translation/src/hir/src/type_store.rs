use hashbrown::HashMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

use crate::ty::Type;

#[derive(Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct TypeId {
    id: NonZeroU32,
}

pub struct TypeStore {
    items: HashMap<TypeId, Type>,
    next_id: NonZeroU32,
}

impl TypeStore {
    pub fn new() -> Self {
        Self {
            items: HashMap::new(),
            next_id: NonZeroU32::new(1).unwrap(),
        }
    }

    pub fn store(&mut self, item: Type) -> TypeId {
        let id = self.next_id;
        self.items.insert(TypeId { id }, item);

        self.next_id = self
            .next_id
            .checked_add(1)
            .expect("TypeStore overflowed NonZeroU32");

        TypeId { id }
    }

    fn get(&self, id: &TypeId) -> &Type {
        self.items.get(id).expect("TypeId not found in TypeStore")
    }

    fn get_mut(&mut self, id: &TypeId) -> &mut Type {
        self.items
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
