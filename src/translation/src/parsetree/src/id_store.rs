use bimap::BiMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;
use std::ops::Deref;
use std::sync::RwLock;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};

macro_rules! impl_interning_category  {
    ($key:ident, $store:ident, $store_global_name:ident, $create_fn:ident) => {
        #[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
        pub struct $key(NonZeroU32);

        impl_interning_category!(@impl_deref $key, $store, $store_global_name);
        impl_interning_category!(@impl_format $key, $store, $store_global_name);
        impl_interning_category!(@impl_serde $key, $store, $store_global_name, $create_fn);
        impl_interning_category!(@impl_store $key, $store, $store_global_name, $create_fn);
    };

    (@impl_deref $key:ident, $store:ident, $store_global_name:ident) => {
        impl std::ops::Deref for $key {
            type Target = str;

            fn deref(&self) -> &'static Self::Target {
                let Some(result) = $store_global_name.lookup(self) else {
                    panic!("Inconsistent state: ID not found in storage. \
                            The string interner storage was probably prematurely \
                            reset. Failed to dereference a dangling ID. \
                            No memory unsafety has resulted from this.");

                };

                result
            }
        }
    };

    (@impl_format $key:ident, $store:ident, $store_global_name:ident) => {
         impl std::fmt::Debug for $key {
            fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
                write!(f, "{:?}", self.deref())
            }
        }

        impl std::fmt::Display for $key {
            fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
                write!(f, "{}", self.deref())
            }
        }
    };

    (@impl_serde $key:ident, $store:ident, $store_global_name:ident, $create_fn:ident) => {
        impl Serialize for $key {
            fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
            where
                S: serde::Serializer,
            {
                let s: &str = self.deref();
                serializer.serialize_str(s)
            }
        }

        impl<'de> Deserialize<'de> for $key {
            fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
            where
                D: serde::Deserializer<'de>,
            {
                struct Visitor;

                impl<'de> serde::de::Visitor<'de> for Visitor {
                    type Value = $key;

                    fn expecting(&self, formatter: &mut std::fmt::Formatter) -> std::fmt::Result {
                        formatter.write_str("a string representing an interned ID")
                    }

                    fn visit_str<E>(self, v: &str) -> Result<Self::Value, E>
                    where
                        E: serde::de::Error,
                    {
                        Ok($create_fn(v.to_string()))
                    }
                }

                deserializer.deserialize_str(Visitor)
            }
        }
    };

    (@impl_store $key:ident, $store:ident, $store_global_name:ident, $create_fn:ident) => {
        struct $store {
            map: RwLock<BiMap<$key, &'static str>>,
            next_id: AtomicU32,
            id_space_exhausted: AtomicBool,
        }

        impl $store {
            fn new() -> Self {
                Self {
                    map: RwLock::new(BiMap::new()),
                    next_id: AtomicU32::new(1),
                    id_space_exhausted: AtomicBool::new(false),
                }
            }

            fn get_or_create(&self, path: String) -> Option<$key> {
                // Step 1: Read lock for a quick check.
                if let Some(id) = self.map.read().unwrap().get_by_right(path.as_str()) {
                    return Some(id.clone());
                }

                // Step 2: Acquire write lock for insertion logic.
                let mut map = self.map.write().unwrap();

                // Step 3: Check again under the write lock to prevent a race.
                if let Some(id) = map.get_by_right(path.as_str()) {
                    return Some(id.clone());
                }

                // Step 4: Check if ID space is exhausted *with a strong load*.
                // If we see the flag is true, we can safely return None.
                if self.id_space_exhausted.load(Ordering::Acquire) {
                    return None;
                }

                // Step 5: Get the next ID atomically.
                let new_id = self.next_id.fetch_add(1, Ordering::Relaxed);

                // Step 6: Create the new id and insert it.
                let file_id = NonZeroU32::new(new_id)
                    .map($key)
                    .expect("Atomic counter generated 0, which should not happen");

                map.insert(file_id.clone(), Box::leak(path.into_boxed_str()));

                // Step 7: If this was the last ID, set the flag.
                // We use a Release store to ensure the memory writes to the map are
                // synchronized with the flag being set.
                if new_id == u32::MAX {
                    self.id_space_exhausted.store(true, Ordering::Release);
                }

                Some(file_id)
            }

            fn lookup(&self, id: &$key) -> Option<&'static str> {
                let map = self.map.read().unwrap();
                map.get_by_left(id).copied()
            }

            fn clear(&self) {
                let mut map = self.map.write().unwrap();
                map.clear();
                map.shrink_to_fit();
                self.next_id.store(1, Ordering::Release);
                self.id_space_exhausted.store(false, Ordering::Release);
            }
        }

        static $store_global_name: once_cell::sync::Lazy<$store> =
            once_cell::sync::Lazy::new($store::new);

        pub fn $create_fn(path: String) -> $key {
            $store_global_name.get_or_create(path).expect("ID space exhausted")
        }
    };
}

impl_interning_category!(
    PackageNameId,
    PackageNameIdStore,
    PACKAGE_NAME_ID_STORE,
    intern_package_name
);

impl_interning_category!(
    ModuleNameId,
    ModuleNameIdStore,
    MODULE_NAME_ID_STORE,
    intern_module_name
);

impl_interning_category!(
    ImportAliasNameId,
    ImportAliasNameIdStore,
    IMPORT_ALIAS_NAME_ID_STORE,
    intern_import_alias_name
);

impl_interning_category!(
    ParameterNameId,
    ParameterNameIdStore,
    PARAMETER_NAME_ID_STORE,
    intern_parameter_name
);

impl_interning_category!(
    TypeNameId,
    TypeNameIdStore,
    TYPE_NAME_ID_STORE,
    intern_type_name
);

impl_interning_category!(
    StructFieldNameId,
    StructFieldNameIdStore,
    STRUCT_FIELD_NAME_ID_STORE,
    intern_struct_field_name
);

impl_interning_category!(
    EnumVariantNameId,
    EnumVariantNameIdStore,
    ENUM_VARIANT_NAME_ID_STORE,
    intern_enum_variant_name
);

impl_interning_category!(
    TraitNameId,
    TraitNameIdStore,
    TRAIT_NAME_ID_STORE,
    intern_trait_name
);

impl_interning_category!(
    FunctionNameId,
    FunctionNameIdStore,
    FUNCTION_NAME_ID_STORE,
    intern_function_name
);

impl_interning_category!(
    VariableNameId,
    VariableNameIdStore,
    VARIABLE_NAME_ID_STORE,
    intern_variable_name
);

impl_interning_category!(
    ArgNameId,
    ArgNameIdStore,
    ARG_NAME_ID_STORE,
    intern_arg_name
);

impl_interning_category!(
    OpaqueTypeNameId,
    OpaqueTypeNameIdStore,
    OPAQUE_TYPE_NAME_ID_STORE,
    intern_opaque_type_name
);

impl_interning_category!(
    LifetimeNameId,
    LifetimeNameIdStore,
    LIFETIME_NAME_ID_STORE,
    intern_lifetime_name
);

impl_interning_category!(
    LabelNameId,
    LabelNameIdStore,
    LABEL_NAME_ID_STORE,
    intern_label_name
);

impl_interning_category!(
    StringLiteralId,
    StringLiteralIdStore,
    STRING_LITERAL_ID_STORE,
    intern_string_literal
);

pub fn erase_interners() {
    PACKAGE_NAME_ID_STORE.clear();
    MODULE_NAME_ID_STORE.clear();
    IMPORT_ALIAS_NAME_ID_STORE.clear();
    PARAMETER_NAME_ID_STORE.clear();
    TYPE_NAME_ID_STORE.clear();
    STRUCT_FIELD_NAME_ID_STORE.clear();
    ENUM_VARIANT_NAME_ID_STORE.clear();
    TRAIT_NAME_ID_STORE.clear();
    FUNCTION_NAME_ID_STORE.clear();
    VARIABLE_NAME_ID_STORE.clear();
    ARG_NAME_ID_STORE.clear();
    OPAQUE_TYPE_NAME_ID_STORE.clear();
    LIFETIME_NAME_ID_STORE.clear();
    LABEL_NAME_ID_STORE.clear();
    STRING_LITERAL_ID_STORE.clear();
}
