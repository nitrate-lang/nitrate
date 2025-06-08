#![no_std]

#[repr(C)]
pub struct Interface {
    x: u32,
}

#[repr(C)]
pub struct GC {}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_create(_support: Interface) -> *mut GC {
    return 0 as *mut GC;
}
