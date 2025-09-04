#![warn(clippy::pedantic)]
#![allow(clippy::inline_always)]

pub(crate) mod abstract_machine;
pub(crate) mod compound;
pub(crate) mod control_flow;
pub(crate) mod symbol;
pub(crate) mod type_system;
pub(crate) mod type_system_test;

pub use abstract_machine::AbstractMachine;
