# Overview

Dependency graph of the module pass system. Arrows indicate system dependencies. 

![Photo](https://github.com/user-attachments/assets/e7a2840d-b54a-4e12-a3c2-50ef847ec52d)

# QXIR Required Analysis Passes

| Pass Identifier | Pass Name                                | Summary                                                          | Implementation Notes              |
| --------------- | ---------------------------------------- | ---------------------------------------------------------------- | --------------------------------- |
| `ds-acyclic`    | Data structure acyclic verification      | Verifies that there are no cyclic references in the module.      | Hybrid (g0) deps []               |
| `ds-nullchk`    | Data structure null pointer verification | Verifies that the module does not contain illegal null pointers. | Hybrid (g0) deps []               |
| `ds-resolv`     | Data structure discovery                 | Builds a lookup table of construct names within the module.      | Standalone (g2) deps [`g0`, `g1`] |

---

# QXIR Required Transformative Passes

| Pass Identifier | Pass Name           | Summary                                                       | Implementation Notes                          |
| --------------- | ------------------- | ------------------------------------------------------------- | --------------------------------------------- |
| `ds-flatten`    | Function flattening | Flattens all nested functions within the module.              | Standalone (g4) deps [`g0`, `g1`, `g2`, `g3`] |
| `tyinfer`       | Type inference      | Applies type annotations to all constructs within the module. | Standalone (g5) deps [`g0`, `g1`, `g2`]       |
| `nm-premangle`  | Name Pre-mangling   | Adds some metadata to named constructs within the module.     | Standalone (g6) deps [`g0`, `g1`, `g2`, `g5`] |

