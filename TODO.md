# Project TODO List

- Diagnostic data bus system
  - Summary:
    - A diagnostic message bus system will be implemented to allow project
      libraries to communicate over a common asynchronous message bus for
      subsystem invariant feedback control.
  - Metadata:
    - Priority: low
    - Complexity: none
    - confidence_of_implementation: absolute
    - confidence_of_integration: mid
  - Requirements:
    - Every library will be able to publish messages with a simple C API [debug, info, warning, ...].
    - Support format strings.
    - Message rendering shall be unspecific in the general case.
    - Flag to flush the message immediately upon receipt.
    - Thread safe.
    - Shared and static libraries shall be supported.
    - The qmsg_t context will be shared by all publishers and subscribers.
    - Each thread may have many qmsg_t contexts coexisting.
    - Macros for easy message publishing.

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
