use serde::{Deserialize, Serialize};

#[derive(Clone, Serialize, Deserialize)]
pub enum Expr {}

impl Expr {
    pub fn digest_128(&self) -> [u8; 16] {
        let mut hasher = blake3::Hasher::new();
        hasher.update(&serde_json::to_vec(self).unwrap_or_default());
        let hash = hasher.finalize();
        let bytes = hash.as_bytes();

        let mut result = [0u8; 16];
        result.copy_from_slice(&bytes[..16]);
        result
    }
}

impl std::fmt::Debug for Expr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        Ok(())
    }
}
