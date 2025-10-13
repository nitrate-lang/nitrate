use crate::Store;

pub trait IntoStoreId {
    type Id;

    fn into_id(self, store: &Store) -> Self::Id;
}
