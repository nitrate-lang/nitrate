use nitrate_hir::Store;

pub struct ValidHir<T> {
    inner: T,
}

impl<T> ValidHir<T> {
    pub(crate) fn new(inner: T) -> Self {
        ValidHir { inner }
    }

    pub fn into_inner(self) -> T {
        self.inner
    }
}

pub trait ValidateHir
where
    Self: Sized,
{
    fn verify(&self, store: &Store) -> Result<(), ()>;
    fn validate(self, store: &Store) -> Result<ValidHir<Self>, ()>;
}
