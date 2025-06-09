#![no_std]

mod azide_gc_setup;

extern crate alloc;

use spin;

#[panic_handler]
#[cfg(not(test))]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    if cfg!(target_arch = "x86_64") || cfg!(target_arch = "x86") {
        // For x86_64, we use the `ud2` instruction to trigger an undefined instruction exception.
        // This is a common way to handle panics in low-level code.
        unsafe {
            core::arch::asm!("ud2");
        }
    }

    // For other architectures, we just loop indefinitely.
    loop {
        // This is a no-op, but it prevents the program from continuing execution.
        // In a real-world scenario, you might want to log the panic or perform some cleanup.
    }
}

#[unsafe(no_mangle)]
extern "C" fn rust_eh_personality() {
    panic!("No exception handling support in this build");
}

pub enum Event {
    TaskCreated = 0,
    TaskExited = 1,
    TaskBlocked = 2,
    TaskUnblocked = 3,
    ObjectDestructed = 10,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct PauseTaskUserData {
    _data: *mut (),
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct ResumeTaskUserData {
    _data: *mut (),
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct AsyncFinalizerUserData {
    _data: *mut (),
}

type PauseTaskFn = extern "C" fn(ud: PauseTaskUserData);
type ResumeTaskFn = extern "C" fn(ud: ResumeTaskUserData);
type AsyncFinalizerFn =
    extern "C" fn(ud: AsyncFinalizerUserData, base: *mut u8, size: usize, object_id: u64);

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Interface {
    pub pause_tasks: Option<PauseTaskFn>,
    pub pause_tasks_ud: PauseTaskUserData,

    pub resume_tasks: Option<ResumeTaskFn>,
    pub resume_tasks_ud: ResumeTaskUserData,

    pub destroyer: Option<AsyncFinalizerFn>,
    pub destroyer_ud: AsyncFinalizerUserData,
}

pub struct GC {
    lock: spin::Mutex<()>,
    enabled: bool,

    pause_tasks: PauseTaskFn,
    pause_tasks_ud: PauseTaskUserData,

    resume_tasks: ResumeTaskFn,
    resume_tasks_ud: ResumeTaskUserData,

    destroyer: AsyncFinalizerFn,
    destroyer_ud: AsyncFinalizerUserData,
}

// new method for GC
impl GC {
    pub fn new(support: Interface) -> GC {
        let destroyer = support.destroyer.expect("async_finalizer ptr is null");
        let pause_tasks = support.pause_tasks.expect("pause_tasks ptr is null");
        let resume_tasks = support.resume_tasks.expect("resume_tasks ptr is null");

        GC {
            lock: spin::Mutex::new(()),
            enabled: false,
            pause_tasks,
            pause_tasks_ud: support.pause_tasks_ud,
            resume_tasks,
            resume_tasks_ud: support.resume_tasks_ud,
            destroyer,
            destroyer_ud: support.destroyer_ud,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_create(support: &Interface) -> *mut GC {
    let layout = alloc::alloc::Layout::new::<GC>();
    let gc_ptr = unsafe { alloc::alloc::alloc(layout) } as *mut GC;
    if gc_ptr.is_null() {
        return gc_ptr;
    }

    unsafe {
        core::ptr::write(gc_ptr, GC::new(*support));
    }

    gc_ptr
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_destroy(gc_ptr: *mut GC) {
    if gc_ptr.is_null() {
        return;
    }

    assert!(gc_ptr.is_aligned());

    let gc: &mut GC = unsafe { &mut *gc_ptr };
    let layout = alloc::alloc::Layout::new::<GC>();

    unsafe {
        core::ptr::drop_in_place(gc);
        alloc::alloc::dealloc(gc_ptr as *mut u8, layout);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_enable(gc: &mut GC) {
    let _lock = gc.lock.lock();

    gc.enabled = true;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_disable(gc: &mut GC) {
    let _lock = gc.lock.lock();

    gc.enabled = false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_is_enabled(gc: &GC) -> bool {
    let _lock = gc.lock.lock();

    gc.enabled
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_manage(gc: &mut GC, _base: *mut u8, _size: usize) -> bool {
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to manage memory

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_unmanage(gc: &mut GC, _base: *mut u8, _size: usize) {
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to unmanage memory
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_is_managed(gc: &mut GC, _base: *mut u8, _size: usize) -> bool {
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to check if the memory is managed

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_add_root(gc: &mut GC, _base: *const *mut u8) -> bool {
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to add a root

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_del_root(gc: &mut GC, _base: *const *mut u8) {
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to delete a root
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_notify(gc: &mut GC, event: u32, _p: u64) -> bool {
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
pub extern "C" fn azide_gc_step(gc: &mut GC) -> bool {
    let _lock = gc.lock.lock();

    if !gc.enabled {
        return true;
    }

    // TODO: Implement the logic to perform a GC step

    return false;
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_catchup(gc: &mut GC) {
    loop {
        let work_remaining = azide_gc_step(gc);
        if !work_remaining {
            break;
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_malloc(gc: &mut GC, _size: usize, _align: usize) -> *mut u8 {
    let _lock = gc.lock.lock();

    // TODO: Implement the logic to allocate memory

    return 0 as *mut u8;
}
