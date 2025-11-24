use nitrate_diagnosis::CompilerLog;
use nitrate_hir::SymbolTab;

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

pub trait ValidateHirValue
where
    Self: Sized,
{
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()>;
    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()>;
}

pub trait ValidateHirItem
where
    Self: Sized,
{
    fn verify(&self, tab: &SymbolTab, log: &CompilerLog) -> Result<(), ()>;
    fn validate(self, tab: &SymbolTab, log: &CompilerLog) -> Result<ValidHir<Self>, ()>;
}

pub struct ValidateTypeOptions {
    pub require_sized: bool,
}

impl ValidateTypeOptions {
    pub fn storable() -> Self {
        ValidateTypeOptions {
            require_sized: true,
        }
    }
}

pub trait ValidateHirType
where
    Self: Sized,
{
    fn verify(
        &self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<(), ()>;

    fn validate(
        self,
        tab: &SymbolTab,
        log: &CompilerLog,
        options: &ValidateTypeOptions,
    ) -> Result<ValidHir<Self>, ()>;
}
