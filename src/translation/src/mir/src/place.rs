use crate::store::LocalId;
use nitrate_nstring::NString;

/// A Place represents a memory location: where data is stored.
/// Places are used to read from (via `Operand::Copy` or `Operand::Move`)
/// and write to (via `Statement::Assign`).
#[derive(Debug, Clone, serde::Serialize, serde::Deserialize, PartialEq, Eq, Hash)]
pub enum Place {
    /// A local variable (SSA register). The type is always Some because
    /// all locals are declared with known types in MIR.
    Local(LocalId),

    /// A static (global) variable, referenced by name.
    Static(NString),

    /// Dereference of a pointer/reference: `*place`
    Deref(Box<Place>),

    /// Field access on a struct: `place.field_name`
    Field { base: Box<Place>, field_name: NString },

    /// Index into an array or slice: `place[index]`
    Index { base: Box<Place>, index: Box<Place> },

    /// Downcast to a specific enum variant: `place as VariantName`
    Downcast { base: Box<Place>, variant_name: NString },
}

impl Place {
    /// Returns the Place for a local variable.
    #[must_use]
    pub fn local(local: LocalId) -> Self {
        Place::Local(local)
    }

    /// Returns the Place for a static/global.
    #[must_use]
    pub fn static_(name: NString) -> Self {
        Place::Static(name)
    }

    /// Returns a Deref place.
    #[must_use]
    pub fn deref(base: Place) -> Self {
        Place::Deref(Box::new(base))
    }

    /// Returns a Field place.
    #[must_use]
    pub fn field(base: Place, field_name: NString) -> Self {
        Place::Field {
            base: Box::new(base),
            field_name,
        }
    }

    /// Returns an Index place.
    #[must_use]
    pub fn index(base: Place, index: Place) -> Self {
        Place::Index {
            base: Box::new(base),
            index: Box::new(index),
        }
    }
}
