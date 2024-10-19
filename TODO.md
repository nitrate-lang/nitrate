# Project TODO List

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
