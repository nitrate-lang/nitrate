use crate::Store;

pub trait SaveToStorage {
    type Id;

    fn save_to_storage(self, ctx: &mut Store) -> Self::Id;

    fn save(self, ctx: &mut Store) -> Self::Id
    where
        Self: Sized,
    {
        self.save_to_storage(ctx)
    }
}
