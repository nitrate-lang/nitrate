#ifndef __AZIDE_GC_GC_H__
#define __AZIDE_GC_GC_H__

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

  struct Interface {
    using PauseTasks = void (*)(void* m);
    using ResumeTasks = void (*)(void* m);
    using AsyncFinalizer = void (*)(void* m, void* base, size_t size, uint64_t object_id);
    using Malloc = void* (*)(void* m, size_t size);
    using Free = void (*)(void* m, void* ptr);

    PauseTasks m_pause = nullptr;
    void* m_pause_m = nullptr;

    ResumeTasks m_resume = nullptr;
    void* m_resume_m = nullptr;

    AsyncFinalizer m_runner = nullptr;
    void* m_runner_m = nullptr;

    Malloc m_malloc = nullptr;
    void* m_malloc_m = nullptr;

    Free m_free = nullptr;
    void* m_free_m = nullptr;
  };

  /**
   * @brief Creates and initializes a new garbage collector instance.
   *
   * This function allocates and returns a pointer to a new GC (Garbage Collector) object.
   * The returned GC instance is ready for use in memory management operations.
   *
   * The GC instance will be in the disabled state upon creation.
   *
   * @param support An Interface structure that provided callbacks for the GC to use.
   *                This dependency injection allows the GC to work without linking
   *                against any specific threading library or the standard library.
   *
   * @return GC* Pointer to the newly created garbage collector instance.
   *         Returns nullptr if creation fails.
   *
   * @note The caller is responsible for managing the lifetime of the returned GC instance.
   * @see azide_gc_destroy for cleanup.
   *
   * @note This function is thread-safe.
   */
  extern "C" [[nodiscard]] auto azide_gc_create(Interface support) noexcept -> GC*;

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
  extern "C" auto azide_gc_destroy(GC* gc) noexcept -> void;

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
  extern "C" auto azide_gc_enable(GC* gc) noexcept -> void;

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
  extern "C" auto azide_gc_disable(GC* gc) noexcept -> void;

  /**
   * @brief Checks if the garbage collector is currently enabled.
   *
   * @param gc Pointer to the GC (Garbage Collector) instance.
   * @return true if the garbage collector is enabled, false otherwise.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" [[nodiscard]] auto azide_gc_is_enabled(const GC* gc) noexcept -> bool;

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
   * This operation will complete, regardless of whether the GC is enabled or disabled.
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
   * @return true if the memory range was successfully registered, false otherwise.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" [[nodiscard]] auto azide_gc_manage(GC* gc, void* base, size_t size) noexcept -> bool;

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
  extern "C" auto azide_gc_unmanage(GC* gc, void* base, size_t size) noexcept -> void;

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
  extern "C" [[nodiscard]] auto azide_gc_is_managed(GC* gc, void* base, size_t size) noexcept -> bool;

  /**
   * @brief Registers a root pointer with the garbage collector.
   *
   * This function adds a pointer to a root variable, preventing the garbage collector
   * from reclaiming any memory that is reachable from this root. The root should
   * remain valid for the lifetime of the garbage collector or until it is explicitly
   * removed.
   *
   * @see azide_gc_del_root for removing a root.
   *
   * @param gc   Pointer to the garbage collector instance.
   * @param root Address of the root pointer to be registered.
   * @return true if the root was successfully added; false otherwise.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" [[nodiscard]] auto azide_gc_add_root(GC* gc, void** root) noexcept -> bool;

  /**
   * @brief Removes a root pointer from the garbage collector's root set.
   *
   * This function unregisters a previously registered root pointer from the
   * garbage collector, so that the memory it points to is no longer considered
   * reachable and may be collected if not referenced elsewhere. If the root pointer
   * is not found in the root set, this function has no effect.
   *
   * @param gc   Pointer to the garbage collector instance.
   * @param root Address of the root pointer to be removed from the root set.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" auto azide_gc_del_root(GC* gc, void** root) noexcept -> void;

  /**
   * @brief Enumeration of events that can be notified to the garbage collector.
   *
   * These events are used to inform the garbage collector about significant
   * occurrences in the system, such as task management or object finalization.
   * The GC requires these notifications to function correctly, as it may
   * need to adjust its behavior based on the current state of tasks and objects.
   *
   * The values of this enumeration are used as parameters in the azide_gc_notify function.
   */
  enum class Event : uint8_t {
    /**
     * @brief New coroutine/task has been created.
     *
     * This event must be notified after the task id becomes valid,
     * but before the task starts running. This is typically done in
     * the task's constructor or initialization phase.
     *
     * @param p The task id of the newly created task. This id is used as an opaque handle
     *          to refer to the task in subsequent notifications. The handle need not be
     *          a OS thread id, or any particular type, but must be unique
     *          within the context of the GC instance.
     */
    TaskCreated = 0,

    /**
     * @brief Task exiting event.
     *
     * This event must be notified right before the task terminates.
     * This task_id must still be valid, and must match the one used
     * when the task was created with TaskCreated.
     *
     * @param p The task id of the task that is exiting.
     */
    TaskExited = 1,

    /**
     * @brief Task blocked event.
     *
     * This event must be notified when a task is blocked, meaning it is waiting
     * for a resource or condition to become available before it can continue execution.
     * If the `TaskExited` has been received for the task_id, this function is a no-op.
     * If the task is already blocked, this function is a no-op as well.
     *
     * @param p The task id of the task that is blocked. This id must match the one used
     *          when the task was created with TaskCreated.
     */
    TaskBlocked = 2,

    /**
     * @brief Task unblocked event.
     *
     * This event must be notified when a previously blocked task is now able to continue execution,
     * meaning the resource or condition it was waiting for has become available.
     * If the `TaskExited` has been received for the task_id, this function is a no-op.
     * If the task is not blocked, this function is a no-op as well.
     *
     * @param p The task id of the task that is unblocked. This id must match the one used
     *          when the task was created with TaskCreated.
     */
    TaskUnblocked = 3,

    /**
     * @brief Asynchronous finalization event.
     *
     * This event must be notified when an object is finalized and is therefore
     * able to be deallocated.
     *
     * @param p The object id of the finalized object. This is the same id provided to
     * the m_runner callback's `object_id` parameter when the GC was created.
     */
    ObjectDestructed = 10,
  };

  /**
   * @brief Notifies the garbage collector of an event.
   *
   * This function is used to inform the garbage collector about specific events
   * that may affect its operation, such as task management or object finalization.
   *
   * @param gc   Pointer to the GC instance.
   * @param event The event type to notify the GC about.
   * @param x    An additional parameter that may be used to provide context for the event.
   *             The meaning of this parameter depends on the event type.
   * @return true if the notification was successfully processed, false otherwise.
   *
   * @note This function is **thread-safe**, including with respect to the GC instance.
   */
  extern "C" [[nodiscard]] auto azide_gc_notify(GC* gc, Event event, uint64_t p) noexcept -> bool;

  extern "C" auto azide_gc_step(GC* gc) noexcept -> uint64_t;
  extern "C" auto azide_gc_catchup(GC* gc) noexcept -> uint64_t;

  extern "C" auto azide_gc_malloc(GC* gc, size_t size, size_t align) noexcept -> void*;
}  // namespace azide::gc

#endif
