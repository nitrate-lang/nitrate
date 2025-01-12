# Nitrate Error Reporting

This directory contains error reporting templates for the Nitrate toolchain.

## Error Reporting

In general, a diagnostic consists of a diagnostic class, source-code location, timestamp, and a message. The diagnostic class is a C++ class representing what the issue. The diagnostic may have the following immutable properties:
- `Flag name` - Compiler flag that can be used to suppress the diagnostic. This field is the primary key of the diagnostic class. For example: `-Wno-unused-variable`.
- `Nice name` - A human-readable name for the diagnostic. For example: "Unused Variable".
- `Details` - A succinct description of the diagnostic. For example: "Some variables are declared but never used."
- `Tags` - A list of tags that can be used to categorize the diagnostic. For example: "style", "performance", "UB".
- `Fixes` - Suggestions for fixing the diagnostic. For example: "Remove the unused variable."
- `Examples` - Examples of code that triggers the diagnostic. For example: "fn(){let x=0;}
- `Developer notes` - Notes for developers working on the diagnostic. For example: "An edge case is when a variable is declared but never used in a macro."
- `User notes` - Any notes saved by the user. For example: "This diagnostic is annoying."

The abovementioned properties are stored in configuration files on disk and are loaded into memory when the Nitrate toolchain is initialized. The diagnostics are then used to generate error messages when the toolchain is run. This way, the error messages are consistent and can be easily updated. It decouples error-emission logic from formatting and style logic.
