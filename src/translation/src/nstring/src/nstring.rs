use bimap::BiMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;
use std::ops::Deref;
use std::sync::RwLock;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct NString(NonZeroU32);

impl std::ops::Deref for NString {
    type Target = str;

    fn deref(&self) -> &'static Self::Target {
        let Some(result) = NSTRING_STORE.lookup(self) else {
            panic!(
                "Inconsistent state: ID not found in storage. \
                            The string interner storage was probably prematurely \
                            reset. Failed to dereference a dangling ID. \
                            No memory unsafety has resulted from this."
            );
        };

        result
    }
}

impl std::fmt::Debug for NString {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.deref())
    }
}

impl std::fmt::Display for NString {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.deref())
    }
}

impl Default for NString {
    fn default() -> Self {
        intern_nstring("".to_string())
    }
}

impl<T: Into<String>> From<T> for NString {
    fn from(s: T) -> Self {
        intern_nstring(s.into())
    }
}

impl Serialize for NString {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        let s: &str = self.deref();
        serializer.serialize_str(s)
    }
}

impl<'de> Deserialize<'de> for NString {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        struct Visitor;

        impl<'de> serde::de::Visitor<'de> for Visitor {
            type Value = NString;

            fn expecting(&self, formatter: &mut std::fmt::Formatter) -> std::fmt::Result {
                formatter.write_str("a string representing an interned ID")
            }

            fn visit_str<E>(self, v: &str) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok(intern_nstring(v.to_string()))
            }
        }

        deserializer.deserialize_str(Visitor)
    }
}

struct NStringStore {
    map: RwLock<BiMap<NString, &'static str>>,
    next_id: AtomicU32,
    id_space_exhausted: AtomicBool,
}

impl NStringStore {
    fn new() -> Self {
        Self {
            map: RwLock::new(BiMap::new()),
            next_id: AtomicU32::new(1),
            id_space_exhausted: AtomicBool::new(false),
        }
    }

    fn get_or_create(&self, path: String) -> Option<NString> {
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
            .map(NString)
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

    fn lookup(&self, id: &NString) -> Option<&'static str> {
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

static NSTRING_STORE: once_cell::sync::Lazy<NStringStore> =
    once_cell::sync::Lazy::new(NStringStore::new);

pub fn intern_nstring<T: Into<String>>(path: T) -> NString {
    NSTRING_STORE
        .get_or_create(path.into())
        .expect("ID space exhausted")
}

pub fn nstring_forget_all() {
    NSTRING_STORE.clear();
}
