# hir_solve2 Audit Report

This document catalogs all issues found during a comprehensive, function-by-function audit of the `hir_solve2` crate. Issues are organized by file and function. No fixes have been applied.

**Fix Progress: 24 / 57 resolved**

---

## lib.rs

- ### 1. `Result<(), ()>` error type silences clippy

- **Severity**: Low (code smell)
- **Description**: The crate uses `Result<(), ()>` throughout its public API (`resolve_function`, `resolve_global`). The unit error type conveys no information about what went wrong. The clippy lint is silenced rather than fixed. Callers cannot distinguish between different failure modes.
- **Recommendation**: Use a proper error type (e.g., an enum) or at minimum a `&'static str` to indicate failure reason.

---

## bounds.rs

- ### ✅ 2. `Bounds::signed` casts negative `hi` i128 to u128 without checking

- **Severity**: Medium
- **Description**: `Self { lo, hi: hi as u128 }` — if `hi` is negative (which is invalid for an upper bound), the `as u128` cast silently wraps to a huge value (e.g., `-1i128 as u128 == u128::MAX`). No debug assertion or error guards against this.
- **Recommendation**: Add `debug_assert!(hi >= 0, "signed upper bound must be non-negative")`.

- ### ✅ 3. `Bounds::unsigned` wraps `lo > i128::MAX` to negative i128

- **Severity**: Medium
- **Description**: `Self { lo: lo as i128, hi }` — if `lo > i128::MAX`, the cast wraps to a negative `i128`, corrupting the lower bound. No assertion guards against this.
- **Recommendation**: Add `debug_assert!(lo <= i128::MAX as u128, "unsigned lower bound exceeds i128 range")`.

- ### 4. `numeric_bounds_for_type` hardcodes `USize` as 64-bit

- **Severity**: Low (the compiler may only target 64-bit)
- **Description**: `USize` always returns `Bounds::unsigned(0, 18_446_744_073_709_551_615)` (u64::MAX). On a 32-bit target, `USize` would be 32 bits wide.
- **Recommendation**: Use the architecture pointer size from the symbol table or target config.

- ### ✅ 5. `lit_to_i128` silently wraps unsigned values > i128::MAX

- **Severity**: Medium
- **Description**: All unsigned literal variants (`U8` through `U128`, `USize`) are cast to `i128` via `as i128`. Values above `i128::MAX` silently wrap to negative numbers. The `Bounds` struct stores `lo` as `i128`, so this is inherent to the design, but the silent wrapping could produce incorrect bound computations (e.g., `lit_to_i128(&Lit::U128(u128::MAX))` returns `-1`).
- **Recommendation**: Document this limitation clearly. Consider saturating at `i128::MAX` instead of wrapping.

- ### ✅ 6. `lit_to_u128` silently wraps negative i128 values

- **Severity**: Medium
- **Description**: Signed literal variants (`I8` through `I128`) are cast to `u128` via `as u128`. Negative values wrap to large unsigned numbers (e.g., `-1i8 as u128 == u128::MAX`). This function is used by `extract_bounds_from_type` and `check_literal_against_refinement`, which expect the max of a refinement type to be a valid upper bound.
- **Recommendation**: Use `TryFrom` or clamp negative values to 0, or redesign the bound representation.

- ### ✅ 7. `compute_binary_bounds` — `i128::MIN.abs()` panic in Mod handling

- **Severity**: **Critical — compiler panic**
- **Description**: `let a = std::cmp::max(r_min.abs(), r_max_i128.abs());` — if `r_min` (the lower bound of the divisor) is `i128::MIN`, calling `.abs()` panics with overflow because `i128::MIN` has no positive representation in `i128`. This can be triggered by user code like `x % y` where `y` has refinement bounds that include `i128::MIN`.
- **Recommendation**: Use `r_min.checked_abs().unwrap_or(i128::MAX)` or `saturating_abs()`.

- ### ✅ 8. `compute_binary_bounds` — `hi` to `i128` cast wraps for large unsigned values

- **Severity**: Medium
- **Description**: `let (l_min, l_max_i128) = (left.lo, left.hi as i128);` — `left.hi` is `u128`. When `left.hi > i128::MAX`, the cast wraps to a negative `i128`. This `l_max_i128` is then used in saturating arithmetic throughout the function, producing incorrect bounds for operations on values whose upper bound exceeds `i128::MAX`.
- **Recommendation**: Clamp at `i128::MAX` instead of wrapping: `left.hi.min(i128::MAX as u128) as i128`.

- ### ✅ 9. `compute_binary_bounds` — Sub: unsigned path uses `right.lo as u128` which wraps negative values

- **Severity**: Medium
- **Description**: `if left.hi >= right.lo as u128` — `right.lo` is `i128`. If `right.lo` is negative (e.g., from a value that's actually a large unsigned wrapped to negative via `Bounds::unsigned`), `as u128` produces a huge value, making the comparison false when it should be true.
- **Recommendation**: Check for negativity before casting, or restructure the bounds representation.

- ### ✅ 10. `compute_binary_bounds` — Sub: casts result of signed subtraction to u128

- **Severity**: Medium
- **Description**: `l_max_i128.saturating_sub(r_min) as u128` — if the subtraction saturates (or produces a negative result), the `as u128` cast wraps. This could produce a wildly incorrect upper bound.
- **Recommendation**: Clamp to 0 before casting: `l_max_i128.saturating_sub(r_min).max(0) as u128`.

- ### ✅ 11. `compute_binary_bounds` — Mul: mixes i128 products for min with u128 products for max

- **Severity**: Medium
- **Description**: When `is_unsigned_both` is true, the function uses `min_i128` from the i128 products (which may be negative due to the `hi as i128` cast) and `max_u128` from the u128 products. The lower bound should come from the u128 products as well when the values are known to be unsigned.
- **Recommendation**: When `is_unsigned_both` is true, compute both min and max from `products_u128`.

- ### 12. `compute_binary_bounds` — Div: division by -1 can overflow (mitigated by saturating)

- **Severity**: Low (correctly uses saturating_div)
- **Description**: `l.saturating_div(-1)` for `l = i128::MIN` produces `i128::MAX` (saturation), which is a reasonable approximation. However, the user won't be informed that `i128::MIN / -1` would overflow at runtime.
- **Recommendation**: Consider emitting a warning diagnostic for this case.

- ### ✅ 13. `compute_binary_bounds` — Shr: `.max(1)` overestimates upper bound

- **Severity**: Low
- **Description**: `left.hi.checked_shr(shift).unwrap_or(0).max(1)` — if the shifted value is 0, it's forced to 1. This means a value that can only be 0 after shifting is reported as having an upper bound of 1. This could mask refinement errors.
- **Recommendation**: Remove `.max(1)` or document the rationale.

- ### ✅ 14. `compute_binary_bounds` — Shr: reads `right.lo as i128` which may already be wrapped

- **Severity**: Medium
- **Description**: `right.lo as i128` — but `right.lo` is already `i128`. The `as i128` is a no-op. If the original `Bounds::unsigned` call wrapped a large value, this check is subtly wrong. The code compares `right.lo as i128 > 0` but `right.lo` IS an i128.
- **Recommendation**: The `as i128` is extraneous but harmless. The variable naming (`right.lo` being i128) is confusing in this context.

- ### ✅ 15. `compute_unary_bounds` — Negation: tangled sign handling

- **Severity**: Medium
- **Description**: The negation bounds computation goes through `max_i128 = max as i128` (which wraps for large unsigned). Then `max_i128.saturating_neg()` computes the negation. For an unsigned value with `hi > i128::MAX`, `max_i128` wraps negative, and `saturating_neg()` of a negative value is positive — this chain of conversions makes the result difficult to reason about.
- **Recommendation**: Separate signed and unsigned negation paths.

- ### ✅ 16. `compute_unary_bounds` — Not: swapped bounds logic is incorrect for unsigned

- **Severity**: Medium
- **Description**: `Bounds::new(!max_i128, (!min) as u128)` — for unsigned types, bitwise NOT bounds should be `[!hi, !lo]`, but here `max_i128` may be a wrapped negative value. The bound computation assumes signed semantics.
- **Recommendation**: Use unsigned bitwise NOT when the operand is unsigned.

- ### ✅ 17. `check_bounds_against_constraint` returns `true` when bounds extraction fails

- **Severity**: Low
- **Description**: If `extract_bounds_from_type(constraint_ty)` returns `None`, the function returns `true` (meaning "passes check"). This silently accepts any computed bounds when the constraint type's bounds can't be determined, potentially masking errors.
- **Recommendation**: Consider returning `false` or a tri-state result when bounds can't be extracted.

---

## diagnosis.rs

- ### 19. `byte_span_to_origin` sets line/column to 0, fileid to None

- **Severity**: **High — diagnostics point to wrong locations**
- **Description**: All `SourcePosition` values created by this function have `line: 0`, `column: 0`, and `fileid: None`. Only the byte `offset` is populated. This means all diagnostics from the solver will display at line 0, column 0 in an unknown file, making them nearly useless for users trying to locate the error source.
- **Recommendation**: Dont fix for now.

- ### ✅ 20. `OperationResultOutOfRefinementBounds` stores target bounds, not computed bounds

- **Severity**: **High — misleading error messages**
- **Description**: The error variant has fields `computed_min: u128, computed_max: u128`, suggesting they hold the computed operation result range. However, callers (`visit_binary` line 565, `visit_unary` line 629) pass `bnds.lo.max(0) as u128` and `bnds.hi` where `bnds` comes from `extract_bounds_from_type(&result_ty)` — the **target** refinement bounds. The actual computed result bounds (`res` in `visit_binary`, or the unary result in `visit_unary`) are discarded. The error message says "operation result range [X, Y] cannot be guaranteed..." but X and Y are the _expected_ bounds, not the _actual_ computed range.
- **Recommendation**: Pass the computed result bounds (`res.lo`, `res.hi`) instead of the target bounds, and rename fields to `computed_lo: i128, computed_hi: u128` to match the Bounds struct.

- ### 21. Non-contiguous variant IDs suggest incomplete maintenance

- **Severity**: Low (maintainability)
- **Description**: Variant IDs are 0, 3, 4, 5, 6, 8, 11, 14 — gaps suggest variants were added and removed over time. This makes it unclear whether IDs 1, 2, 7, 9, 10, 12, 13 are reserved or free.
- **Recommendation**: renumber contiguously.

---

## solve.rs

- ### ✅ 22. `Solver::find_common_integer_type` — `best` may be a `Refine` type while comparisons unwrap Refine

- **Severity**: Medium
- **Description**: `best` is set to the raw constraint type, which could be `Type::Refine { base, .. }`. Later, `best.unwrap().is_signed_primitive()` and `Self::type_bit_width(&best.unwrap())` are called directly on `best`. If `best` is a `Refine` type, `is_signed_primitive()` and `type_bit_width` might not behave as expected (depending on their implementation on `Type`). The code should unwrap `Refine` to `base` for `best` consistently, just as it does for `eff`.
- **Recommendation**: Unwrap Refine when setting `best`: `best = Some(match &*eff { Type::Refine { base, .. } => *base, _ => eff })`.

- ### ✅ 23. `Solver::find_common_integer_type` — signed type range checks only verify positive half

- **Severity**: Medium
- **Description**: For signed types (I8, I16, I32, I64), the `fits` check only verifies `value <= MAX_POSITIVE`. Negative literals (which have large `u128` values in two's complement representation) would not "fit" these checks, but they fall through to the `_ => true` wildcard for `I128`. So negative literals would only fit into `I128`, never into `I8`–`I64`. This means a literal like `-5` with no type annotation would be inferred as `I128` instead of the more natural `I32`.
- **Recommendation**: Also check for negative values: `value <= MAX_POSITIVE || (value as i128) >= MIN_NEGATIVE`.

- ### ✅ 24. `Solver::find_common_integer_type` — `U128` not explicitly handled, uses wildcard

- **Severity**: Low
- **Description**: `U128` is not listed in the match arms. The `_ => true` wildcard makes it always "fit". This means any literal value is considered to fit in `U128`, which is correct since `u128` can hold any `u128` value. But relying on the wildcard is fragile and could mask bugs if new types are added.
- **Recommendation**: Add an explicit `Type::U128 { .. } => true` arm.

- ### ✅ 25. `Solver::find_common_integer_type` — non-deterministic preference for same-width types

- **Severity**: Low
- **Description**: When two types have the same bit width, the one encountered first in iteration is kept. Since constraints are stored in a `HashSet`, iteration order is deterministic per run but may vary across Rust versions or hashCode changes. This could cause non-deterministic type inference for edge cases.
- **Recommendation**: Add a deterministic tiebreaker (e.g., prefer signed over unsigned, or use a type ordering).

- ### ✅ 26. `Solver::solve_inferred_integer` — clones entire constraint set unnecessarily

- **Severity**: Low (performance)
- **Description**: `self.constraints.get(id).cloned().unwrap_or_default().into_iter().collect()` allocates a Vec of cloned constraints. The function only needs to iterate over references. This allocation happens for every inferred integer literal.
- **Recommendation**: Iterate over `self.constraints.get(id).map(HashSet::iter).into_iter().flatten()`.

- ### ✅ 27. `Solver::solve_inferred_integer` — casts u128 value to i128 for error reporting, wraps

- **Severity**: Low
- **Description**: `check_errors.push((ty, value as i128))` — the original `value` is `u128`. If the literal value is larger than `i128::MAX`, the cast wraps to a negative number, and the error diagnostic will show an incorrect (negative) value.
- **Recommendation**: Store the original `u128` value and convert appropriately for display.

- ### 28. `Solver::solve_inferred_integer` — defaults to `I32` when no constraints match

- **Severity**: Low
- **Description**: `unwrap_or_else(|| TypeId::from(Type::I32 { span }))` — an integer literal with zero type constraints defaults to I32. If the literal value is `300_000_000_000` (doesn't fit in I32), the subsequent conversion will fail with an "out of range" error, and the literal stays as `InferredInteger` (unresolved). The fallback should be the smallest type that fits the value.
- **Recommendation**: Choose the smallest integer type that can hold the literal value.

- ### ✅ 29. `Solver::solve_inferred_float` — non-deterministic preference between F32 and F64

- **Severity**: Medium
- **Description**: When constraints include both `F32` and `F64`, the result depends on iteration order of the HashSet. If `F32` is encountered first, `best = F32`, then `F64` replaces it (correct: prefer wider). But if `F64` is encountered first, `best = F64`, and `F32` does NOT replace it. So the outcome is deterministic per run but dependent on hash order. This is a bug: both orderings should prefer `F64`.
- **Recommendation**: Always pick the widest type: if current is F64 and we see F32, keep F64; if current is F32 and we see F64, upgrade. Or just select based on bit-width after the loop.

- ### 30. `Solver::solve_inferred_float` — precision loss from f64 to f32 not warned

- **Severity**: Low
- **Description**: `*value as f32` — truncates the f64 value to f32. This can lose precision. The solver doesn't warn about potential precision loss.
- **Recommendation**: Consider emitting a warning for lossy float conversions, or at least checking `value == OrderedFloat(*value as f32 as f64)`.

- ### ✅ 31. `Solver::visit_enum_variant` — panics on missing variant

- **Severity**: Medium (robustness)
- **Description**: `.expect("variant not present")` will panic if the variant name in the value doesn't match any variant in the enum definition. This should be a diagnostic, not a panic.
- **Recommendation**: Return early or emit a `TypeErr` diagnostic instead of panicking.

- ### 32. `Solver::visit_binary` — same misleading bounds bug as diagnosis issue #20

- **Severity**: **High**
- **Description**: The error reporting for operation result out of refinement bounds uses `bnds.lo.max(0) as u128` and `bnds.hi` where `bnds` is from `extract_bounds_from_type(&result_ty)` (the target bounds), NOT the computed bounds `res`. The computed bounds are in `res` but are discarded. See also diagnosis issue #20.
- **Recommendation**: Use `res.lo` and `res.hi` instead of `bnds.lo` and `bnds.hi`.

- ### 33. `Solver::visit_unary` — same misleading bounds bug

- **Severity**: **High**
- **Description**: Same issue as #32 for unary operations. The error stores target bounds instead of computed bounds.
- **Recommendation**: Same as #32.

- ### ✅ 34. `Solver::visit_unary` — clones constraints twice

- **Severity**: Low (performance)
- **Description**: Constraints are cloned before visiting the operand (line 613) and then cloned again after visiting (line 619) for bounds checking. The second clone is unnecessary — the first clone could be reused if stored.
- **Recommendation**: Store the cloned constraints in a local variable and reuse.

- ### 35. `Solver::visit_field_access` — does not propagate constraints

- **Severity**: Low
- **Description**: The function only visits the expression (`self.visit(expr)`) but does not propagate any type constraints to the field or from the containing struct type. Field access types must be determined by other mechanisms (struct field lookup in HIR type resolution), which may happen before the solver runs.
- **Recommendation**: add appropriate constraint propagation.

- ### 36. `Solver::visit_method_call` — self borrow always uses `mutable: false, exclusive: false`

- **Severity**: **High — `&mut self` methods get immutable borrow**
- **Description**: When converting a method call to a function call, the solver checks if the first parameter is a reference type and, if so, automatically wraps the self object in a `Borrow` node. However, the `Borrow` is always created with `mutable: false` and `exclusive: false`, regardless of whether the method takes `&self` or `&mut self`. This means `&mut self` methods would receive an immutable shared reference, which is semantically incorrect and would likely cause borrow-checker errors downstream.
- **Recommendation**: Extract mutability and exclusivity from the parameter type: if `Type::Reference { mutable, exclusive, .. }`, use those values in the `Borrow` node.

- ### 37. `Solver::visit_call` — generic inference can fire twice with different substitution strategies

- **Severity**: Medium
- **Description**: `infer_generic_args_from_call` is tried first (positional only). Then, if the callee is still generic, `infer_generic_args_from_call_named` is tried. But the first call may have already replaced `callee` with a monomorphized version. The second check `if let Some(ref fid) = callee_func_id` uses the saved `callee_func_id` from before the replacement, so it still fires. This means the callee can be replaced twice, and `monomorphize_function` is called twice for the same function with potentially different substitutions. The second replacement wins, silently discarding the first. This is likely unintentional — the second attempt should only fire if the first failed.
- **Recommendation**: Use `if ... else if` or check whether the first substitution succeeded before trying the second.

- ### 38. `Solver::infer_generic_args_from_call` — rejects calls with fewer args than params

- **Severity**: Medium
- **Description**: `if ptypes.len() != args.len() { return None; }` — returns None if argument count doesn't match parameter count exactly. But functions can have default-valued parameters, and calls may legitimately provide fewer arguments. This causes generic inference to fail entirely for such calls, even though inference from the provided arguments should still be possible.
- **Recommendation**: Only require `args.len() <= ptypes.len()` and skip parameters beyond the provided argument count.

- ### 39. `Solver::infer_generic_args_from_call` — returns None if no substitution was found, even for valid empty-generics case

- **Severity**: Medium
- **Description**: `if subst.mapping.is_empty() { None } else { Some(subst) }` — if no generic parameters were mapped to concrete types, returns None. But a generic function might be called where type inference is supposed to come from the return type context, not from arguments. For example, `fn foo<T>() -> T; let x: I32 = foo();` — the call provides no arguments, so `subst.mapping` is empty, and inference fails. This prevents monomorphization, which would otherwise happen when the return type constraint propagates.
- **Recommendation**: Return `Some(subst)` even when empty, and let `monomorphize_function` handle the empty substitution (it would produce the same function but without generics).

- ### 40. `Solver::infer_generic_args_from_call_named` — nested generic params not detected

- **Severity**: **High — incomplete monomorphization**
- **Description**: The function attempts to verify that all generic params are accounted for. It uses `type_contains_generic_param_name` to check if a param appears in any parameter type, then looks for a direct `Type::GenericParam` at the top level of that parameter type. If the generic param is nested inside a compound type (e.g., `Array<T>` or `Pointer<T>`), `type_contains_generic_param_name` returns true, but the `if let Type::GenericParam { index, .. } = &*p.ty` check fails (because the top-level type is `Array`, not `GenericParam`), and `idx` is `None`. The param is then skipped, and the function may return `Some(subst)` with missing mappings.
- **Recommendation**: Use `collect_generic_params_from_type` to build a proper mapping, or recursively search for the index instead of only checking the top level.

- ### 41. `Solver::infer_generic_args_from_struct_fields` — same nested generic param detection issue

- **Severity**: **High — incomplete monomorphization**
- **Description**: Same pattern as `infer_generic_args_from_call_named` — the `appears` flag is set correctly via `type_contains_generic_param_name`, but the check for whether a param is "covered" at lines 1160-1163 assumes the substitution will contain the param's index. If a param appears only nested in compound types, the unification at line 1153 (`unify_types_with_subst(&at, ft, &mut subst)`) would need to reach into those compound types to extract the mapping, but `unify_types_with_subst` itself doesn't recurse into `Parameterized` types (see issue #49). So the mapping might indeed be incomplete.
- **Recommendation**: Fix both `unify_types_with_subst` and `collect_generic_params_from_type` to handle all compound types.

- ### 42. `Solver::monomorphize_function` — panics on depth limit instead of reporting diagnostic

- **Severity**: Medium (robustness)
- **Description**: `panic!("mono depth limit exceeded")` — if monomorphization recursion exceeds `MAX_MONO_DEPTH` (64), the compiler panics and crashes. This should be a user-facing diagnostic.
- **Recommendation**: Emit a `TypeErr` diagnostic and return the original un-monomorphized function id (or an error sentinel).

- ### 43. `Solver::monomorphize_function` — mono_depth can underflow due to cycle detection

- **Severity**: **High — integer underflow in release, or panic in debug**
- **Description**: When `mono_in_progress` contains the cache key (indicating a recursive monomorphization cycle), the function returns early at line 1227 WITHOUT incrementing `mono_depth`. However, the _caller_ of this function already incremented `mono_depth` at line 1229 (or will do so). After the early return, the caller decrements `mono_depth` at line 1269. Net effect: `mono_depth` decreases by 1 for each cycle detection. If enough cycles are detected (e.g., deeply nested recursive generic calls), `mono_depth` can underflow from 0 to `u32::MAX`, which would then fail the depth check at line 1219 and panic. In debug mode, this would cause an arithmetic overflow panic.
- **Recommendation**: Track `mono_depth` on a per-key basis (e.g., store the depth alongside the cache key in `mono_in_progress`), or use a separate cycle-detection mechanism that doesn't affect the depth counter.

- ### 44. `Solver::monomorphize_struct` — same mono_depth underflow bug

- **Severity**: **High — same as #43**
- **Description**: Identical issue to `monomorphize_function`. The early return at line 1283 doesn't increment depth, but the caller will decrement.

- ### 45. `resolve_type_impl` — UnresolvedArray silently defaults length to 0 on evaluation failure

- **Severity**: Medium
- **Description**: `u32::try_from(v).ok().unwrap_or(0)` — if the array length expression evaluates to a non-u32 value (e.g., `u64::MAX`) or can't be evaluated (constant evaluation fails), the length silently becomes 0. This could cause incorrect memory layouts and code generation.
- **Recommendation**: Return the type unchanged (keep as `UnresolvedArray`) when the length can't be evaluated, and let the validator emit a diagnostic.

- ### 46. `resolve_type_impl` — Parameterized type: named arguments not resolved

- **Severity**: **High — named generic arguments silently not resolved**
- **Description**: The code resolves positional args (`args.positional.iter().map(|a| resolve_type_impl(s, a, log))`) but clones named args as-is (`named: args.named.clone()`). If named type arguments contain nested `UnresolvedArray`, `UnresolvedRefine`, or other resolvable types, they will remain unresolved. This could lead to unresolved types appearing in the final HIR.
- **Recommendation**: Also resolve named arguments: `args.named.iter().map(|(k, v)| (k.clone(), resolve_type_impl(s, v, log))).collect()`.

- ### 47. `resolve_type_impl` — catch-all returns unchanged type for unknown variants

- **Severity**: Low
- **Description**: `_ => ty.clone()` — if a new type variant is added to the HIR that needs resolution (e.g., a new compound type wrapper), it would be silently ignored here.
- **Recommendation**: Add a `debug_assert!` or log a warning for unhandled type variants that might need resolution.

- ### 48. `finalize_value_recursive` — catch-all returns without processing children for unknown variants

- **Severity**: Low
- **Description**: `_ => return` — for unrecognized Value variants, the function returns without recursing into children. If a new Value variant is added that contains nested inferred literals, they would not be finalized.
- **Recommendation**: Use exhaustive matching or add a compile-time check that all variants are handled.

---

## monomorphize.rs

- ### 49. `unify_types_with_subst` — does not handle `Parameterized` types

- **Severity**: **High — generic type arguments in Parameterized types are not unified**
- **Description**: The function recursively handles `Pointer`, `SliceRef`, `Reference`, `Array`, `Tuple`, and `Function` types, but `Type::Parameterized` is not handled. A type like `Vec<T>` compared to `Vec<I32>` would fall through to `_ => {}` and no unification would occur. The generic parameter `T` would not be bound to `I32`. This means monomorphization of functions or structs involving generic container types would produce incomplete or incorrect substitutions.
- **Recommendation**: Add a match arm for `Type::Parameterized` that recursively unifies the base type and all type arguments.

- ### 50. `type_contains_any_generic_param` — does not inspect `Parameterized` types

- **Severity**: **High — generic field types missed**
- **Description**: The function checks for `GenericParam` in various compound types but does not recurse into `Type::Parameterized { base, args }`. A field of type `Vec<T>` would return `false`, causing `apply_struct_field_constraints` to add an equality constraint instead of skipping the field. This could lead to incorrect type inference.
- **Recommendation**: Add handling for `Type::Parameterized`: check both `base` and all positional/named args.

- ### 51. `type_contains_any_generic_param` — does not inspect `Refine`'s min/max

- **Severity**: Low
- **Description**: Only `base` of `Refine` is checked. The `min` and `max` are `Lit` values, which can't contain type parameters — this is fine. But worth noting for completeness.

- ### 52. `type_contains_generic_param_name` — does not inspect `Parameterized` types

- **Severity**: **High — same impact as #50**
- **Description**: Same issue as `type_contains_any_generic_param` but for the name-specific variant. A generic parameter name inside a `Parameterized` type would not be found.

- ### 53. `collect_generic_params_from_type` — does not inspect `Parameterized` types

- **Severity**: **High — same impact**
- **Description**: Same issue. Generic params in `Parameterized` type arguments are not collected. This affects `infer_generic_args_from_constraints` which uses this function to build the name→index mapping.

---

## range.rs

- ### 54. `ensure_range_structs` — range struct names could conflict with user code

- **Severity**: Low
- **Description**: The function creates synthetic struct definitions with names like "Range", "RangeInclusive", etc. in the global symbol table. If user code defines a struct with the same name, there would be a conflict. The function checks `tab.get_struct(&name).is_some()` and skips if it exists, but this means either the synthetic or user definition wins silently.
- **Recommendation**: Use a reserved namespace (e.g., `@Range`) or add these to a separate scope that doesn't conflict with user-defined names.

- ### 55. `make_range_struct_object` — panics if range struct not found

- **Severity**: Low (robustness)
- **Description**: `.expect("range struct not found")` — if `ensure_range_structs` somehow didn't run or the struct was removed, this panics.
- **Recommendation**: Return a diagnostic error or call `ensure_range_structs` inline as a fallback.

- ### 56. `ensure_range_structs` called redundantly from both public entry points

- **Severity**: Low (performance)
- **Description**: Both `resolve_function` and `resolve_global` call `ensure_range_structs`. If resolving many functions and globals, this idempotent check runs many times. It's cheap (a HashMap lookup), but still redundant.
- **Recommendation**: Call `ensure_range_structs` once at the start of the solver phase, or make it lazy (run on first range encounter).

---

## substitution.rs

- ### 57. `Substitution::apply` — `Parameterized` type arguments are lost

- **Severity**: **Critical — monomorphized types lose all generic arguments**
- **Description**: `Type::Parameterized { base, .. } => self.apply(base)` — only the base type is substituted; the type arguments (`args.positional` and `args.named`) are completely dropped. When monomorphizing `Vec<T>` with `T -> I32`, this produces `Vec` (a bare struct reference with no type arguments) instead of `Vec<I32>`. This is a critical correctness bug that affects all generic container types. The returned type is semantically different from the input.
- **Recommendation**: Reconstruct the Parameterized type with substituted arguments:

```rust
Type::Parameterized { base, args, span } => {
    let new_base = self.apply(base);
    let new_positional: Vec<TypeId> = args.positional.iter()
        .map(|a| TypeId::from(self.apply(a)))
        .collect();
    let new_named: BTreeMap<_, _> = args.named.iter()
        .map(|(k, v)| (k.clone(), TypeId::from(self.apply(v))))
        .collect();
    Type::Parameterized {
        span: *span,
        base: TypeId::from(new_base),
        args: Arguments {
            positional: new_positional.into(),
            named: new_named,
        },
    }
}
```

---

## Summary Statistics

- [ ] **Critical**: 1 remaining — #57
- [ ] **High**: 11 remaining — #19, #36, #40, #41, #43, #44, #46, #49, #50, #52, #53
- [ ] **Medium**: 5 remaining — #37, #38, #39, #42, #45
- [ ] **Low**: 14 remaining — #1, #4, #12, #18, #21, #28, #30, #35, #47, #48, #51, #54, #55, #56

**Total: 57 issues found across 8 files. Fix Progress: 24 / 57 resolved.**

### Most Critical Issues (fix these first):

- [x] 1. **#7** — `i128::MIN.abs()` panic in `compute_binary_bounds` Mod handling (compiler crash)
- [ ] 2. **#57** — `Substitution::apply` drops all type arguments from Parameterized types (silent incorrect code generation)
- [ ] 3. **#19** — Diagnostics point to line 0, column 0 (users can't find errors)
- [x] 4. **#20, #32, #33** — Error messages show wrong bounds (misleading diagnostics) (visit_binary and visit_unary callers fixed)
- [ ] 5. **#43, #44** — `mono_depth` underflow in cycle detection (potential compiler crash)
- [ ] 6. **#49, #50, #52, #53** — Parameterized types not handled in unification/generic detection (incomplete monomorphization)
