# QUIX Language Server Implementation

This is the implementation of a language server for the QUIX programming language. The language server is implemented in C++20 and uses the [LSP](https://microsoft.github.io/language-server-protocol/) protocol.

## Progress on LSP Features

- ✅ Multi-threaded request handling and cancellation support
- 🚧 Did Open Text Document
- 🚧 Did Change Text Document
- 🚧 Will Save Text Document
- 🚧 Will Save Document Wait Until
- 🚧 Did Save Text Document
- 🚧 Did Close Text Document
- 🚧 Rename a Text Document
- 🚧 Overview - Notebook Document
- 🚧 Did Open Notebook Document
- 🚧 Did Change Notebook Document
- 🚧 Did Save Notebook Document
- 🚧 Did Close Notebook Document
- 🚧 Go to Declaration
- 🚧 Go to Definition
- 🚧 Go to Type Definition
- 🚧 Go to Implementation
- 🚧 Find References
- 🚧 Prepare Call Hierarchy
- 🚧 Call Hierarchy Incoming Calls
- 🚧 Call Hierarchy Outgoing Calls
- 🚧 Prepare Type Hierarchy
- 🚧 Type Hierarchy Super Types
- 🚧 Type Hierarchy Sub Types
- 🚧 Document Highlight
- 🚧 Document Link
- 🚧 Document Link Resolve
- 🚧 Hover
- 🚧 Code Lens
- 🚧 Code Lens Refresh
- 🚧 Folding Range
- 🚧 Selection Range
- 🚧 Document Symbols
- 🚧 Semantic Tokens
- 🚧 Inline Value
- 🚧 Inline Value Refresh
- 🚧 Inlay Hint
- 🚧 Inlay Hint Resolve
- 🚧 Inlay Hint Refresh
- 🚧 Moniker
- 🚧 Completion Proposals
- 🚧 Completion Item Resolve
- 🚧 Publish Diagnostics
- 🚧 Pull Diagnostics
- 🚧 Signature Help
- 🚧 Code Action
- 🚧 Code Action Resolve
- 🚧 Document Color
- 🚧 Color Presentation
- 🚧 Formatting
- 🚧 Range Formatting
- 🚧 On type Formatting
- 🚧 Rename
- 🚧 Prepare Rename
- 🚧 Linked Editing Range

## Building

- The language server is built automatically when building the QUIX toolchain. 
  Use the build tool provided in the root of the repository. 
- Instructions for standalone building is are documented.
