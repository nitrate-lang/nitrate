use crate::ty::Type;

pub enum DigestError {
    SerializationError(serde_json::Error),
}

pub trait NodeDigest {
    fn digest_256(&self) -> Result<[u8; 32], DigestError>;
    fn digest_128(&self) -> Result<[u8; 16], DigestError>;
}

impl NodeDigest for Type {
    fn digest_256(&self) -> Result<[u8; 32], DigestError> {
        let mut hasher = blake3::Hasher::new();

        let json = serde_json::to_vec(self).map_err(DigestError::SerializationError)?;
        hasher.update(&json);

        Ok(hasher.finalize().as_bytes().to_owned())
    }

    fn digest_128(&self) -> Result<[u8; 16], DigestError> {
        let full_hash = self.digest_256()?;
        let mut short_hash = [0u8; 16];
        short_hash.copy_from_slice(&full_hash[..16]);
        Ok(short_hash)
    }
}
