# QXIR Required Analysis Passes

| Pass Identifier | Pass Name                                    | Summary                                                                 | Implementation Notes                                        |
|-----------------|----------------------------------------------|-------------------------------------------------------------------------|-------------------------------------------------------------|
| `ds-acyclic`    | Data structure acyclic verification          | Verifies that there are no cyclic references in the module.             | Hybrid (g0) deps []                                         |
| `ds-nilchk`     | Data structure null pointer verification     | Verifies that the module does not contain illegal null pointers.        | Hybrid (g0) deps []                                         |
| `ds-chtype`     | Data structure child type                    | Verifies all nodes identity is permitted with respect to the parent.    | Standalone (g1) deps [`g0`]                                 |
| `ds-discov`     | Data structure discovery                     | Builds a lookup table of construct names within the module.             | Standalone (g2) deps [`g0`, `g1`]                           |

---

# QXIR Required Transformative Passes

| Pass Identifier | Pass Name                                    | Summary                                                                 | Implementation Notes                                        |
|-----------------|----------------------------------------------|-------------------------------------------------------------------------|-------------------------------------------------------------|
| `ns-flatten`    | Namespace flattening                         | Flattens all namespaces within the module.                              | Standalone (g3) deps [`g0`, `g1`, `g2`]                     |
| `fnflatten`     | Function flattening                          | Flattens all nested functions within the module.                        | Standalone (g4) deps [`g0`, `g1`, `g2`, `g3`]               |
| `tyinfer`       | Type inference                               | Applies type annotations to all constructs within the module.           | Standalone (g5) deps [`g0`, `g1`, `g2`]                     |
| `nm-premangle`  | Name Pre-mangling                            | Adds some metadata to named names within the module.                    | Standalone (g5) deps [`g0`, `g1`, `g2`, `g5`]               |
