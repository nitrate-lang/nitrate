---
layout: page
title: Source File Encoding
permalink: /docs/source-file-encoding/
parent: Documentation
nav_order: 1
---

Source file encoding
---

All QUIX source files must be encoded in UTF-8. Newlines must be encoded as LF (line feed) characters (0x0A). Basically, source files should be a Linux UTF-8 text file.

QUIX requires that an additional newline character be present at the end of the file. This may be handled automatically by the compiler, but it is a requirement for the source file to be considered truely valid.