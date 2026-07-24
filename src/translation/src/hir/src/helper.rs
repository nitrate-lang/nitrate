use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct PowOf2<T>(T);

impl PowOf2<u32> {
    pub fn new(value: u32) -> Option<Self> {
        if value.is_power_of_two() {
            Some(PowOf2(value))
        } else {
            None
        }
    }

    pub fn get(&self) -> u32 {
        self.0
    }
}

impl<T> std::ops::Deref for PowOf2<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
