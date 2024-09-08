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
