use log::debug;
use nitrate_diagnosis::CompilerLog;
use std::collections::HashSet;

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

pub(crate) fn establish_property(name: &str, f: impl FnOnce() -> Result<(), ()>) -> Result<(), ()> {
    debug!("Establishing property: \"{}\"", name);
    let result = f();
    match result {
        Ok(_) => debug!("Established property \"{}\"", name),
        Err(_) => debug!("Failed to establish property \"{}\"", name),
    }
    result
}

pub struct ValidateCtx<'log> {
    pub(crate) log: &'log CompilerLog,
    pub(crate) visited: HashSet<*const ()>,
}

impl<'log> ValidateCtx<'log> {
    pub fn new(log: &'log CompilerLog) -> Self {
        ValidateCtx {
            log,
            visited: HashSet::new(),
        }
    }

    pub(crate) fn cyclic_bail<T>(&mut self, item: &T) -> bool {
        let ptr = item as *const _ as *const ();
        let not_visited = self.visited.insert(ptr);
        !not_visited
    }
}

pub trait ValidateHirValue
where
    Self: Sized,
{
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()>;
    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()>;
}

pub trait ValidateHirItem
where
    Self: Sized,
{
    fn verify(&self, ctx: &mut ValidateCtx) -> Result<(), ()>;
    fn validate(self, ctx: &mut ValidateCtx) -> Result<ValidHir<Self>, ()>;
}

pub struct ValidateTypeOptions {
    pub require_sized: bool,
}

impl ValidateTypeOptions {
    pub fn sized() -> Self {
        ValidateTypeOptions { require_sized: true }
    }

    pub fn un_sized() -> Self {
        ValidateTypeOptions { require_sized: false }
    }
}

pub trait ValidateHirType
where
    Self: Sized,
{
    fn verify(&self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<(), ()>;

    fn validate(self, ctx: &mut ValidateCtx, options: &ValidateTypeOptions) -> Result<ValidHir<Self>, ()>;
}
