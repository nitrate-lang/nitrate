use bimap::BiMap;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;
use std::sync::RwLock;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};

#[derive(Clone, Debug, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct FileId(NonZeroU32);

impl std::ops::Deref for FileId {
    type Target = str;

    fn deref(&self) -> &'static Self::Target {
        FILE_ID_STORE
            .lookup_path(self)
            .expect("FileId should always map to a valid string")
    }
}

struct FileIdStore {
    map: RwLock<BiMap<FileId, &'static str>>,
    next_id: AtomicU32,
    id_space_exhausted: AtomicBool,
}

impl FileIdStore {
    fn new() -> Self {
        Self {
            map: RwLock::new(BiMap::new()),
            next_id: AtomicU32::new(1),
            id_space_exhausted: AtomicBool::new(false),
        }
    }

    fn get_or_create(&self, path: &str) -> Option<FileId> {
        // Step 1: Read lock for a quick check.
        if let Some(id) = self.map.read().unwrap().get_by_right(path) {
            return Some(id.clone());
        }

        // Step 2: Acquire write lock for insertion logic.
        let mut map = self.map.write().unwrap();

        // Step 3: Check again under the write lock to prevent a race.
        if let Some(id) = map.get_by_right(path) {
            return Some(id.clone());
        }

        // Step 4: Check if ID space is exhausted *with a strong load*.
        // If we see the flag is true, we can safely return None.
        if self.id_space_exhausted.load(Ordering::Acquire) {
            return None;
        }

        // Step 5: Get the next ID atomically.
        let new_id = self.next_id.fetch_add(1, Ordering::Relaxed);

        // Step 6: Create the new FileId and insert it.
        let file_id = NonZeroU32::new(new_id)
            .map(FileId)
            .expect("Atomic counter generated 0, which should not happen");

        map.insert(file_id.clone(), Box::leak(path.to_owned().into_boxed_str()));

        // Step 7: If this was the last ID, set the flag.
        // We use a Release store to ensure the memory writes to the map are
        // synchronized with the flag being set.
        if new_id == u32::MAX {
            self.id_space_exhausted.store(true, Ordering::Release);
        }

        Some(file_id)
    }

    fn lookup_path(&self, id: &FileId) -> Option<&'static str> {
        let map = self.map.read().unwrap();
        map.get_by_left(id).copied()
    }
}

static FILE_ID_STORE: once_cell::sync::Lazy<FileIdStore> =
    once_cell::sync::Lazy::new(FileIdStore::new);

pub fn intern_file_id(path: &str) -> Option<FileId> {
    FILE_ID_STORE.get_or_create(path)
}
