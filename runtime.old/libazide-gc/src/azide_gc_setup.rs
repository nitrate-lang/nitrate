#[repr(C)]
#[derive(Copy, Clone)]
pub struct MallocUD {
    _data: *mut (),
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct FreeUD {
    _data: *mut (),
}

type MallocFn = unsafe extern "C" fn(ud: MallocUD, size: usize) -> *mut u8;
type FreeFn = unsafe extern "C" fn(ud: FreeUD, ptr: *mut u8);

#[derive(Copy, Clone)]
struct AzideGCInternalAllocator {
    malloc_fn: Option<MallocFn>,
    malloc_ud: MallocUD,
    free_fn: Option<FreeFn>,
    free_ud: FreeUD,
}

unsafe impl alloc::alloc::GlobalAlloc for AzideGCInternalAllocator {
    unsafe fn alloc(&self, layout: alloc::alloc::Layout) -> *mut u8 {
        if let Some(malloc_fn) = self.malloc_fn {
            unsafe { malloc_fn(self.malloc_ud, layout.size()) }
        } else {
            core::ptr::null_mut()
        }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: alloc::alloc::Layout) {
        if let Some(free_fn) = self.free_fn {
            unsafe { free_fn(self.free_ud, ptr) };
        }
    }
}

#[global_allocator]
static mut GLOBAL_ALLOCATOR: AzideGCInternalAllocator = AzideGCInternalAllocator {
    malloc_fn: None,
    malloc_ud: MallocUD {
        _data: core::ptr::null_mut(),
    },
    free_fn: None,
    free_ud: FreeUD {
        _data: core::ptr::null_mut(),
    },
};

#[unsafe(no_mangle)]
pub extern "C" fn azide_gc_setup(
    malloc: Option<MallocFn>,
    malloc_ud: MallocUD,
    free: Option<FreeFn>,
    free_ud: FreeUD,
) {
    let malloc_fn = malloc.expect("malloc ptr is null");
    let free_fn = free.expect("free ptr is null");

    unsafe {
        GLOBAL_ALLOCATOR = AzideGCInternalAllocator {
            malloc_fn: Some(malloc_fn),
            malloc_ud: malloc_ud,
            free_fn: Some(free_fn),
            free_ud: free_ud,
        };
    }
}
