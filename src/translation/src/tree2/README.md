# Full-fidelity Concrete Syntax Tree (CST)

This crate provides an alternative CST representation (`tree2`) with full-fidelity source information, used by the newer parser (`tree_parse2`).

## Dependencies

All dependencies are workspace-managed from the root `Cargo.toml`:

```toml
# excerpt from Cargo.toml
[dependencies]
nitrate_nstring.workspace = true
nitrate_diagnosis.workspace = true
nitrate_token.workspace = true
nitrate_token_lexer.workspace = true
ordered-float.workspace = true
serde = { workspace = true, features = ["derive", "rc"] }
append-only-vec.workspace = true
thin-vec = { workspace = true, features = ["serde"] }
bitflags.workspace = true
```
