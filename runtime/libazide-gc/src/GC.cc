#include <azide-gc/GC.hh>
#include <boost/config.hpp>

namespace azide::gc {
  auto azide_gc_create(TaskAPI task_api, AsyncFinalization runner) -> GC* {
    // TODO: Create and initialize a GC instance
    (void)task_api;
    (void)runner;

    return nullptr;
  }

  auto azide_gc_destroy(GC* gc) -> void {
    // TODO: Destroy the GC instance
    (void)gc;
  }

  auto azide_gc_enable(GC* gc) -> void {
    // TODO: Enable the GC instance
    (void)gc;
  }

  auto azide_gc_disable(GC* gc) -> void {
    // TODO: Disable the GC instance
    (void)gc;
  }

  auto azide_gc_is_enabled(const GC* gc) -> bool {
    // TODO: Check if the GC instance is enabled
    (void)gc;

    return false;
  }

  auto azide_gc_manage(GC* gc, void* base, size_t size) -> bool {
    // TODO: Manage the memory range
    (void)gc;
    (void)base;
    (void)size;

    return false;
  }

  auto azide_gc_unmanage(GC* gc, void* base, size_t size) -> void {
    // TODO: Unmanage the memory range
    (void)gc;
    (void)base;
    (void)size;
  }

  auto azide_gc_is_managed(GC* gc, void* base, size_t size) -> bool {
    // TODO: Check if the memory range is managed
    (void)gc;
    (void)base;
    (void)size;

    return false;
  }

  auto azide_gc_add_root(GC* gc, void** root) -> bool {
    // TODO: Add a root pointer
    (void)gc;
    (void)root;

    return false;
  }

  auto azide_gc_del_root(GC* gc, void** root) -> void {
    // TODO: Remove a root pointer
    (void)gc;
    (void)root;
  }

  auto azide_gc_notify(GC* gc, Event event, uint64_t p) -> bool {
    // TODO: Handle the event notification
    (void)gc;
    (void)event;
    (void)p;

    return false;
  }

  auto azide_gc_step(GC* gc) -> uint64_t {
    // TODO: Perform a deterministic unit of work for the GC
    (void)gc;
    return 0;
  }

  auto azide_gc_catchup(GC* gc) -> uint64_t {
    // TODO: Perform all pending work for the GC
    (void)gc;

    return 0;
  }

  auto azide_gc_malloc(GC* gc, size_t size, size_t align) -> void* {
    // TODO: Allocate memory with the GC
    (void)gc;
    (void)size;
    (void)align;

    return nullptr;
  }
}  // namespace azide::gc
