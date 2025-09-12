use crate::type_system::Type;
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
        self.next_id += 1;
        TypeId { id }
    }

    pub fn get(&self, id: &TypeId) -> Option<&Type> {
        self.types.get(id)
    }

    pub fn get_mut(&mut self, id: &TypeId) -> Option<&mut Type> {
        self.types.get_mut(id)
    }
}
