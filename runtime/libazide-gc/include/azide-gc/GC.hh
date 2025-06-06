#ifndef __AZIDE_GC_GC_H__
#define __AZIDE_GC_GC_H__

#include <array>
#include <cstddef>
#include <cstdint>

namespace azide::gc {
  /**
   * @brief Opaque handle representing a garbage collector instance.
   *
   * This structure contains an internal mutex for thread-safety.
   * Unless otherwise specified, all exported functions that operate
   * on this structure are thread-safe.
   */
  struct GC;

  struct TaskAPI {
    using Suspend = void (*)(void* user_data, uint64_t task_id);
    using Resume = void (*)(void* user_data, uint64_t task_id);

    Suspend m_suspend = nullptr;
    Resume m_resume = nullptr;
    void* m_user_data = nullptr;
  };

  struct AsyncFinalization {
    using Runner = void (*)(void* user_data, void* base, size_t size, uint64_t object_id);

    Runner m_runner = nullptr;
    void* m_user_data = nullptr;
  };

  /**
   * @brief Creates and initializes a new garbage collector instance.
   *
   * This function allocates and returns a pointer to a new GC (Garbage Collector) object.
   * The returned GC instance is ready for use in memory management operations.
   *
   * @return GC* Pointer to the newly created garbage collector instance.
   *         Returns nullptr if creation fails.
   *
   * @note The caller is responsible for managing the lifetime of the returned GC instance.
   * @see azide_gc_destroy for cleanup.
   *
   * @note This function is thread-safe.
   */
  extern "C" [[nodiscard]] auto azide_gc_create(TaskAPI task_api, AsyncFinalization runner) -> GC*;

  /**
   * @brief Destroys the specified garbage collector instance and releases all
   *        associated resources.
   *
   * This function should be called when the GC instance is no longer needed.
   * After calling this function, the provided GC pointer must **not** be used.
   *
   * @param gc Pointer to the GC instance to destroy.
   * @note The GC instance must have been created with azide_gc_create.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" auto azide_gc_destroy(GC* gc) -> void;

  /**
   * @brief Enables the garbage collector for the specified GC instance.
   *
   * This function temporarily enables automatic garbage collection operations
   * for the provided GC instance. When enabled, the garbage collector will
   * automatically manage memory by reclaiming unused objects and resources.
   * If the GC instance is already enabled, this function has no effect.
   *
   * @param gc Pointer to the GC instance to enable.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" auto azide_gc_enable(GC* gc) -> void;

  /**
   * @brief Disables the garbage collector for the specified GC instance.
   *
   * This function temporarily disables automatic garbage collection operations
   * for the provided GC instance. While disabled, no garbage collection cycles
   * will be triggered. This can be useful for performance-critical sections
   * where garbage collection pauses are undesirable.
   *
   * @param gc Pointer to the GC instance to disable.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" auto azide_gc_disable(GC* gc) -> void;

  /**
   * @brief Checks if the garbage collector is currently enabled.
   *
   * @param gc Pointer to the GC (Garbage Collector) instance.
   * @return true if the garbage collector is enabled, false otherwise.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" [[nodiscard]] auto azide_gc_is_enabled(const GC* gc) -> bool;

  /**
   * @brief Registers a memory range with the garbage collector.
   *
   * Informs the garbage collector (`gc`) that all bytes within the specified
   * memory range, starting at `base` and spanning `size`, should be managed.
   * This means that the GC will track these bytes and reclaim them when they are
   * no longer in use, allowing for automatic memory management.
   *
   * The memory range *need not* be valid upon registration, but it must be
   * valid before any calls to `azide_gc_step`.
   *
   * This operation will complete successfully, regardless of whether the GC is
   * enabled or disabled.
   *
   * This function will ignore any byte within the specified range that is
   * already managed by the GC. If the entire range is already managed, the function
   * will simply have no effect. No internal reference counting is performed for
   * for any byte within the range, so be careful to avoid premature calls to
   * `azide_gc_unmanage`, particularly when `azide_gc_manage` is called multiple times
   * for overlapping ranges.
   *
   * @param gc   Pointer to the garbage collector instance.
   * @param base Pointer to the start of the memory range to be managed.
   * @param size Size (in bytes) of the memory range to be managed.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" auto azide_gc_manage(GC* gc, void* base, size_t size) -> void;

  /**
   * @brief Removes a memory range from garbage collection management.
   *
   * Informs the garbage collector (`gc`) that all bytes within the specified
   * memory range, starting at `base` and spanning `size`, should no longer be managed.
   * After this call, the GC will no longer track or automatically reclaim
   * any memory within this range.
   *
   * This function will ignore any byte within the specified range that is not
   * currently managed by the GC. If the entire range is not managed, the function
   * will simply have no effect.
   *
   * @param gc   Pointer to the GC instance managing the memory.
   * @param base Pointer to the base address of the memory range to unmanage.
   * @param size Size (in bytes) of the memory range to unmanage.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" auto azide_gc_unmanage(GC* gc, void* base, size_t size) -> void;

  /**
   * @brief Checks if a memory range is managed by the given garbage collector.
   *
   * This function determines whether the specified memory range, in its entirety,
   * defined by a base pointer and size, is currently managed by the provided GC instance.
   *
   * @param gc   Pointer to the garbage collector instance.
   * @param base Pointer to the start of the memory range to check.
   * @param size Size (in bytes) of the memory range to check.
   * @return true if the memory range is managed by the garbage collector, false otherwise.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" [[nodiscard]] auto azide_gc_is_managed(GC* gc, void* base, size_t size) -> bool;

  extern "C" auto azide_gc_add_task(GC* gc, uint64_t task_id) -> void;
  extern "C" auto azide_gc_del_task(GC* gc, uint64_t task_id) -> void;

  extern "C" auto azide_gc_destroyed(GC* gc, uint64_t object_id) -> void;

  extern "C" auto azide_gc_step(GC* gc) -> uint64_t;

  extern "C" auto azide_gc_malloc(GC* gc, size_t size, size_t align) -> void*;
  extern "C" auto azide_gc_sizeof(GC* gc, const void* ptr) -> size_t;
}  // namespace azide::gc

#endif

static void test() {
  using namespace azide::gc;

  auto* gc = azide_gc_create(
      {
          .m_suspend = nullptr, /* Suspend callback */
          .m_resume = nullptr,  /* Resume callback */
      },
      {
          .m_runner = nullptr, /* Async finalization callback */
      });
  if (gc == nullptr) {
    // Handle error: GC creation failed
    return;
  }

  const auto heap_size = 1024 * 1024;  // 1 MB heap
  std::array<uint8_t, heap_size> heap;

  azide_gc_manage(gc, heap.data(), heap.size());
  azide_gc_add_task(gc, 1);
  azide_gc_add_task(gc, 2);
  azide_gc_add_task(gc, 3);
  azide_gc_enable(gc);

  azide_gc_step(gc);

  azide_gc_destroy(gc);
}