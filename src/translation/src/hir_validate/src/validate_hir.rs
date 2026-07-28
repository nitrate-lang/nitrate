use log::debug;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::SymbolTab;
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use std::collections::HashSet;

use crate::diagnosis::ValidateErr;

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

pub struct ValidateCtx<'m> {
    pub(crate) visited: HashSet<*const ()>,
    pub(crate) m: &'m SymbolTab,
    /// The current module path (e.g., ["root", "foo"] for module foo inside root).
    pub(crate) current_module_path: Vec<NString>,
    pub(crate) log: &'m CompilerLog,
}

impl<'m> ValidateCtx<'m> {
    pub fn new(m: &'m SymbolTab, log: &'m CompilerLog) -> Self {
        ValidateCtx {
            visited: HashSet::new(),
            m,
            current_module_path: Vec::new(),
            log,
        }
    }

    /// Report a validation error to the compiler log.
    pub(crate) fn report(&self, err: ValidateErr) {
        self.log.report(&err);
    }

    pub(crate) fn cyclic_bail<T>(&mut self, item: &T) -> bool {
        let ptr = item as *const _ as *const ();
        let not_visited = self.visited.insert(ptr);
        !not_visited
    }

    /// Check whether `target_visibility` is accessible from the current module path,
    /// given the fully-qualified name of the target item.
    pub(crate) fn check_visibility(&self, target_visibility: &Visibility, target_qualified_name: &NString) -> bool {
        match target_visibility {
            Visibility::Pub => true,
            Visibility::Pro => {
                // "Protected" visibility: accessible from the same project.
                // Since we don't have a project/package concept yet, allow all.
                // TODO: Restrict to sibling modules
                true
            }
            Visibility::Sec => {
                // "Section/private" visibility: accessible only from within the same module.
                // Check if the target is in the same module as the current scope.
                let target_path: &str = target_qualified_name;

                // Build the current module path string
                let current_path: String = {
                    let mut s = String::new();
                    for (i, segment) in self.current_module_path.iter().enumerate() {
                        if i > 0 {
                            s.push_str("::");
                        }
                        s.push_str(segment);
                    }
                    s
                };

                // If the current scope is empty (root), only root items are accessible.
                if self.current_module_path.is_empty() {
                    // Root-level items have no "::" in their qualified name
                    return !target_path.contains("::");
                }

                // Check if the target's module path equals the current module path
                // Target name is like "module::item_name". Extract the module part.
                if let Some(last_sep) = target_path.rfind("::") {
                    let target_module = &target_path[..last_sep];
                    target_module == current_path
                } else {
                    // The target is at root level, but we're inside a module -> not accessible
                    false
                }
            }
        }
    }

    /// Check if a value expression refers to a mutable place.
    /// Returns true if the place is mutable.
    pub(crate) fn is_place_mutable(&self, value: &Value) -> bool {
        match value {
            Value::LocalVariableSymbol { id, .. } => id.borrow().is_mutable,
            Value::GlobalVariableSymbol { id, .. } => id.borrow().is_mutable,
            Value::ParameterSymbol { id, .. } => id.borrow().is_mutable,
            Value::Deref { place, .. } => {
                // Dereferencing a mutable reference/pointer yields a mutable place
                let place = place.borrow();
                if let Ok(ty) = place.determine_type(self.m) {
                    match ty {
                        Type::Reference { mutable, .. } | Type::Pointer { mutable, .. } => mutable,
                        Type::SliceRef { mutable, .. } | Type::SlicePtr { mutable, .. } => mutable,
                        _ => false,
                    }
                } else {
                    false
                }
            }
            _ => false,
        }
    }
}

/// Check a property and report the given error if the property check fails.
///
/// The closure `f` receives `ctx` as `&mut ValidateCtx` to allow checking
/// properties that need access to the validation context (e.g., type verification).
pub(crate) fn establish_property(
    ctx: &mut ValidateCtx,
    name: &str,
    err: ValidateErr,
    f: impl FnOnce(&mut ValidateCtx) -> Result<(), ()>,
) -> Result<(), ()> {
    debug!("Establishing property: \"{}\"", name);
    let result = f(ctx);
    match result {
        Ok(_) => debug!("Established property \"{}\"", name),
        Err(_) => {
            debug!("Failed to establish property \"{}\"", name);
            ctx.report(err);
        }
    }
    result
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
