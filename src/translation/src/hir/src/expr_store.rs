use hashbrown::HashMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

use crate::expr::Expr;

#[derive(Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct ExprId {
    id: NonZeroU32,
}

pub struct ExprStore {
    items: HashMap<ExprId, Expr>,
    next_id: NonZeroU32,
}

impl ExprStore {
    pub fn new() -> Self {
        Self {
            items: HashMap::new(),
            next_id: NonZeroU32::new(1).unwrap(),
        }
    }

    pub fn store(&mut self, item: Expr) -> ExprId {
        let id = self.next_id;
        self.items.insert(ExprId { id }, item);

        self.next_id = self
            .next_id
            .checked_add(1)
            .expect("ExprStore overflowed NonZeroU32");

        ExprId { id }
    }

    fn get(&self, id: &ExprId) -> &Expr {
        self.items.get(id).expect("ExprId not found in ExprStore")
    }

    fn get_mut(&mut self, id: &ExprId) -> &mut Expr {
        self.items
            .get_mut(id)
            .expect("ExprId not found in ExprStore")
    }
}

impl std::ops::Index<&ExprId> for ExprStore {
    type Output = Expr;

    fn index(&self, index: &ExprId) -> &Self::Output {
        self.get(index)
    }
}

impl std::ops::IndexMut<&ExprId> for ExprStore {
    fn index_mut(&mut self, index: &ExprId) -> &mut Self::Output {
        self.get_mut(index)
    }
}
