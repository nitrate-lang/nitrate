# Nitrate: Semantic Analysis

This document provides an overview of the capabilities, performance, and completeness of the Nitrate language semantic analysis system.

### Definition:
For our purposes, semantic analysis is the process of automated quality assurance that helps catch some bugs and bad practices that you will probably miss, ignore, or just not want to think about.

### Overview
Developing a diagnostic system is difficult. Developing a general and correct diagnostic system is impossible because of [Rice's Theorem](https://en.wikipedia.org/wiki/Rice%27s_theorem).
With that in mind, the Nitrate diagnostic system, hereinafter referred to as an ("Analyzer"), focuses on practical checks that can be implemented efficiently, striving to minimize diagnostic false-positives and maximize value creation for the programmer, thereby easing development.

### Performance Trade offs
Diagnosis passes generally have at least `O(n)` best-case time complexity.

> **Informal proof**: Most diagnostic passes will attempt to determine, in the furtherance of semantic diagnosis, at least one semantic property per-function by checking all functions in the program. Assuming no caching and there are `n` functions, then this iteration will require at least `n` steps (which might be parallelizable). Therefore, every pass will have, at least, `O(n)` best-case time complexity.

So it follows, the compile times will increase proportionally to the number of diagnostic checks ran. 

Compiler latency, that is how long it takes to synthesize your hard earned executable, can be decreased by running semantic analysis passes asynchronously. The command (`no3 build --async-diagnosis`) can be used to this end.


### Classification of Bugs
TODO: Section

### Classification of Diagnostics
TODO: Section
