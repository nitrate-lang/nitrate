# Nitrate Preprocessor WASM Plugin Manager

# Future Vision

The powerful and flexible Nitrate Preprocessor WASM Plugin Manager will enable compiler specialists to create and deploy plugins (or preprocessor loadable modules) into the Nitrate Preprocessor. These plugins will be able to render additional functionality in the form of QSysCalls invokable from within Nitrate source code. Plugins will work similarly to the way that kernel modules work. That is to say, they will be loaded dynamically into the compiler at runtime, and will reside on the users installation of the Nitrate toolchain. Modules will be updated and maintained by the community, and will be able to be installed and removed at will.

## Capabilities

The WASM modules will be unconstrained in their capabilities, they will be able to read and write to compiler main memory, and run will the full privileges of the compiler / user. Sandboxing is only provided for stability, not at all for security**.
