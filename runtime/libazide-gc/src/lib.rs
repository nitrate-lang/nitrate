#![no_std]

extern crate std;

use spin;

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

#[repr(C)]
#[derive(Copy, Clone)]
pub struct MallocUserData {
    _data: *mut (),
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct FreeUserData {
    _data: *mut (),
}

#[repr(C)]
pub struct Interface {
    pub pause_tasks: Option<extern "C" fn(ud: PauseTaskUserData)>,
    pub pause_tasks_ud: PauseTaskUserData,

    pub resume_tasks: Option<extern "C" fn(ud: ResumeTaskUserData)>,
    pub resume_tasks_ud: ResumeTaskUserData,

    pub destroyer: Option<
        extern "C" fn(ud: AsyncFinalizerUserData, base: *mut u8, size: usize, object_id: u64),
    >,
    pub destroyer_ud: AsyncFinalizerUserData,

    pub malloc: Option<extern "C" fn(ud: MallocUserData, size: usize) -> *mut u8>,
    pub malloc_ud: MallocUserData,

    pub free: Option<unsafe extern "C" fn(ud: FreeUserData, ptr: *mut u8)>,
    pub free_ud: FreeUserData,
}

pub struct GC {
    lock: spin::Mutex<()>,
    enabled: bool,

    pause_tasks: extern "C" fn(ud: PauseTaskUserData),
    pause_tasks_ud: PauseTaskUserData,

    resume_tasks: extern "C" fn(ud: ResumeTaskUserData),
    resume_tasks_ud: ResumeTaskUserData,

    destroyer:
        extern "C" fn(ud: AsyncFinalizerUserData, base: *mut u8, size: usize, object_id: u64),
    destroyer_ud: AsyncFinalizerUserData,

    malloc: extern "C" fn(ud: MallocUserData, size: usize) -> *mut u8,
    malloc_ud: MallocUserData,

    free: unsafe extern "C" fn(ud: FreeUserData, ptr: *mut u8),
    free_ud: FreeUserData,
}

// new method for GC
impl GC {
    pub fn new(support: Interface) -> GC {
        let malloc = support.malloc.expect("malloc ptr is null");
        let free = support.free.expect("free ptr is null");
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
            malloc,
            malloc_ud: support.malloc_ud,
            free,
            free_ud: support.free_ud,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_create(support: Interface) -> *mut GC {
    let malloc = support.malloc.expect("malloc ptr is null");

    let alloc_size = size_of::<GC>();
    let ptr = malloc(support.malloc_ud, alloc_size) as *mut GC;
    if ptr.is_null() || !ptr.is_aligned() {
        return ptr;
    }

    let gc = unsafe { &mut *ptr };

    unsafe {
        std::ptr::write(gc, GC::new(support));
    }

    gc
}

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_destroy(gc_ptr: *mut GC) {
    if gc_ptr.is_null() {
        return;
    }

    assert!(gc_ptr.is_aligned());

    let gc: &mut GC = unsafe { &mut *gc_ptr };

    let free = gc.free;
    let free_ud = gc.free_ud;

    unsafe {
        std::ptr::drop_in_place(gc);
        free(free_ud, gc_ptr as *mut u8);
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
