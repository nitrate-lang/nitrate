#![no_std]

use spin;

#[panic_handler]
#[cfg(not(test))]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

pub enum Event {
    TaskCreated = 0,
    TaskExited = 1,
    TaskBlocked = 2,
    TaskUnblocked = 3,
    ObjectDestructed = 10,
}

#[repr(C)]
pub struct Interface {
    x: u32,
}

pub struct GC {
    pub lock: spin::Mutex<()>,
    pub enabled: bool,
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_create(_support: Interface) -> *mut GC {
    // TODO: Implement the logic to create a new GC instance

    return 0 as *mut GC;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_destroy(gc: *mut GC) {
    if gc.is_null() {
        return;
    }

    let gc = unsafe { &mut *gc };
    let _lock = gc.lock.lock();

    // TODO: Clean up the GC instance
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_enable(_gc: *mut GC) {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &mut *_gc };
    let _lock = gc.lock.lock();

    gc.enabled = true;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_disable(_gc: *mut GC) {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &mut *_gc };
    let _lock = gc.lock.lock();

    gc.enabled = false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_is_enabled(_gc: *const GC) -> bool {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    gc.enabled
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_manage(_gc: *mut GC, _base: *mut u8, _size: usize) -> bool {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to manage memory

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_unmanage(_gc: *mut GC, _base: *mut u8, _size: usize) {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to unmanage memory
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_is_managed(_gc: *mut GC, _base: *mut u8, _size: usize) -> bool {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to check if the memory is managed

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_add_root(_gc: *mut GC, _base: *const *mut u8) -> bool {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to add a root

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_del_root(_gc: *mut GC, _base: *const *mut u8) {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to delete a root
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_notify(_gc: *mut GC, event: u32, _p: u64) -> bool {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to notify the GC

    match event {
        x if x == Event::TaskCreated as u32 => {
            // TODO: Handle task creation event
            return false;
        }

        x if x == Event::TaskExited as u32 => {
            // TODO: Handle task exit event
            return false;
        }

        x if x == Event::TaskBlocked as u32 => {
            // TODO: Handle task blocked event
            return false;
        }

        x if x == Event::TaskUnblocked as u32 => {
            // TODO: Handle task unblocked event
            return false;
        }

        x if x == Event::ObjectDestructed as u32 => {
            // TODO: Handle object destructed event
            return false;
        }

        _ => {
            return false;
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_step(_gc: *mut GC) -> bool {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    if !gc.enabled {
        return true;
    }

    // TODO: Implement the logic to perform a GC step

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_catchup(_gc: *mut GC) {
    assert!(!_gc.is_null(), "GC instance must not be null");

    loop {
        let work_remaining = azide_gc_step(_gc);
        if !work_remaining {
            break;
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_malloc(_gc: *mut GC, _size: usize, _align: usize) -> *mut u8 {
    assert!(!_gc.is_null(), "GC instance must not be null");

    let gc = unsafe { &*_gc };
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to allocate memory

    return 0 as *mut u8;
}
