# Project TODO List

---

- Nitrate IR dev:
  - A lot of work to do on IR dev. 
    - Formalize the IR expression nodes and semantics.
    - Add a way to register IR passes at runtime.
    - Serialize and deserialize IR
    - Add a builder API to generate IR like llvm::Builder.
    - Make IR synthesis like: [parse-tree -> quasi-IR -> IR -> IR-finalization passes -> IR];
      - Finalization passes include:
        - Name mangling
        - Final checks on the data structure itself
        - Others as I think of them.

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
