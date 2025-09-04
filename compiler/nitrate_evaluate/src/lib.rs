#![warn(clippy::pedantic)]
#![allow(clippy::inline_always)]

mod abstract_machine;
mod compound;
mod control_flow;
mod symbol;
mod type_system;
mod type_system_test;

pub use abstract_machine::AbstractMachine;
