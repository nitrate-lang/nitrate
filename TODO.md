# Project TODO List

fn-flatten pass broke QIR_NODE_EXTERN, need to fix.

---

Write a facade library named `libquix`.
```c
// Library initialization is handled automatically by the library.
bool quix_cc(FILE *source, FILE *output, bool (*diag_cb)(const char *utf8_message, const char *from_subsystem),
             const char *options[]);

/// @brief Deinitialize the library.
void quix_deinit(void);
```

---

- Garbage Collection
  - Summary:
    - A runtime-less garbage collection system that will have the collector invoked
      before every function that allocates memory returns to the caller. The collector
      will store a counter that will be incremented it is called. When the counter
      reaches a certain threshold, the collector will precede will the collection
      otherwise it will return immediately. The threshold will be runtime programmable
      or statically set. The collector can also be invoked manually that as in functions that do 
      not allocate directly or indirectly (ex: pure math functions).
    - Metadata:
      - Priority: low
      - Complexity: significant
      - confidence_of_implementation: low
      - confidence_of_integration: low
    - Requirements:
      - Work on bare metal systems without any OS or system call services.
      Unsure at this time.
