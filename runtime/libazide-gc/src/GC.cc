#include <azide-gc/GC.hh>
#include <boost/config.hpp>
#include <cassert>
#include <cstring>
#include <mutex>

namespace azide::gc {
  struct GC {
  private:
    mutable std::mutex m_mutex;  // Internal mutex for thread-safety

    bool m_enabled = false;

  public:
    Interface::Malloc m_malloc;
    void* m_malloc_m;

    Interface::Free m_free;
    void* m_free_m;

  private:
    Interface::AsyncFinalizer m_runner;
    void* m_runner_m;

    Interface::PauseTasks m_pause;
    void* m_pause_m;

    Interface::ResumeTasks m_resume;
    void* m_resume_m;

  public:
    GC(Interface support)
        : m_malloc(support.m_malloc),
          m_malloc_m(support.m_malloc_m),
          m_free(support.m_free),
          m_free_m(support.m_free_m),
          m_runner(support.m_runner),
          m_runner_m(support.m_runner_m),
          m_pause(support.m_pause),
          m_pause_m(support.m_pause_m),
          m_resume(support.m_resume),
          m_resume_m(support.m_resume_m) {
      (void)m_runner;
      (void)m_runner_m;
      (void)m_pause;
      (void)m_pause_m;
      (void)m_resume;
      (void)m_resume_m;
    }

    ~GC() {
      // TODO: Implement any necessary cleanup logic here.
    }

    [[nodiscard]] auto malloc(size_t size) -> void* { return m_malloc(m_malloc_m, size); }
    auto free(void* ptr) -> void { m_free(m_free_m, ptr); }

    auto lock() -> void { m_mutex.lock(); }
    auto unlock() -> void { m_mutex.unlock(); }

    auto critical_section(auto section) const -> decltype(section()) {
      std::lock_guard lock(m_mutex);
      return section();
    }

    [[nodiscard]] auto is_enabled() const -> bool { return m_enabled; }
    auto set_enabled(bool enabled) -> void { m_enabled = enabled; }
  };

  auto azide_gc_create(Interface support) -> GC* {
    if (const auto missing_required_callbacks = support.m_runner == nullptr     // AsyncFinalizer
                                                || support.m_malloc == nullptr  // Malloc
                                                || support.m_free == nullptr;   // Free
        missing_required_callbacks) [[unlikely]] {
      return nullptr;
    }

    void* v_ptr = support.m_malloc(support.m_malloc_m, sizeof(GC));
    if (v_ptr == nullptr) [[unlikely]] {
      return nullptr;
    }

    return new (v_ptr) GC(support);
  }

  auto azide_gc_destroy(GC* gc) -> void {
    if (gc == nullptr) {
      return;
    }

    gc->lock();

    const auto gc_free = gc->m_free;
    auto* gc_free_m = gc->m_free_m;

    gc->~GC();

    {  // Clear the object for safety
      const uint8_t sentinel_byte = 0xa3;
      memset(gc, sentinel_byte, sizeof(GC));
    }

    gc_free(gc_free_m, gc);
  }

  auto azide_gc_enable(GC* gc) -> void {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      gc->set_enabled(true);  //
    });
  }

  auto azide_gc_disable(GC* gc) -> void {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      gc->set_enabled(false);  //
    });
  }

  auto azide_gc_is_enabled(const GC* gc) -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      return gc->is_enabled();  //
    });
  }

  auto azide_gc_manage(GC* gc, void* base, size_t size) -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Manage the memory range
    (void)gc;
    (void)base;
    (void)size;

    return false;
  }

  auto azide_gc_unmanage(GC* gc, void* base, size_t size) -> void {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Unmanage the memory range
    (void)gc;
    (void)base;
    (void)size;
  }

  auto azide_gc_is_managed(GC* gc, void* base, size_t size) -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Check if the memory range is managed
    (void)gc;
    (void)base;
    (void)size;

    return false;
  }

  auto azide_gc_add_root(GC* gc, void** root) -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Add a root pointer
    (void)gc;
    (void)root;

    return false;
  }

  auto azide_gc_del_root(GC* gc, void** root) -> void {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Remove a root pointer
    (void)gc;
    (void)root;
  }

  auto azide_gc_notify(GC* gc, Event event, uint64_t p) -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    (void)p;

    switch (event) {
      case Event::TaskCreated: {
        // TODO: Handle task creation event
        return false;
      }

      case Event::TaskExited: {
        // TODO: Handle task exit event
        return false;
      }

      case Event::TaskBlocked: {
        // TODO: Handle task blocked event
        return false;
      }

      case Event::TaskUnblocked: {
        // TODO: Handle task unblocked event
        return false;
      }

      case Event::ObjectDestructed: {
        // TODO: Handle object destructed event
        return false;
      }

      default: {
        // Unknown event type, unhandlable
        return false;
      }
    }
  }

  auto azide_gc_step(GC* gc) -> uint64_t {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Perform a deterministic unit of work for the GC
    (void)gc;
    return 0;
  }

  auto azide_gc_catchup(GC* gc) -> uint64_t {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Perform all pending work for the GC
    (void)gc;

    return 0;
  }

  auto azide_gc_malloc(GC* gc, size_t size, size_t align) -> void* {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Allocate memory with the GC
    (void)gc;
    (void)size;
    (void)align;

    return nullptr;
  }
}  // namespace azide::gc
