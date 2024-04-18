---
layout: page
title: Source File Encoding
permalink: /docs/source-file-encoding/
parent: Documentation
nav_order: 1
---

Source file encoding
---

- All QUIX source files must be encoded in UTF-8. 
- Newlines must be encoded as LF (line feed) characters (0x0A). 
- White space characters are not significant in QUIX, the compiler will only use them as separators between tokens. QUIX is a whitespace insensitive language, so the compiler will ignore any white space characters that are not necessary for tokenization.
- QUIX requires that an additional newline character be present at the end of the file.