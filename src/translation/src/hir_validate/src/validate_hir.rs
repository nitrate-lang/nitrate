use nitrate_hir::Store;

pub trait ValidateHir {
    fn validate(&self, store: &Store) -> Result<(), ()>;
}
