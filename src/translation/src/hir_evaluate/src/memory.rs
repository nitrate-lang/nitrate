use crate::error::EvalError;
use std::collections::BTreeMap;

/// A contiguous allocation in the abstract heap.
#[derive(Debug, Clone)]
pub struct Allocation {
    pub data: Vec<u8>,
    pub mutable: bool,
}

impl Allocation {
    pub fn new(size: usize, mutable: bool) -> Self {
        Self {
            data: vec![0u8; size],
            mutable,
        }
    }
}

/// Abstract memory for the HIR evaluator.
///
/// All pointer values are validated to be within known allocations.
/// Invalid pointers produce `EvalError` rather than crashing.
///
/// Allocations are keyed by their base address (a `u64`), and each
/// allocation has a size and mutability flag.
#[derive(Debug, Clone)]
pub struct Memory {
    /// Map from base address to allocation data.
    allocations: BTreeMap<u64, Allocation>,
    /// Next free address for allocation.
    next_addr: u64,
    /// Total allocated bytes (for memory limit enforcement).
    total_allocated: usize,
    /// Maximum total allocated bytes.
    limit: usize,
}

impl Memory {
    pub fn new(limit: usize) -> Self {
        Self {
            allocations: BTreeMap::new(),
            next_addr: 0x1000, // start at page-aligned address
            total_allocated: 0,
            limit,
        }
    }

    /// Allocate a new memory region.
    ///
    /// Returns the base address of the allocation.
    pub fn allocate(&mut self, size: usize, align: u64, mutable: bool) -> Result<u64, EvalError> {
        if self.total_allocated + size > self.limit {
            return Err(EvalError::MemoryLimitExceeded);
        }

        // Align the next address
        let addr = self.next_addr;
        let aligned = ((addr + align - 1) / align) * align;

        self.allocations.insert(aligned, Allocation::new(size, mutable));
        self.total_allocated += size;
        self.next_addr = aligned + size as u64;

        Ok(aligned)
    }

    /// Read bytes from an allocation at the given offset.
    pub fn read(&self, base: u64, offset: u64, size: usize) -> Result<Vec<u8>, EvalError> {
        let alloc = self.allocations.get(&base).ok_or(EvalError::InvalidPointer)?;

        let start = offset as usize;
        let end = start + size;
        if end > alloc.data.len() {
            return Err(EvalError::OutOfBoundsAccess);
        }

        Ok(alloc.data[start..end].to_vec())
    }

    /// Write bytes to an allocation at the given offset.
    pub fn write(&mut self, base: u64, offset: u64, data: &[u8]) -> Result<(), EvalError> {
        let alloc = self.allocations.get_mut(&base).ok_or(EvalError::InvalidPointer)?;

        if !alloc.mutable {
            return Err(EvalError::TypeError); // writing to immutable memory
        }

        let start = offset as usize;
        let end = start + data.len();
        if end > alloc.data.len() {
            return Err(EvalError::OutOfBoundsAccess);
        }

        alloc.data[start..end].copy_from_slice(data);
        Ok(())
    }

    /// Check if a pointer is valid (points to a known allocation).
    pub fn is_valid_pointer(&self, base: u64) -> bool {
        self.allocations.contains_key(&base)
    }

    /// Get the size of an allocation at the given base address.
    pub fn allocation_size(&self, base: u64) -> Option<usize> {
        self.allocations.get(&base).map(|a| a.data.len())
    }

    /// Get whether an allocation is mutable.
    pub fn allocation_mutable(&self, base: u64) -> Option<bool> {
        self.allocations.get(&base).map(|a| a.mutable)
    }

    /// Deallocate a region (for completeness — may not be needed in const-eval).
    pub fn deallocate(&mut self, base: u64) {
        if let Some(alloc) = self.allocations.remove(&base) {
            self.total_allocated = self.total_allocated.saturating_sub(alloc.data.len());
        }
    }
}
