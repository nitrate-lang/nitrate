#include <algorithm>
#include <array>
#include <atomic>
#include <azide-gc/GC.hh>
#include <boost/config.hpp>
#include <cassert>
#include <cstring>
#include <memory_resource>
#include <mutex>

namespace std {
  void __throw_bad_array_new_length() { __builtin_trap(); }
  void __throw_length_error(const char*) { __builtin_trap(); }
}  // namespace std

namespace azide::gc {
  class SpinLock {
    std::atomic<bool> m_locked = false;

  public:
    void lock() {
      while (m_locked.exchange(true, std::memory_order_acquire)) {
        // Spin-wait (busy-wait)
      }
    }

    void unlock() { m_locked.store(false, std::memory_order_release); }
  };

  struct GC {
  private:
    mutable SpinLock m_mutex;
    bool m_enabled = false;

  public:
    Interface::Malloc m_malloc;
    void* m_malloc_m;

    Interface::Free m_free;
    void* m_free_m;

    Interface::AsyncFinalizer m_runner;
    void* m_runner_m;

    Interface::PauseTasks m_pause;
    void* m_pause_m;

    Interface::ResumeTasks m_resume;
    void* m_resume_m;

    struct ManagedRange {
      uint8_t* m_base = nullptr;
      size_t m_size = 0;
    };

    static constexpr size_t MAX_MANAGED_RANGES = 20;

    std::array<ManagedRange, MAX_MANAGED_RANGES> m_managed_ranges;

    std::pmr::vector<void**> m_roots;

    GC(Interface support, std::pmr::memory_resource* resource)
        : m_malloc(support.m_malloc),
          m_malloc_m(support.m_malloc_m),
          m_free(support.m_free),
          m_free_m(support.m_free_m),
          m_runner(support.m_runner),
          m_runner_m(support.m_runner_m),
          m_pause(support.m_pause),
          m_pause_m(support.m_pause_m),
          m_resume(support.m_resume),
          m_resume_m(support.m_resume_m),
          m_managed_ranges(),
          m_roots(resource) {
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

    auto lock() const -> void { m_mutex.lock(); }
    auto unlock() const -> void { m_mutex.unlock(); }

    [[nodiscard]] auto critical_section(auto section) const -> decltype(section()) {
      std::lock_guard lock(*this);
      return section();
    }

    [[nodiscard]] auto is_enabled() const -> bool { return m_enabled; }
    auto set_enabled(bool enabled) -> void { m_enabled = enabled; }
  };

  BOOST_SYMBOL_EXPORT auto azide_gc_create(Interface support) noexcept -> GC* {
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

    // FIXME: Use the m_malloc, m_free in a pmr allocator
    std::pmr::memory_resource* resource = std::pmr::null_memory_resource();

    return new (v_ptr) GC(support, resource);
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_destroy(GC* gc) noexcept -> void {
    if (gc == nullptr) {
      return;
    }

    gc->lock();

    const auto gc_free = gc->m_free;
    auto* gc_free_m = gc->m_free_m;

    gc->~GC();

    {  // Clear the object for safety
      const uint8_t sentinel_byte = 0xa3;
      memset(static_cast<void*>(gc), sentinel_byte, sizeof(GC));
    }

    gc_free(gc_free_m, gc);
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_enable(GC* gc) noexcept -> void {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      gc->set_enabled(true);  //
    });
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_disable(GC* gc) noexcept -> void {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      gc->set_enabled(false);  //
    });
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_is_enabled(const GC* gc) noexcept -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      return gc->is_enabled();  //
    });
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_manage(GC* gc, void* base, size_t size) noexcept -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Manage the memory range
    (void)gc;
    (void)base;
    (void)size;

    return false;
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_unmanage(GC* gc, void* base, size_t size) noexcept -> void {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Unmanage the memory range
    (void)gc;
    (void)base;
    (void)size;
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_is_managed(GC* gc, void* base, size_t size) noexcept -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      const auto* check_start = static_cast<uint8_t*>(base);
      const auto* check_end = check_start + size;

      return std::ranges::any_of(gc->m_managed_ranges, [&](const auto& range) {
        if (range.m_base == nullptr || range.m_size == 0) {
          return false;  // Skip uninitialized ranges
        }

        // Check if the range overlaps with the managed range
        const auto* range_start = range.m_base;
        const auto* range_end = range_start + range.m_size;

        return (check_start >= range_start && check_end <= range_end);
      });
    });
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_add_root(GC* gc, void** root) noexcept -> bool {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      // FIXME: Handle allocator error
      gc->m_roots.push_back(root);

      return true;
    });
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_del_root(GC* gc, void** root) noexcept -> void {
    assert(gc != nullptr && "GC instance must not be null");

    return gc->critical_section([&] {
      auto it = std::find(gc->m_roots.begin(), gc->m_roots.end(), root);
      if (it != gc->m_roots.end()) {
        gc->m_roots.erase(it);
      }
    });
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_notify(GC* gc, Event event, uint64_t p) noexcept -> bool {
    assert(gc != nullptr && "GC instance must not be null");
    (void)gc;

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

  BOOST_SYMBOL_EXPORT auto azide_gc_step(GC* gc) noexcept -> uint64_t {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Perform a deterministic unit of work for the GC
    (void)gc;
    return 0;
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_catchup(GC* gc) noexcept -> uint64_t {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Perform all pending work for the GC
    (void)gc;

    return 0;
  }

  BOOST_SYMBOL_EXPORT auto azide_gc_malloc(GC* gc, size_t size, size_t align) noexcept -> void* {
    assert(gc != nullptr && "GC instance must not be null");

    // TODO: Allocate memory with the GC
    (void)gc;
    (void)size;
    (void)align;

    return nullptr;
  }
}  // namespace azide::gc
